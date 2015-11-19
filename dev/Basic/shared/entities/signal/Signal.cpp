//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <cmath>

#include "Signal.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/models/Constants.h"
#include "GenConfig.h"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "util/Profiler.hpp"
#include "geospatial/network/RoadNetwork.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using namespace sim_mob;
using namespace std;

typedef Entity::UpdateStatus UpdateStatus;

Signal::Signal(const Node *node, const MutexStrategy &mtxStrat, unsigned int id, SignalType)
: Agent(mtxStrat, id), node(node), trafficLightId(node->getTrafficLightId())
{
}

Signal::~Signal()
{
	clear_delete_vector(phases);
}

const Node* Signal::getNode() const
{
	return node;
}

SignalType Signal::getSignalType() const
{
	return signalType;
}

const std::vector<Phase *>& Signal::getPhases()
{
	return phases;
}

bool Signal::isNonspatial()
{
	return true;
}

Signal_SCATS::Signal_SCATS(const Node *node, const MutexStrategy &mtxStrat)
: Signal(node, mtxStrat, -1, SignalType::SIGNAL_TYPE_SCATS), currCycleTimer(0), currPhaseAtGreen(0), isNewCycle(false)
{
	updateInterval = ConfigManager::GetInstance().FullConfig().granSignalsTicks * ConfigManager::GetInstance().FullConfig().baseGranMS() / 1000;
	splitPlan = new SplitPlan();
}

Signal_SCATS::~Signal_SCATS()
{
	safe_delete_item(splitPlan);	
}

bool Signal_SCATS::frame_init(timeslice now)
{
	initialise();
	return true;
}

Entity::UpdateStatus Signal_SCATS::frame_tick(timeslice now)
{
	if (ConfigManager::GetInstance().FullConfig().loopDetectorCounts.outputEnabled)
	{
		curVehicleCounter.aggregateCounts(now);
	}

	isNewCycle = updateCurrCycleTimer();

	//We do not update currPhaseAtGreen to the new value as we still need some information 
	//(degree of saturation) obtained during the last phase
	int phaseId = computeCurrPhase(currCycleTimer);

	if (phaseId < phases.size())
	{
		phases[phaseId]->update(currCycleTimer);
	}
	else
	{
		std::stringstream msg;
		msg << "Error: Signal_SCATS::frame_tick(): phaseId (" << phaseId << ") out of range (" << phases.size() << ")";
		throw std::runtime_error(msg.str());
	}

	if (currPhaseAtGreen != phaseId)
	{
		computePhaseDS(currPhaseAtGreen, now);
		currPhaseAtGreen = phaseId;
	}

	if (isNewCycle)
	{
		updateNewCycle();
		initialisePhases();
	}

	return UpdateStatus::Continue;
}

void Signal_SCATS::frame_output(timeslice now)
{
	std::stringstream output;

	output << "{\"TrafficSignalUpdate\":";
	output << "{\"SignalId\":\"" << trafficLightId << "\",";
	output << "\"frame\": " << now.frame() << ",";

	if (!phases.empty())
	{
		output << "\"currPhase\": \"" << phases[currPhaseAtGreen]->getName() << "\",";
		output << "\"phases\":[";
		
		for (unsigned int i = 0; i < phases.size(); ++i)
		{
			const Phase *phase = phases[i];
			linksMapping &linksMap = phase->getLinksMap();

			if (!linksMap.empty())
			{
				output << "{\"name\": \"" << phase->getName() << "\",";
				output << "\"links\":[";

				Phase::linksMappingConstIterator itLinksMap = linksMap.begin();

				while (itLinksMap != linksMap.end())
				{
					output << "{";
					output << "\"link_from\":\"" << (*itLinksMap).first << "\",";
					output << "\"link_to\":\"" << (*itLinksMap).second.toLink << "\",";
					output <<"\"current_colour\":" << trafficColorMap.find((*itLinksMap).second.currColor)->second << "},";
					++itLinksMap;
				}

				output << "]},";
			}
		}
		
		output << "]";
	}
	
	output << "}}\n";

	LogOut(output.str());
}

void Signal_SCATS::initialise()
{
	//Initialise the vehicle counter
	curVehicleCounter.initialise(this);
	
	//Create plans and phases
	createPlans();
	
	//Initialise the phases
	initialisePhases();
	phaseDensity.resize(phases.size(), 0);
}

bool Signal_SCATS::updateCurrCycleTimer()
{
	bool isNew = false;
	
	if ((currCycleTimer + updateInterval) >= splitPlan->getCycleLength())
	{
		isNew = true;
	}
	
	//Even if it is a new cycle (and therefore a new cycle length, the currCycleTimer will hold the amount of time 
	//system has proceeded to the new cycle
	currCycleTimer = std::fmod((currCycleTimer + updateInterval), splitPlan->getCycleLength());
	
	return isNew;
}

std::size_t Signal_SCATS::computeCurrPhase(double currCycleTimer)
{
	const std::vector< double > &currSplitPlan = splitPlan->getCurrSplitPlan();
	double sum = 0;
	std::size_t phase = 0;
	
	for (phase = 0; phase < phases.size(); phase++)
	{
		//In each iteration, the sum will represent the time (with respect to cycle length) each phase would end at
		sum += splitPlan->getCycleLength() * currSplitPlan[phase] / 100;
		
		if (sum > currCycleTimer)
		{
			break;
		}
	}

	if (phase >= phases.size())
	{
		std::stringstream str("");
		str << "Error: Signal_SCATS::computeCurrPhase(): phase (" << phase << ") >= numOfPhases (" << phases.size() << ")"; 
		str << "\ncurrCycleTimer(" << currCycleTimer << ") <= sum (" << sum << ")";
		throw std::runtime_error(str.str());
	}

	return phase;
}

double Signal_SCATS::computePhaseDS(int phaseId, const timeslice &now)
{
	double lane_DS = 0, maxPhaseDS = 0;
	Phase *phase = phases[phaseId];

	double totalGreen = phase->computeTotalGreenTime();
	
	Phase::linksMappingIterator linkIterator = phase->getLinksMapBegin();
	for (; linkIterator != phase->getLinksMapEnd(); linkIterator++)
	{
		const RoadNetwork *network = RoadNetwork::getInstance();
		const Link *link = network->getById(network->getMapOfIdVsLinks(), linkIterator->first);
		
		if (link->getToNode() != node)
		{
			continue;
		}
		
		const std::vector<Lane *> lanes = link->getRoadSegments().back()->getLanes();
		
		for (std::size_t i = 0; i < lanes.size(); i++)
		{ 
			const Lane *lane = NULL;
			lane = lanes.at(i);
			
			if (lane->isPedestrianLane())
			{
				continue;
			}
			
			const Sensor::CountAndTimePair &ctPair = loopDetectorAgent->getCountAndTimePair(*lane);
			lane_DS = computeLaneDS(ctPair, totalGreen);
			
			if (lane_DS > maxPhaseDS)
			{
				maxPhaseDS = lane_DS;
			}
		}
	}

	phaseDensity[phaseId] = maxPhaseDS;
	curVehicleCounter.update();
	loopDetectorAgent->reset();
	return phaseDensity[phaseId];
}

double Signal_SCATS::computeLaneDS(const Sensor::CountAndTimePair& ctPair, double totalGreen)
{
	//CountAndTimePair gives the 'T' and 'n' of formula 2 in section 3.2 of the memorandum (page 3)
	std::size_t vehicleCount = ctPair.vehicleCount;
	unsigned int spaceTime = ctPair.spaceTimeInMilliSeconds;
	double stdSpaceTime = 1.04 * 1000; //1.04 seconds
	
	//Formula 2 in section 3.2 of the memorandum (page 3)
	double usedGreen = (vehicleCount == 0) ? 0 : totalGreen - (spaceTime - stdSpaceTime * vehicleCount);
	
	//Formula 1 in section 3.2 of the memorandum (page 3)
	return usedGreen / totalGreen;
}

void Signal_SCATS::updateNewCycle()
{
	//Update the split plan
	splitPlan->update(phaseDensity);
	
	resetCycle();
	loopDetectorAgent->reset();
	isNewCycle = false;
}

void Signal_SCATS::resetCycle()
{
	for (int i = 0; i < phaseDensity.size(); ++i)
	{
		phaseDensity[i] = 0;
	}
}

TrafficColor Signal_SCATS::getDriverLight(unsigned int fromLink, unsigned int toLink) const
{
	const Phase *currPhase = phases[currPhaseAtGreen];
	Phase::linksMappingEqualRange range = currPhase->getLinkTos(fromLink);
	Phase::linksMappingConstIterator iter;
	
	for (iter = range.first; iter != range.second; iter++)
	{
		if ((*iter).second.toLink == toLink)
		{
			break;
		}
	}

	//If the link is not listed in the current phase return red
	if (iter == range.second)
	{
		return TrafficColor::TRAFFIC_COLOUR_RED;
	}
	
	return (*iter).second.currColor;
}

void Signal_SCATS::createPlans()
{
	splitPlan->setParentSignal(this);
	createPhases();
	
	if(!phases.empty())
	{
		splitPlan->setDefaultSplitPlan(phases.size());
	}
}

void Signal_SCATS::createPhases()
{
	//Turning groups at the node
	const std::map<unsigned int, std::map<unsigned int, TurningGroup *> >& turningsOuterMap = node->getTurningGroups();
	std::map<unsigned int, std::map<unsigned int, TurningGroup *> >::const_iterator itOuterMap = turningsOuterMap.begin();
	
	//Iterate through the groups originating at each of the 'from links'
	while(itOuterMap != turningsOuterMap.end())
	{
		const std::map<unsigned int, TurningGroup *> &turningsInnerMap = itOuterMap->second;
		std::map<unsigned int, TurningGroup *>::const_iterator itInnerMap = turningsInnerMap.begin();
		
		//Iterate through the groups ending at each of the 'to links' for the selected 'from link'
		while(itInnerMap != turningsInnerMap.end())
		{
			//The colour sequence structure for this phase
			ToLinkColourSequence toLinkClrSeq(itInnerMap->first);
				
			//Phase name
			const std::string &phaseName = itInnerMap->second->getPhases();
			
			//Check whether a phase with with the same name is already created for this signal
			bool phaseExists = false;
			for(std::vector<Phase *>::const_iterator itPhases = phases.begin(); itPhases != phases.end(); ++itPhases)
			{
				if((*itPhases)->getName() == phaseName)
				{
					//Associate the colour sequence structure with this phase
					(*itPhases)->addLinkMapping(itOuterMap->first, toLinkClrSeq);
					
					phaseExists = true;
					
					break;
				}
			}
			
			if(!phaseExists)
			{
				//Create a new phase for this turning group
				Phase *phase = new Phase(phaseName, splitPlan);				

				//Associate the colour sequence structure with this phase
				phase->addLinkMapping(itOuterMap->first, toLinkClrSeq);

				//Add the phase to the vector of phases
				phases.push_back(phase);
			}
			
			++itInnerMap;
		}
		++itOuterMap;
	}
}

void Signal_SCATS::initialisePhases()
{
	const std::vector<double> &choice = splitPlan->getCurrSplitPlan();
	
	if (choice.size() != phases.size())
	{
		throw std::runtime_error("Mismatch on number of phases");
	}
	
	int i = 0;
	double percentageSum = 0;
	
	//Set percentage and phase offset for each phase
	for (int phaseIdx = 0; phaseIdx < getPhases().size(); phaseIdx++, i++)
	{
		//this ugly line of code is due to the fact that multi index renders constant versions of its elements
		Phase *phase = phases[phaseIdx];
		
		//The first phase has phase offset equal to zero, so skip the 0th index
		if (i > 0)
		{
			percentageSum += choice[i - 1];
		}
		
		phase->setPercentage(choice[i]);
		phase->setPhaseOffset(percentageSum * splitPlan->getCycleLength() / 100);
	}
	
	//Initialise the phases
	for (int phase = 0; phase < phases.size(); phase++, i++)
	{
		phases[phase]->initialize(splitPlan);
	}
}

VehicleCounter::VehicleCounter() : simStartTime(ConfigManager::GetInstance().FullConfig().simStartTime()),
frequency(ConfigManager::GetInstance().FullConfig().loopDetectorCounts.frequency),
logger(Logger::log(ConfigManager::GetInstance().FullConfig().loopDetectorCounts.fileName)), curTimeSlice(0, 0)
{
}

VehicleCounter::~VehicleCounter()
{
	serialize(curTimeSlice.ms());
}

void VehicleCounter::initialise(const Signal_SCATS* signal)
{
	this->signal = signal;

	const std::map<const Lane *, Shared<Sensor::CountAndTimePair> *>& countAndTimePairs = signal->getLoopDetector()->getCountAndTimePairMap();
	std::map<const Lane *, Shared<Sensor::CountAndTimePair> *>::const_iterator it;

	for (it = countAndTimePairs.begin(); it != countAndTimePairs.end(); ++it)
	{
		counter[it->first] = 0;
	}
}

void VehicleCounter::resetCounter()
{
	for (std::map<const Lane*, int> ::iterator it = counter.begin(); it != counter.end(); ++it)
	{
		it->second = 0;
	}
}

void VehicleCounter::serialize(const uint32_t& time)
{
	if (ConfigManager::GetInstance().FullConfig().loopDetectorCounts.outputEnabled)
	{
		std::map<const Lane*, int> ::iterator it(counter.begin());
		for (; it != counter.end(); it++)
		{
			logger << time << "," << signal->getNode()->getTrafficLightId() << "," << signal->getNode()->getNodeId() \
					<< "," << it->first->getRoadSegmentId() \
					<< "," << it->first->getLaneId() << "," << it->second << "\n";
		}
	}
}

void VehicleCounter::update()
{
	std::map<const Lane*, int> ::iterator it(counter.begin());
	const Sensor* loopDetector = signal->getLoopDetector();

	for (; it != counter.end(); ++it)
	{
		const Lane* lane = it->first;
		const Sensor::CountAndTimePair& ctPair = loopDetector->getCountAndTimePair(*lane);
		int& count = it->second;
		count = count + ctPair.vehicleCount;
	}
}

void VehicleCounter::aggregateCounts(const timeslice &currTime)
{
	curTimeSlice = currTime;
	const uint32_t& time = currTime.ms();

	if (time != 0 && time % frequency == 0)
	{
		update();
		serialize(time);
		resetCounter();
	}
}