//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverFacets.hpp"

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cstdio>
#include <ostream>
#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "Driver.hpp"
#include "entities/conflux/LinkStats.hpp"
#include "entities/Person_MT.hpp"
#include "entities/ScreenLineCounter.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/Vehicle.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/SurveillanceStation.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "MesoReroute.hpp"
#include "message/MessageBus.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "partitions/PartitionManager.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "path/PathSetManager.hpp"
#include "util/DebugFlags.hpp"
#include "util/Utils.hpp"
#include <random>
#include <array>
#include "geospatial/network/RoadNetwork.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;

namespace
{

/**
 * converts time from milli-seconds to seconds
 */
inline double convertToSeconds(uint32_t timeInMs)
{
	return (timeInMs / 1000.0);
}

/**an infinitesimal double to avoid rounding issues*/
const double INFINITESIMAL_DOUBLE = 0.0001;
}

namespace sim_mob
{
namespace medium
{

DriverBehavior::DriverBehavior() :
BehaviorFacet(), parentDriver(nullptr)
{
}

DriverBehavior::~DriverBehavior()
{
}

void DriverBehavior::frame_init()
{
	throw std::runtime_error("DriverBehavior::frame_init is not implemented yet");
}

void DriverBehavior::frame_tick()
{
	throw std::runtime_error("DriverBehavior::frame_tick is not implemented yet");
}

std::string DriverBehavior::frame_tick_output()
{
	throw std::runtime_error("DriverBehavior::frame_tick_output is not implemented yet");
}

Driver* DriverBehavior::getParentDriver()
{
	return parentDriver;
}

DriverMovement::DriverMovement() :
MovementFacet(), parentDriver(nullptr), currLane(nullptr), isQueuing(false), laneConnectorOverride(false), timeStep(ConfigManager::GetInstance().FullConfig().baseGranSecond()),
isRouteChangedInVQ(false)
{
	rerouter.reset(new MesoReroute(*this));
	if (MT_Config::getInstance().isEnergyModelEnabled())
	{
		speedCollector.clear();
		speedCollector.push_back(0.0);
		speedCollector.push_back(0.0);
		speedCollector.push_back(0.0);
	}
}

DriverMovement::~DriverMovement()
{
	/*
	 * possible candidate place for finalize
	 * if(!travelMetric.finalized){
		finalizeTravelTimeMetric();
	}*/
}

void DriverMovement::setPath(std::vector<const SegmentStats*> &path,Node *toNode,std::vector<RoadSegment*> segs)
{
	MT_Config& mtCfg = MT_Config::getInstance();
	const Conflux* confluxForLnk = mtCfg.getConfluxForNode(toNode);
	for(std::vector<RoadSegment *>::iterator itr=segs.begin();itr!=segs.end();itr++)
	{
		const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*itr);
		path.insert(path.end(), statsInSegment.begin(), statsInSegment.end());

	}
}
void DriverMovement::frame_init()
{
	bool pathInitialized = initializePath();
	if (pathInitialized)
	{
		Vehicle::VehicleType vehicleType = Vehicle::CAR;
		if((*parentDriver->parent->currTripChainItem)->getMode() == "Taxi")
		{
			vehicleType = Vehicle::TAXI;
		}
		Vehicle* newVehicle = new Vehicle(vehicleType, PASSENGER_CAR_UNIT);
		VehicleBase* oldVehicle = parentDriver->getResource();
		safe_delete_item(oldVehicle);
		parentDriver->setResource(newVehicle);
		TripChainItem* currentSubTrip = (*parentDriver->parent->currTripChainItem);

		if (MT_Config::getInstance().isEnergyModelEnabled())
		{
            speedCollector.clear();
            speedCollector.push_back(0.0);
            speedCollector.push_back(0.0);
            speedCollector.push_back(0.0);

			// INITIALIZE TRAJECTORY INFO AS EMPTY
			trajectoryInfo.totalDistanceDriven = 0.0;
			trajectoryInfo.totalTimeDriven = 0.0;
			trajectoryInfo.totalTimeFast = 0.0;
			trajectoryInfo.totalTimeSlow = 0.0;
		}
	}
	else
	{
		parentDriver->parent->setToBeRemoved();
	}
}


void DriverMovement::onNewDriverVelocitySample(double driverVelocitySample)
{	
	if (driverVelocitySample < 0)
	{
		driverVelocitySample = 0;
		Warn() << "Negative speed recorded on segment " << (parentDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId()) << std::endl;
	}

	if (MT_Config::getInstance().isEnergyModelEnabled())
	{
			parentDriver->parent->getPersonInfoNotConst().getVehicleParams().updatePreviousEnergy();
			if (speedCollector.size() == 3)
			{
                speedCollector.pop_front();
                speedCollector.push_back(driverVelocitySample);
                MT_Config::getInstance().getEnergyModel()->computeEnergyWithSpeedHolder(speedCollector, parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getVehicleStruct(), timeStep, 0);
                double timeStepEnergy = parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getTimestepEnergy();
                //double timeStepEnergy = parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getFrameEnergy(); //jo
                if (prevSegStats != nullptr)
                {
                    prevSegStats->onNewEnergySample(timeStepEnergy, speedCollector[1] * timeStep);
                }
            }
//			} else {
//				std::cout << "OH NO: bad speed collector " << speedCollector.size() << std::endl;
//			}
			trajectoryInfo.totalDistanceDriven += driverVelocitySample*timeStep;
			trajectoryInfo.totalTimeDriven += timeStep;
			if (driverVelocitySample >= 25)
			{
				trajectoryInfo.totalTimeFast += timeStep;
			}
			else if (driverVelocitySample <= 10)
			{
				trajectoryInfo.totalTimeSlow += timeStep;
			}
			double timeStepEnergy = parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getTimestepEnergy();
			if (prevSegStats != nullptr)
			{
				prevSegStats->onNewEnergySample(timeStepEnergy, speedCollector[1]*timeStep);
			}
			prevSegStats = const_cast<SegmentStats*>(pathMover.getCurrSegStats() );
//			driverVelocity.push_back(driverVelocitySample); //aa: this was already done by Jimi and Micheal
	}
}

void DriverMovement::onTripCompletion()
{	
	if (MT_Config::getInstance().isEnergyModelEnabled())
	{
		if (parentDriver->roleType == Role<Person_MT>::RL_ON_CALL_DRIVER) //(parentDriver->parent->getRole()->getRoleName() == "OnCallDriver")
		{
			MT_Config::getInstance().getEnergyModel()->onOnCallTripCompletion(this,parentDriver->parent, driverVelocity, timeStep);
			this->trajectoryInfo.totalDistanceDriven = 0.0;
			this->trajectoryInfo.totalTimeDriven = 0.0;
			this->trajectoryInfo.totalTimeFast = 0.0;
			this->trajectoryInfo.totalTimeSlow = 0.0;
			parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getVehicleStruct().tripTotalEnergy = 0.0;
		}
		else if (parentDriver->roleType == Role<Person_MT>::RL_ON_HAIL_DRIVER)
		{
			MT_Config::getInstance().getEnergyModel()->onOnHailTripCompletion(this,parentDriver->parent, driverVelocity, timeStep);
			this->trajectoryInfo.totalDistanceDriven = 0.0;
			this->trajectoryInfo.totalTimeDriven = 0.0;
			this->trajectoryInfo.totalTimeFast = 0.0;
			this->trajectoryInfo.totalTimeSlow = 0.0;
			parentDriver->parent->getPersonInfoNotConst().getVehicleParams().getVehicleStruct().tripTotalEnergy = 0.0;
		}
		else
		{
			MT_Config::getInstance().getEnergyModel()->onTripCompletion(this,parentDriver->parent, driverVelocity, timeStep);
		}
	}
}


void DriverMovement::frame_tick()
{

	DriverUpdateParams& params = parentDriver->getParams();
	const SegmentStats* currSegStats = pathMover.getCurrSegStats();
	if (!currSegStats)
	{
		//if currSegstats is NULL, either the driver did not find a path to his
		//destination or his path is completed. Either way, we remove this
		//person from the simulation.

		// if energy model is running, compute energy consumed during this frametick
		if (MT_Config::getInstance().isEnergyModelEnabled())
		{
//			if (MT_Config::getInstance().getEnergyModel()->getModelType() == "tripenergy")
//			{
				// ZN: Compute energy for final timestep in trajectory (if enabled)
			onNewDriverVelocitySample(0.0);
			//}
			// Record (or Compute --- in the case of the SimpleEnergy Model ---) Energy consumption
			onTripCompletion();
		}
		parentDriver->parent->setToBeRemoved();
		return;
	}
	else if (currSegStats == parentDriver->parent->getCurrSegStats())
	{
		//the vehicle will be in lane infinity before it starts.
		//set origin will move it to the correct lane
		if (parentDriver->parent->getCurrLane() == currSegStats->laneInfinity)
		{
			setOrigin(params);
		}
	}

	if (MT_Config::getInstance().isEnergyModelEnabled()) //jo Apr5 only need for Energy computation (I think)
	{
		pathMover.initDriverPathTracking();
	}

	//canMoveToNextSegment is GRANTED/DENIED only when this driver had previously
	//requested permission to move to the next segment. This request is made
	//only when the driver has reached the end of the current link
	if (parentDriver->parent->canMoveToNextSegment == Person_MT::GRANTED)
	{
		flowIntoNextLinkIfPossible(params);
	}
	else if (parentDriver->parent->canMoveToNextSegment == Person_MT::DENIED)
	{
		if (currLane)
		{
			if (parentDriver->parent->isQueuing)
			{
				moveInQueue();
			}
			else
			{
				addToQueue(); // adds to queue if not already in queue
			}

			params.elapsedSeconds = params.secondsInTick;
			parentDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
			parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
			setParentData(params);

			if (MT_Config::getInstance().isEnergyModelEnabled()) //jo Apr5 only need for Energy computation (I think)
			{
				pathMover.finalizeDriverPathTracking();
                onNewDriverVelocitySample(pathMover.getDistanceCovered() / (double)timeStep);
			}
/*			if(parentDriver && parentDriver->roleType != Role<Person_MT>::RL_BUSDRIVER)
			{
				Person_MT* person = parentDriver->parent;
				unsigned int segId = (person->getCurrSegStats() ? person->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() : 0);
				uint16_t statsNum = (person->getCurrSegStats() ? person->getCurrSegStats()->getStatsNumberInSegment() : 0);

				char logbuf[1000];
				sprintf(logbuf, "%s,%s,%u,seg:%u-%u,ln:%u,d:%f,q:%c,elpsd:%fs\n",
						parentDriver->getRoleName().c_str(),
						person->ge#include <boost/algorithm/string/config.hpp>tDatabaseId().c_str(),
						parentDriver->getParams().now.frame(),
						segId,
						statsNum,
						(person->getCurrLane() ? person->getCurrLane()->getLaneId() : 0),
						person->distanceToEndOfSegment,
						(person->isQueuing ? 'T' : 'F'),
						params.elapsedSeconds
						);
				person->log(std::string(logbuf));

			}
*/
			return;
		}
	}
	//if driver is still in lane infinity (currLane is null),
	//he shouldn't be advanced
	if (currLane && parentDriver->parent->canMoveToNextSegment == Person_MT::NONE && !isRouteChangedInVQ)
	{
		advance(params);
		setParentData(params);
	}

	if (MT_Config::getInstance().isEnergyModelEnabled()) //jo Apr5 only need for Energy computation (I think)
	{
		pathMover.finalizeDriverPathTracking(); //jo Apr4
		//aa{
		onNewDriverVelocitySample(pathMover.getDistanceCovered() / timeStep); //jo Apr 4
		//aa}
	}

	//Reset the value, so that we can enter above condition when needed next time.
	isRouteChangedInVQ = false;

/*	//Debug print
	if(parentDriver && parentDriver->roleType != Role<Person_MT>::RL_BUSDRIVER)
	{
		Person_MT* person = parentDriver->parent;
		unsigned int segId = (person->getCurrSegStats() ? person->getCurrSegStats()->getRoadSegment()->getRoadSegmentId() : 0);
		uint16_t statsNum = (person->getCurrSegStats() ? person->getCurrSegStats()->getStatsNumberInSegment() : 0);
		char logbuf[1000];
		sprintf(logbuf, "%s,%s,%u,seg:%u-%u,ln:%u,d:%f,q:%c,elpsd:%fs\n",
				parentDriver->getRoleName().c_str(),
				person->getDatabaseId().c_str(),
				parentDriver->getParams().now.frame(),
				segId,
				statsNum,
				(person->getCurrLane() ? person->getCurrLane()->getLaneId() : 0),
				person->distanceToEndOfSegment,
				(person->isQueuing ? 'T' : 'F'),
				params.elapsedSeconds
				);
		person->log(std::string(logbuf));

	}
*/
}

std::string DriverMovement::frame_tick_output()
{
	const DriverUpdateParams& params = parentDriver->getParams();
	if (pathMover.isPathCompleted() || !parentDriver->parent->getCurrSegStats() || !parentDriver->parent->getCurrLane()
	    || ConfigManager::GetInstance().CMakeConfig().OutputDisabled())
	{
		return std::string();
	}

	std::stringstream logout;
	logout << "(\"Driver\""
			<< "," << parentDriver->parent->getId()
			<< "," << params.now.frame()
			<< ",{"
			<< "\"RoadSegment\":\"" << (parentDriver->parent->getCurrSegStats()->getRoadSegment()->getRoadSegmentId())
			<< "\",\"Lane\":\"" << ((parentDriver->parent->getCurrLane()) ? parentDriver->parent->getCurrLane()->getLaneId() : 0)
			<< "\",\"DistanceToEndSeg\":\"" << parentDriver->parent->distanceToEndOfSegment;
	if (this->parentDriver->parent->isQueuing)
	{
		logout << "\",\"queuing\":\"" << "true";
	}
	else
	{
		logout << "\",\"queuing\":\"" << "false";
	}
	logout << "\"})" << std::endl;
	return logout.str();
}

void DriverMovement::initSegStatsPath(vector<WayPoint>& wpPath, vector<const SegmentStats*>& ssPath) const
{
	MT_Config& mtCfg = MT_Config::getInstance();
	for (vector<WayPoint>::iterator it = wpPath.begin(); it != wpPath.end(); it++)
	{
		if (it->type == WayPoint::LINK)
		{
			const Link* lnk = it->link;
			const Conflux* confluxForLnk = mtCfg.getConfluxForNode(lnk->getToNode());
			const vector<RoadSegment*>& segsInLnk = lnk->getRoadSegments();
			for(vector<RoadSegment*>::const_iterator segIt=segsInLnk.begin(); segIt!=segsInLnk.end(); segIt++)
			{
				const vector<SegmentStats*>& statsInSegment = confluxForLnk->findSegStats(*segIt);
				ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
			}
		}
	}
}

void DriverMovement::initSegStatsPath(const std::vector<const RoadSegment*>& rsPath, std::vector<const SegmentStats*>& ssPath)
{
	for (vector<const RoadSegment*>::const_iterator it = rsPath.begin(); it != rsPath.end(); it++)
	{
		const RoadSegment* rdSeg = *it;
		const vector<SegmentStats*>& statsInSegment = Conflux::getConflux(rdSeg)->findSegStats(rdSeg);
		ssPath.insert(ssPath.end(), statsInSegment.begin(), statsInSegment.end());
	}
}

bool DriverMovement::initializePath()
{
	//Only initialize if the next path has not been planned for yet.
	Person_MT* person = parentDriver->parent;
	if (!person->getNextPathPlanned())
	{
		//Save local copies of the parent's origin/destination nodes.
		parentDriver->origin = person->originNode;
		parentDriver->goal = person->destNode;

		if (person->originNode.node == person->destNode.node)
		{
			Print() << "DriverMovement::initializePath | Can't initializePath because origin and destination are the same for driver " << person->getId()
					<< "\norigin:" << person->originNode.node->getNodeId() << "\ndestination:" << person->destNode.node->getNodeId() << std::endl;
			return false;
		}

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> wp_path;
		const SubTrip& currSubTrip = *(person->currSubTrip);
		if (ConfigManager::GetInstance().FullConfig().PathSetMode()) // if use path set
		{
			bool useInSimulationTT = parentDriver->parent->usesInSimulationTravelTime();
			wp_path = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip, false, nullptr, useInSimulationTT);
		}
		else
		{
			const StreetDirectory& stdir = StreetDirectory::Instance();
			wp_path = stdir.SearchShortestDrivingPath<Node, Node>(*(parentDriver->origin).node, *(parentDriver->goal).node);
		}

		if (wp_path.empty()) //ideally should not be empty after randomization.
		{
			Print() << "Can't DriverMovement::initializePath(); path is empty for driver " << person->getDatabaseId() << std::endl;
			return false;
		}

		//Restricted area logic
		if(MT_Config::getInstance().isRegionRestrictionEnabled())
		{
			bool fromLocationInRestrictedRegion = RestrictedRegion::getInstance().isInRestrictedZone(wp_path.front());
			bool toLocationInRestrictedRegion = RestrictedRegion::getInstance().isInRestrictedZone(wp_path.back());
			if (!toLocationInRestrictedRegion && !fromLocationInRestrictedRegion)
			{//both O & D outside
				if (RestrictedRegion::getInstance().isInRestrictedZone(wp_path))
				{
					currSubTrip.cbdTraverseType = TravelMetric::CBD_PASS;
				}
			}
			else if (!(toLocationInRestrictedRegion && fromLocationInRestrictedRegion))
			{//exactly one of O & D is inside restricted region
				currSubTrip.cbdTraverseType = fromLocationInRestrictedRegion ? TravelMetric::CBD_EXIT : TravelMetric::CBD_ENTER;
			}
			//else we leave the cbdTraverseType as CBD_NONE
		}

		std::vector<const SegmentStats*> path;
		initSegStatsPath(wp_path, path);
		if (path.empty())
		{
			return false;
		}
		pathMover.setPath(path);
		const SegmentStats* firstSegStat = path.front();
		person->setCurrSegStats(firstSegStat);
		person->setCurrLane(firstSegStat->laneInfinity);
		person->distanceToEndOfSegment = firstSegStat->getLength();
	}
	person->setNextPathPlanned(true); //to indicate that the path to next activity is already planned
	return true;
}

void DriverMovement::setParentData(DriverUpdateParams& params)
{
	if (!pathMover.isPathCompleted())
	{
		parentDriver->parent->distanceToEndOfSegment = pathMover.getPositionInSegment();
		parentDriver->parent->setCurrLane(currLane);
		parentDriver->parent->setCurrSegStats(pathMover.getCurrSegStats());
		parentDriver->parent->setRemainingTimeThisTick(params.secondsInTick - params.elapsedSeconds);
	}
	else
	{
		parentDriver->parent->distanceToEndOfSegment = 0.0;
		parentDriver->parent->setCurrLane(nullptr);
		parentDriver->parent->setCurrSegStats(nullptr);
		parentDriver->parent->setRemainingTimeThisTick(0.0);
		parentDriver->parent->isQueuing = false;
	}
}

void DriverMovement::stepFwdInTime(DriverUpdateParams& params, double time)
{
	params.elapsedSeconds = params.elapsedSeconds + time;
}

bool DriverMovement::advance(DriverUpdateParams& params)
{
	if (pathMover.isPathCompleted())
	{
		
		// if energy model is running, compute energy consumed during this frametick
		if (MT_Config::getInstance().isEnergyModelEnabled()) {
			// ZN: Compute energy for final timestep in trajectory (if enabled)
			onNewDriverVelocitySample(0.0);
			onTripCompletion();	
		}
		// ZN: Need to add another call to onNewDriverVelocitySample here to catch final timestep energy
		parentDriver->parent->setToBeRemoved();
		return false;
	}

	if (parentDriver->parent->getRemainingTimeThisTick() <= 0)
	{
		return false;
	}

	if (isQueuing)
	{
		return advanceQueuingVehicle(params);
	}
	else //vehicle is moving
	{
		return advanceMovingVehicle(params);
	}
}

bool DriverMovement::moveToNextSegment(DriverUpdateParams& params)
{
	MT_Config& mtConfig = MT_Config::getInstance();
	bool res = false;
	bool isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	const SegmentStats* nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

	//currently the best place to call a handler indicating 'Done' with segment.
	const RoadSegment *curRs = (*(pathMover.getCurrSegStats())).getRoadSegment();
	//Although the name of the method suggests segment change, it is actually segStat change. so we check again!
	const RoadSegment *nxtRs = (nxtSegStat ? nxtSegStat->getRoadSegment() : nullptr);

	if (curRs && curRs != nxtRs)
	{
		onSegmentCompleted(curRs, nxtRs);
	}

	if (isNewLinkNext)
	{
		onLinkCompleted(curRs->getParentLink(), (nxtRs ? nxtRs->getParentLink() : nullptr));
	}

	//reset these local variables in case path has been changed in onLinkCompleted
	isNewLinkNext = (!pathMover.hasNextSegStats(true) && pathMover.hasNextSegStats(false));
	currSegStat = pathMover.getCurrSegStats();
	nxtSegStat = pathMover.getNextSegStats(!isNewLinkNext);

	if (!nxtSegStat)
	{
		const SegmentStats* lastSeg = currSegStat ;
		//vehicle is done
		pathMover.advanceInPath();
		
		// if energy model is running, compute energy consumed during this frametick
		if (MT_Config::getInstance().isEnergyModelEnabled())
		{ //jo Apr3 - only call these if mobilityServiceController NOT enabled to avoid issues
			//if(!ConfigManager::GetInstance().FullConfig().mobilityServiceController.enabled) //ConfigParams.mobilityServiceController.enabled)
			//{
			// ZN: Compute energy for final timestep in trajectory (if enabled)
			onNewDriverVelocitySample(0.0);
			onTripCompletion();
			//}
			// ZN: Add call to onNewDriverVelocitySample
		}

		if (pathMover.isPathCompleted())
		{
			const Link* currLink = currSegStat->getRoadSegment()->getParentLink();
			double linkExitTime = params.elapsedSeconds + (convertToSeconds(params.now.ms()));
			parentDriver->parent->currLinkTravelStats.finalize(currLink, linkExitTime, nullptr);
			TravelTimeManager::getInstance()->addTravelTime(parentDriver->parent->currLinkTravelStats); //in seconds
			currSegStat->getParentConflux()->setLinkTravelTimes(linkExitTime, currLink);
			parentDriver->parent->currLinkTravelStats.reset();
			currSegStat->getParentConflux()->getLinkStats(currLink).removeEntitiy(parentDriver->parent);
			decrementOutputCounter(currLane, currSegStat);
			currLane = nullptr;
			parentDriver->parent->setToBeRemoved();
			// linkExitTime and segmentExitTime are equal for the last segment in the path.
			updateScreenlineCounts(lastSeg, linkExitTime);
		}
		return false;
	}

	const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
	const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);

	if (isNewLinkNext)
	{
		parentDriver->parent->requestedNextSegStats = nxtSegStat;
		parentDriver->parent->requestedNextLane = laneInNextSegment;
		parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
		return false; // return whenever a new link is to be entered. Seek permission from Conflux.
	}

	double departTime = getLastAccept(laneInNextSegment, nxtSegStat);
/*	if(!nxtSegStat->isShortSegment())
	{
		if(nxtSegStat->hasQueue())
		{
			departTime += getAcceptRate(laneInNextSegment, nxtSegStat); //in seconds
		}
		else
		{
			departTime += (PASSENGER_CAR_UNIT / (nxtSegStat->getNumVehicleLanes() *nxtSegStat->getSegSpeed(true)));
		}
	}
*/
	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - convertToSeconds(params.now.ms())); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(nxtSegStat);
	if (canGoToNextRdSeg(params, nxtSegStat, nextLink))
	{
		if (isQueuing)
		{
			removeFromQueue();
		}

		decrementOutputCounter(currLane, currSegStat); // decrement from the currLane before updating it
		decrementInputCounter(laneInNextSegment, nxtSegStat);
		const Lane *prevLane = currLane;
		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nxtSegStat->getLength());
		double segExitTimeSec = params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		setLastAccept(currLane, segExitTimeSec, nxtSegStat);

		const SegmentStats* prevSegStats = pathMover.getPrevSegStats(true); //previous segment is in the same link
		if ((mtConfig.screenLineParams.outputEnabled) && prevSegStats
			&& prevSegStats->getRoadSegment() != pathMover.getCurrSegStats()->getRoadSegment())
		{
			// update road segment travel times
			updateScreenlineCounts(prevSegStats, segExitTimeSec);

			//Check if the current and next lanes belong to different links
			unsigned int prevLinkId = prevSegStats->getRoadSegment()->getLinkId();
			unsigned int currLinkId = pathMover.getCurrSegStats()->getRoadSegment()->getLinkId();

			if(prevLinkId != currLinkId)
			{
				//Link has changed, add the length of the turning path to the distance covered
				const Node *node = currLane->getParentSegment()->getParentLink()->getFromNode();
				const TurningGroup *tGroup = node->getTurningGroup(prevLinkId, currLinkId);
				const map<unsigned int, TurningPath*> *tPaths = tGroup->getTurningPaths(prevLane->getLaneId());

				if(tPaths)
				{
					auto itTurning = tPaths->find(currLane->getLaneId());

					if (itTurning != tPaths->end())
					{
						travelMetric.distance += itTurning->second->getLength();
					}
					else
					{
						stringstream msg;
						msg << "Vehicle is trying to move from link " << prevLinkId << " to " << currLinkId
						    << ". Current lane is " << currLane->getLaneId()
						    << ", but it is not connected to the selected next lane " << currLane->getLaneId();
						throw runtime_error(msg.str());
					}
				}
				else
				{
					stringstream msg;
					msg << "Vehicle is trying to move from link " << prevLinkId << " to " << currLinkId
					    << ". Current lane is " << currLane->getLaneId()
					    << ", but it is not connected to the next link!";
					throw runtime_error(msg.str());
				}
			}
		}

		res = true;
		advance(params);		
	}
	else
	{
		if (isQueuing)
		{
			moveInQueue();
		}
		else
		{
			addToQueue();
		}
		params.elapsedSeconds = params.secondsInTick;
		parentDriver->parent->setRemainingTimeThisTick(0.0);
	}
	return res;
}

/*
 * List of the operations performed in this method
 * 1- CBD Travel Metrics collections
 * 2- Re-routing
 */
void DriverMovement::onSegmentCompleted(const RoadSegment* completedRS, const RoadSegment* nextRS)
{
	if(traversed.empty() || traversed.back() != completedRS)
	{
		//1.record
		traversed.push_back(completedRS);

		//2. update travel distance
		travelMetric.distance += completedRS->getPolyLine()->getLength();

		//3. update the next surveillance station
		nextSurveillanceStn = nextRS ? nextRS->getSurveillanceStations().begin() :
		                      completedRS->getSurveillanceStations().end();
	}
}

void DriverMovement::onLinkCompleted(const Link * completedLink, const Link * nextLink)
{
	//1. Re-routing
	if (ConfigManager::GetInstance().FullConfig().getPathSetConf().reroute)
	{
		reroute();
	}
	
	//2. CBD
	processCBD_TravelMetrics(completedLink, nextLink);

	//3. update the next surveillance station
	nextSurveillanceStn = nextLink ? nextLink->getRoadSegments().front()->getSurveillanceStations().begin() :
	                      completedLink->getRoadSegments().back()->getSurveillanceStations().end();
}

void DriverMovement::flowIntoNextLinkIfPossible(DriverUpdateParams& params)
{
	//This function gets called for 2 cases.
	//1. Driver is added to virtual queue
	//2. Driver is in previous segment trying to add to the next
	MT_Config& mtConfig = MT_Config::getInstance();
	const SegmentStats* nextSegStats = pathMover.getNextSegStats(false);
	const SegmentStats* nextToNextSegStats = pathMover.getSecondSegStatsAhead();
	const Lane* laneInNextSegment = getBestTargetLane(nextSegStats, nextToNextSegStats);

	double departTime = getLastAccept(laneInNextSegment, nextSegStats);
/*	if(!nextSegStats->isShortSegment())
	{
		if(nextSegStats->hasQueue())
		{
			departTime += getAcceptRate(laneInNextSegment, nextSegStats); //in seconds
		}
		else
		{
			departTime += (PASSENGER_CAR_UNIT / (nextSegStats->getNumVehicleLanes() * nextSegStats->getSegSpeed(true)));
		}
	}
*/
	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (convertToSeconds(params.now.ms()))); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
	if (canGoToNextRdSeg(params, nextSegStats, nextLink))
	{
		if (isQueuing)
		{
			removeFromQueue();
		}

		currLane = laneInNextSegment;
		pathMover.advanceInPath();
		pathMover.setPositionInSegment(nextSegStats->getLength());

		//todo: consider supplying milliseconds to be consistent with short-term
		double linkExitTimeSec = params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		//set Link Travel time for previous link
		const SegmentStats* prevSegStats = pathMover.getPrevSegStats(false);
		if (prevSegStats)
		{
			// update link travel times
			updateLinkTravelTimes(prevSegStats, linkExitTimeSec);

			//update link stats
			updateLinkStats(prevSegStats);

			// update road segment screenline counts
			if (mtConfig.screenLineParams.outputEnabled) {
				updateScreenlineCounts(prevSegStats, linkExitTimeSec);
			}
		}
			setLastAccept(currLane, linkExitTimeSec, nextSegStats);
			setParentData(params);
			parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
	}
	else
	{
		//Person is in previous segment (should be added to queue if canGoTo failed)
		if (pathMover.getCurrSegStats() == parentDriver->parent->getCurrSegStats())
		{
			if (currLane)
			{
				if (parentDriver->parent->isQueuing)
				{
					moveInQueue();
				}
				else
				{
					addToQueue(); // adds to queue if not already in queue
				}
				parentDriver->parent->canMoveToNextSegment = Person_MT::NONE; // so that advance() and setParentData() is called subsequently
			}
		}
		else if (pathMover.getNextSegStats(false) == parentDriver->parent->getCurrSegStats())
		{
			//Person is in virtual queue (should remain in virtual queues if canGoTo failed)
			//do nothing
		}
		else
		{
			/**
			 * Person is in the virtual queue, but the path changed while there. As a result the
			 * next seg stats from the path mover no longer matches the current seg stats stored
			 * in the person object (the one we have permission to move to)
			 * We must now, get permission for the the new next segment
			 */
			nextSegStats = pathMover.getNextSegStats(false);
			parentDriver->parent->requestedNextSegStats = nextSegStats;
			parentDriver->parent->requestedNextLane = getBestTargetLane(nextSegStats, pathMover.getSecondSegStatsAhead());
			parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;

			// set current lane and segment stats back to pathMover's current
			currLane = getBestTargetLane(pathMover.getCurrSegStats(), nextSegStats);
			parentDriver->parent->setCurrSegStats(pathMover.getCurrSegStats());
			parentDriver->parent->setCurrLane(currLane);
			isRouteChangedInVQ = true;
			return;
		}
		params.elapsedSeconds = params.secondsInTick;
		parentDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

bool DriverMovement::canGoToNextRdSeg(DriverUpdateParams& params, const SegmentStats* nextSegStats, const Link* nextLink) const
{
	//return false if the Driver cannot be added during this time tick
	if (params.elapsedSeconds >= params.secondsInTick)
	{
		return false;
	}

	//check if the next road segment has sufficient empty space to accommodate one more vehicle
	if (!nextSegStats)
	{
		return false;
	}

	double enteringVehicleLength = parentDriver->getResource()->getLengthInM();
	double maxAllowed = nextSegStats->getNumVehicleLanes() * nextSegStats->getLength();
	double total = nextSegStats->getTotalVehicleLength();

	//if the segment is shorter than the vehicle's length and there are no vehicles in the segment just allow the vehicle to pass through
	//this is just an interim arrangement. this segment should either be removed from database or it's length must be updated.
	//if this hack is not in place, all vehicles will start queuing in upstream segments forever.
	//TODO: remove this hack and put permanent fix
	if ((maxAllowed < enteringVehicleLength) && (total <= 0))
	{
		return true;
	}
	
	//if this segment is a bus terminus segment, we assume only buses try to enter this segment and allow the bus inside irrespective of available space.
	if(nextSegStats->getRoadSegment()->isBusTerminusSegment())
	{
		return true;
	}

	bool hasSpaceInNextStats = ((maxAllowed - total) >= enteringVehicleLength);

	if (hasSpaceInNextStats && nextLink)
	{
		//additionally check if the length of vehicles in the lanegroup is not too long to accommodate this driver
		try
		{
			double maxAllowedInLG = nextSegStats->getAllowedVehicleLengthForLaneGroup(nextLink);
			double totalInLG = nextSegStats->getVehicleLengthForLaneGroup(nextLink);
			return (totalInLG < maxAllowedInLG);
		}
		catch (...)
		{
			const SegmentStats *curStats = pathMover.getCurrSegStats();
			std::string path = pathMover.printPath();
			std::cout << "curr stats:" << curStats->getRoadSegment()->getRoadSegmentId()
			          << "-" << curStats->getStatsNumberInSegment() << " path:" << path << std::endl;
			throw;
		}
	}
	return hasSpaceInNextStats;
}

void DriverMovement::moveInQueue()
{
	//1.update position in queue (vehicle->setPosition(distInQueue))
	//2.update p.timeThisTick
	double positionOfLastUpdatedAgentInLane = pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane);
	if (positionOfLastUpdatedAgentInLane == -1.0)
	{
		pathMover.setPositionInSegment(0.0);
	}
	else
	{
		pathMover.setPositionInSegment(positionOfLastUpdatedAgentInLane /*+ parentDriver->getResource()->getLengthCm()*/);
	}
}

bool DriverMovement::moveInSegment(double distance)
{
	double startPos = pathMover.getPositionInSegment();

	try
	{
		pathMover.moveFwdInSegStats(distance);
	}
	catch (std::exception& ex)
	{
		std::stringstream msg;
		msg << "Error moving vehicle forward for Agent ID: " << parentDriver->parent->getId() << "," << pathMover.getPositionInSegment() << "\n" << ex.what();
		throw std::runtime_error(msg.str().c_str());
		return false;
	}

	double endPos = pathMover.getPositionInSegment();
	updateFlow(pathMover.getCurrSegStats(), startPos, endPos);

	return true;
}

bool DriverMovement::advanceQueuingVehicle(DriverUpdateParams& params)
{
	bool res = false;

	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalTimeSpent = 0.0;
	double finalDistToSegEnd = 0.0;

	int inOutCounter = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);

	const SegmentStats* nxtSegStat = pathMover.getNextSegStats(false);
	if (nxtSegStat)
	{
		const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);
		inOutCounter = std::min(inOutCounter, getInputCounter(laneInNextSegment, nxtSegStat));
	}

	//The following line of code assumes vehicle length is in cm;
	//vehicle length and outrate cannot be 0.
	//There was a magic factor 3.0 in the denominator. It was removed because
	//its purpose was not clear to anyone.~Harish
	finalTimeSpent = initialTimeSpent + initialDistToSegEnd / (PASSENGER_CAR_UNIT * outRate);

	if (inOutCounter > 0 && finalTimeSpent < params.secondsInTick
			&& pathMover.getCurrSegStats()->getPositionOfLastUpdatedAgentInLane(currLane) == -1)
	{
		res = moveToNextSegment(params);
		finalDistToSegEnd = pathMover.getPositionInSegment();
	}
	else
	{
		moveInQueue();
		finalDistToSegEnd = pathMover.getPositionInSegment();
		params.elapsedSeconds = params.secondsInTick;
	}
	//unless it is handled previously;
	//1. update current position of vehicle/driver with finalDistToSegEnd
	//2. update current time, p.elapsedSeconds, with finalTimeSpent
	pathMover.setPositionInSegment(finalDistToSegEnd);

	return res;
}

bool DriverMovement::advanceMovingVehicle(DriverUpdateParams& params)
{
	bool res = false;
	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalDistToSegEnd = 0.0;
	double finalTimeSpent = 0.0;

	if (!currLane)
	{
		throw std::runtime_error("agent's current lane is not set!");
	}

	const SegmentStats* currSegStats = pathMover.getCurrSegStats();
	const LaneStats *laneStats = currSegStats->getLaneStats().find(currLane)->second;
	//We can infer that the path is not completed if this function is called.
	//Therefore currSegStats cannot be NULL. It is safe to use it in this function.
	double velocity = laneStats->getLaneVehSpeed(true);
	int inOutCounter = getOutputCounter(currLane, currSegStats);
	const SegmentStats* nxtSegStat = pathMover.getNextSegStats(false);

	if (nxtSegStat)
	{
		const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);
		inOutCounter = std::min(inOutCounter, getInputCounter(laneInNextSegment, nxtSegStat));
	}

	// add driver to queue if required
	double laneQueueLength = getQueueLength(currLane);
	if (laneQueueLength > currSegStats->getLength())
	{
		addToQueue();
		params.elapsedSeconds = params.secondsInTick;
	}
	else if (laneQueueLength > 0)
	{
		finalTimeSpent = initialTimeSpent + (initialDistToSegEnd - laneQueueLength) / velocity; //time to reach end of queue

		if (finalTimeSpent < params.secondsInTick)
		{
			addToQueue();
			params.elapsedSeconds = params.secondsInTick;
		}
		else
		{
			finalDistToSegEnd = initialDistToSegEnd - (velocity * (params.secondsInTick - initialTimeSpent));
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
			pathMover.setPositionInSegment(finalDistToSegEnd);
			params.elapsedSeconds = params.secondsInTick;

			double oldDistCovered = currLane->getParentSegment()->getLength() - initialDistToSegEnd;
			double acceleration = 2 * ((initialDistToSegEnd - finalDistToSegEnd) - (velocity * finalTimeSpent)) / (finalTimeSpent * finalTimeSpent);
			updateTrafficSensor(oldDistCovered, oldDistCovered + initialDistToSegEnd - finalDistToSegEnd, velocity, acceleration);
		}
	}
	else if (getInitialQueueLength(currLane) > 0)
	{
		res = advanceMovingVehicleWithInitialQ(params);
	}
	else //no queue or no initial queue
	{
		finalTimeSpent = initialTimeSpent + initialDistToSegEnd / velocity;
		if (finalTimeSpent < params.secondsInTick)
		{
			if (inOutCounter > 0)
			{
				pathMover.setPositionInSegment(0.0);
				params.elapsedSeconds = finalTimeSpent;
				res = moveToNextSegment(params);
			}
			else
			{
				addToQueue();
				params.elapsedSeconds = params.secondsInTick;
			}
		}
		else
		{
			finalTimeSpent = params.secondsInTick;
			finalDistToSegEnd = initialDistToSegEnd - (velocity * (finalTimeSpent - initialTimeSpent));
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
			pathMover.setPositionInSegment(finalDistToSegEnd);
			params.elapsedSeconds = finalTimeSpent;

			double oldDistCovered = currLane->getParentSegment()->getLength() - initialDistToSegEnd;
			double acceleration = 2 * ((initialDistToSegEnd - finalDistToSegEnd) - (velocity * finalTimeSpent)) / (finalTimeSpent * finalTimeSpent);
			updateTrafficSensor(oldDistCovered, oldDistCovered + initialDistToSegEnd - finalDistToSegEnd, velocity, acceleration);
		}
	}
	return res;
}

bool DriverMovement::advanceMovingVehicleWithInitialQ(DriverUpdateParams& params)
{
	bool res = false;
	double initialTimeSpent = params.elapsedSeconds;
	double initialDistToSegEnd = pathMover.getPositionInSegment();
	double finalTimeSpent = 0.0;
	double finalDistToSegEnd = 0.0;

	const LaneStats *laneStats = pathMover.getCurrSegStats()->getLaneStats().find(currLane)->second;
	double velocity = laneStats->getLaneVehSpeed(true);

	int inOutCounter = getOutputCounter(currLane, pathMover.getCurrSegStats());
	double outRate = getOutputFlowRate(currLane);

	const SegmentStats* nxtSegStat = pathMover.getNextSegStats(false);
	if (nxtSegStat)
	{
		const SegmentStats* nextToNextSegStat = pathMover.getSecondSegStatsAhead();
		const Lane* laneInNextSegment = getBestTargetLane(nxtSegStat, nextToNextSegStat);
		inOutCounter = std::min(inOutCounter, getInputCounter(laneInNextSegment, nxtSegStat));
	}

	//The following line of code assumes vehicle length is in cm;
	//vehicle length and outrate cannot be 0.
	//There was a magic factor 3.0 in the denominator. It was removed because
	//its purpose was not clear to anyone. ~Harish
	double timeToDissipateQ = getInitialQueueLength(currLane) / (outRate * PASSENGER_CAR_UNIT);
	double timeToReachEndSeg = initialTimeSpent + initialDistToSegEnd / velocity;
	finalTimeSpent = std::max(timeToDissipateQ, timeToReachEndSeg);

	if (finalTimeSpent < params.secondsInTick)
	{
		if (inOutCounter > 0)
		{
			pathMover.setPositionInSegment(0.0);
			params.elapsedSeconds = finalTimeSpent;
			res = moveToNextSegment(params);
		}
		else
		{
			addToQueue();
			params.elapsedSeconds = params.secondsInTick;
		}
	}
	else
	{
		if (fabs(finalTimeSpent - timeToReachEndSeg) < INFINITESIMAL_DOUBLE
				&& timeToReachEndSeg > params.secondsInTick)
		{
			finalTimeSpent = params.secondsInTick;
			finalDistToSegEnd = initialDistToSegEnd - velocity * (finalTimeSpent - initialTimeSpent);
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
		}
		else
		{
			finalDistToSegEnd = 0.0;
			res = moveInSegment(initialDistToSegEnd - finalDistToSegEnd);
			finalTimeSpent = params.secondsInTick;
		}

		pathMover.setPositionInSegment(finalDistToSegEnd);
		params.elapsedSeconds = finalTimeSpent;
	}
	return res;
}

int DriverMovement::getOutputCounter(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->getOutputCounter();
}

void DriverMovement::decrementOutputCounter(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->decrementOutputCounter();
}

int DriverMovement::getInputCounter(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->getInputCounter();
}

void DriverMovement::decrementInputCounter(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->decrementInputCounter();
}

double DriverMovement::getOutputFlowRate(const Lane* lane)
{
	return pathMover.getCurrSegStats()->getLaneParams(lane)->getOutputFlowRate();
}

double DriverMovement::getAcceptRate(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->getAcceptRate();
}

double DriverMovement::getQueueLength(const Lane* lane)
{
	return pathMover.getCurrSegStats()->getLaneQueueLength(lane);
}

double DriverMovement::getLastAccept(const Lane* lane, const SegmentStats* segStats)
{
	return segStats->getLaneParams(lane)->getLastAccept();
}

void DriverMovement::setLastAccept(const Lane* lane, double lastAccept, const SegmentStats* segStats)
{
	segStats->getLaneParams(lane)->setLastAccept(lastAccept);
}

void DriverMovement::updateFlow(const SegmentStats* segStats, double startPos, double endPos)
{
	double mid = segStats->getLength() / 2.0;
	const RoadSegment* rdSeg = segStats->getRoadSegment();
	if (startPos >= mid && mid >= endPos)
	{
		segStats->getParentConflux()->incrementSegmentFlow(rdSeg, segStats->getStatsNumberInSegment());
	}
}

void DriverMovement::setOrigin(DriverUpdateParams& params)
{
	if (params.now.ms() < parentDriver->parent->getStartTime())
	{
		stepFwdInTime(params, (parentDriver->parent->getStartTime() - params.now.ms()) / 1000.0); //set time to start - to accommodate drivers starting during the frame
	}

	// here the person tries to move into a proper lane in the current segstats from lane infinity
	const SegmentStats* currSegStats = pathMover.getCurrSegStats();
	const SegmentStats* nextSegStats = nullptr;
	if (pathMover.hasNextSegStats(true))
	{
		nextSegStats = pathMover.getNextSegStats(true);
	}
	else if (pathMover.hasNextSegStats(false))
	{
		nextSegStats = pathMover.getNextSegStats(false);
	}

	const Lane* laneInNextSegment = getBestTargetLane(currSegStats, nextSegStats);
	const LaneStats *laneInNextSegStats = currSegStats->getLaneStats().find(laneInNextSegment)->second;

	//this will space out the drivers on the same lane, by separating them by the time taken for the previous car to move a car's length
	double departTime = getLastAccept(laneInNextSegment, currSegStats);
	/*if(!currSegStats->isShortSegment())
	{
		if(currSegStats->hasQueue())
		{
			departTime += getAcceptRate(laneInNextSegment, currSegStats); //in seconds
		}
		else
		{
			departTime += (PASSENGER_CAR_UNIT / laneInNextSegStats->getLaneVehSpeed(true));
		}
	}*/

	params.elapsedSeconds = std::max(params.elapsedSeconds, departTime - (convertToSeconds(params.now.ms()))); //in seconds

	const Link* nextLink = getNextLinkForLaneChoice(currSegStats);
	if (canGoToNextRdSeg(params, currSegStats, nextLink))
	{
		//set position to start
		if (currSegStats)
		{
			pathMover.setPositionInSegment(currSegStats->getLength());
		}
		currLane = laneInNextSegment;
		double actualT = params.elapsedSeconds + (convertToSeconds(params.now.ms()));
		try
		{
			parentDriver->parent->currLinkTravelStats.start(currSegStats->getRoadSegment()->getParentLink(), actualT);
		}
		catch (const std::runtime_error &e)
		{
			throw std::runtime_error(
					"Error in the movement of driver " + parentDriver->parent->getDatabaseId() + ":" + e.what());
		}
		nextSurveillanceStn = currLane->getParentSegment()->getSurveillanceStations().begin();

		setLastAccept(currLane, actualT, currSegStats);
		setParentData(params);
		parentDriver->parent->canMoveToNextSegment = Person_MT::NONE;
		const Role<Person_MT>::Type  rType = getParentDriver()->roleType;
		if (rType != Role<Person_MT>::RL_BUSDRIVER && rType != Role<Person_MT>::RL_TAXIDRIVER &&
		    rType != Role<Person_MT>::RL_ON_HAIL_DRIVER && rType != Role<Person_MT>::RL_ON_CALL_DRIVER)
		{
			//initialize some travel metrics for this subTrip
			startTravelTimeMetric(); //not for bus drivers or any other role
		}
		updateLinkStats(nullptr);
	}
	else
	{
		params.elapsedSeconds = params.secondsInTick;
		parentDriver->parent->setRemainingTimeThisTick(0.0); //(elapsed - seconds this tick)
	}
}

void DriverMovement::addToQueue()
{
	/* 1. set position to queue length in front
	 * 2. set isQueuing = true
	 */
	if (parentDriver->parent)
	{
		if (!parentDriver->parent->isQueuing)
		{
			pathMover.setPositionInSegment(getQueueLength(currLane));
			isQueuing = true;
			parentDriver->parent->isQueuing = isQueuing;
		}
		else
		{
			DebugStream << "addToQueue() was called for a driver who is already in queue. Person: " << parentDriver->parent->getId()
					<< "|RoadSegment: " << currLane->getParentSegment()->getRoadSegmentId()
					<< "|Lane: " << currLane->getLaneId() << std::endl;
			throw std::runtime_error(DebugStream.str());
		}
	}
}

void DriverMovement::removeFromQueue()
{
	if (parentDriver->parent)
	{
		if (parentDriver->parent->isQueuing)
		{
			parentDriver->parent->isQueuing = false;
			isQueuing = false;
		}
		else
		{
			DebugStream << "removeFromQueue() was called for a driver who is not in queue. Person: " << parentDriver->parent->getId()
					<< "|RoadSegment: " << currLane->getParentSegment()->getRoadSegmentId()
					<< "|Lane: " << currLane->getLaneId() << std::endl;
			throw std::runtime_error(DebugStream.str());
		}
	}
}

const Lane* DriverMovement::getBestTargetLane(const SegmentStats* nextSegStats, const SegmentStats* nextToNextSegStats)
{
	if (!nextSegStats)
	{
		return nullptr;
	}
	const Lane* minLane = nullptr;
	double minQueueLength = std::numeric_limits<double>::max();
	double minLength = std::numeric_limits<double>::max();
	int outputCounter = 0;
	int maxOutputCounter = 0;
	double queueLength = 0.0;
	double totalLength = 0.0;
	int hasInputCounter = 0;
	int curHasinputCounter = 0;

	const Link* nextLink = getNextLinkForLaneChoice(nextSegStats);
	const std::vector<const Lane*>& lanes = nextSegStats->getRoadSegment()->getLanes();
	for (vector<const Lane* >::const_iterator lnIt = lanes.begin(); lnIt != lanes.end(); ++lnIt)
	{
		const Lane* lane = *lnIt;
		if (!lane->isPedestrianLane())
		{
			if (!laneConnectorOverride
					&& nextToNextSegStats
					&& !isConnectedToNextSeg(lane, nextToNextSegStats->getRoadSegment())
					&& nextLink
					&& !nextSegStats->isConnectedToDownstreamLink(nextLink, lane))
			{
				continue;
			}

			totalLength = nextSegStats->getLaneTotalVehicleLength(lane);
			queueLength = nextSegStats->getLaneQueueLength(lane);
			outputCounter = nextSegStats->getLaneParams(lane)->getOutputCounter();
			curHasinputCounter = nextSegStats->getLaneParams(lane)->getInputCounter() > 0 ? 1 : 0;
			if ((curHasinputCounter > hasInputCounter) ||
				((curHasinputCounter == hasInputCounter) &&
				 (minLength > totalLength || (minLength == totalLength && outputCounter > maxOutputCounter))))
			{
				//Choose lane with lower number of vehicles on it as both temp chosen lane
				// current lane have no queue
				minLength = totalLength;
				minQueueLength = queueLength;
				minLane = lane;
				maxOutputCounter = outputCounter;
				if (curHasinputCounter > hasInputCounter)
				{
					hasInputCounter = curHasinputCounter;
				}
			}

			/*if(queueLength == 0)
			{
				if (minQueueLength == 0)
				{
					if (minLength > totalLength)
					{
						//Choose lane with lower number of vehicles on it as both temp chosen lane
						// current lane have no queue
						minLength = totalLength;
						minQueueLength = queueLength;
						minLane = lane;
					}
				}
				else
				{
					// as the temp chosen lane has queue and the current lane does not
					// we choose current one
					minLength = totalLength;
					minQueueLength = queueLength;
					minLane = lane;
				}
			}
			else
			{
				// current lane has a queue
				if(minQueueLength > queueLength)
				{
					//Choose lane with lower queue length
					minLength = totalLength;
					minQueueLength = queueLength;
					minLane = lane;
				}
				else if(minQueueLength == queueLength)
				{
					//In case of a tie, use the one with smaller total length
					if(minLength > totalLength)
					{
						minLength = totalLength;
						minQueueLength = queueLength;
						minLane = lane;
					}
				}
				else if(minQueueLength == queueLength)
				{
					//In case of a tie, use the one with smaller total length
					if(minLength > totalLength)
					{
						minLength = totalLength;
						minQueueLength = queueLength;
						minLane = lane;
					}
				}
			}*/
		}
	}

	if (!minLane)
	{
		Print() << "\nCurrent Path " << pathMover.getPath().size() << std::endl;
		Print() << MesoPathMover::getPathString(pathMover.getPath());

		std::ostringstream out("");
		out << "best target lane was not set!" << "\nCurrent Segment: " << pathMover.getCurrSegStats()->getRoadSegment()->getRoadSegmentId() <<
				" =>" << nextSegStats->getRoadSegment()->getRoadSegmentId() <<
				" =>" << nextToNextSegStats->getRoadSegment()->getRoadSegmentId() << std::endl;
		out << "firstSegInNextLink:" << (nextLink ? nextLink->getRoadSegments().front()->getRoadSegmentId() : 0)
				<< "|NextLink: " << (nextLink ? nextLink->getLinkId() : 0)
				<< "|downstreamLinks of " << nextSegStats->getRoadSegment()->getRoadSegmentId() << std::endl;

		Print() << out.str();
		nextSegStats->printDownstreamLinks();
		throw std::runtime_error(out.str());
	}
	return minLane;
}

double DriverMovement::getInitialQueueLength(const Lane* lane)
{
	return pathMover.getCurrSegStats()->getInitialQueueLength(lane);
}

void DriverMovement::updateLinkStats(const SegmentStats* prevSegStat)
{
	if(prevSegStat)
	{
		const RoadSegment* prevSeg = prevSegStat->getRoadSegment();
		const Link* prevLink = prevSeg->getParentLink();
		LinkStats& prevLnkStats = prevSegStat->getParentConflux()->getLinkStats(prevLink);
		prevLnkStats.removeEntitiy(parentDriver->getParent());
	}

	const SegmentStats* currSegStat = pathMover.getCurrSegStats();
	if (currSegStat)
	{
		const Link* currLink = currSegStat ? currSegStat->getRoadSegment()->getParentLink() : nullptr;
		LinkStats& currLnkStats = currSegStat->getParentConflux()->getLinkStats(currLink);
		currLnkStats.addEntity(parentDriver->parent);
	}
}

void DriverMovement::updateLinkTravelTimes(const SegmentStats* prevSegStat, double linkExitTimeSec)
{
	const RoadSegment* prevSeg = prevSegStat->getRoadSegment();
	const SegmentStats* currSegStats = pathMover.getCurrSegStats();
	const Link* prevLink = prevSeg->getParentLink();
	const Link* currLink = currSegStats ? currSegStats->getRoadSegment()->getParentLink() : nullptr;
	if (prevLink == parentDriver->parent->currLinkTravelStats.link)
	{
		parentDriver->parent->currLinkTravelStats.finalize(prevLink, linkExitTimeSec, currLink);
		TravelTimeManager::getInstance()->addTravelTime(parentDriver->parent->currLinkTravelStats); //in seconds
		prevSegStat->getParentConflux()->setLinkTravelTimes(linkExitTimeSec, prevLink);
	}
	//creating a new entry in agent's travelStats for the new link, with entry time
	parentDriver->parent->currLinkTravelStats.reset();
	parentDriver->parent->currLinkTravelStats.start(currLink, linkExitTimeSec);
}

void DriverMovement::updateScreenlineCounts(const SegmentStats* prevSegStat, double segEnterExitTime)
{
	MT_Config& mtConfig = MT_Config::getInstance();
	if(!mtConfig.screenLineParams.outputEnabled) //double check for SceenLine Enabled
	{
		return;
	}
	Person_MT *parent = parentDriver->parent;
	const TripChainItem* tripChain = *(parent->currTripChainItem);
	const std::string& travelMode = tripChain->getMode();
	ScreenLineCounter::getInstance()->updateScreenLineCount(prevSegStat->getRoadSegment()->getRoadSegmentId(), segEnterExitTime, travelMode);
}

void DriverMovement::updateTrafficSensor(double oldPos, double newPos, double speed, double acceleration)
{
	/*
	 * Occupance is calculated when the vehicle is present in the detection zone of a sensor. Flow counts, instantaneous speed,
	 * and other point data are calculate when the back bumper crosses the down-edge of the detection zone.
	 * NOTE: We do not count a vehicle until its back-bumper leaves the detection zone of the sensor.
	 * This is to avoid double counting.
	 */

	//Get the surveillance stations in the segment
	const vector<SurveillanceStation *> &survStns = currLane->getParentSegment()->getSurveillanceStations();
	vector<SurveillanceStation *>::const_iterator itSurvStn = nextSurveillanceStn;

	//Vehicle front and back bumper positions
	double halfVehicleLen = (parentDriver->getResource()->getLengthInM() / 2);
	unsigned int vehBackBumper = newPos > halfVehicleLen ? newPos - halfVehicleLen : 0;
	unsigned int vehFrontBumper = newPos + halfVehicleLen;

	unsigned int laneIdx = currLane->getLaneIndex();

	//Some data items such as occupancy are accumulated if the entire or a part of the vehicle was/is in the detection zone.
	while((itSurvStn != survStns.end()) && (vehFrontBumper - (*itSurvStn)->getOffsetDistance() <= (*itSurvStn)->getZoneLength()))
	{
		TrafficSensor *sensor = (*itSurvStn)->getTrafficSensor(laneIdx);
		sensor->calculateActivatingData(oldPos, parentDriver->getResource()->getLengthInM(), speed, acceleration, parentDriver->getParams().now.ms());

		++itSurvStn;
	}

	//Other data items such as flow and headways are counted when the back bumper of the vehicle crosses the
	//down-edge of the of detection zone
	while((nextSurveillanceStn != survStns.end()) && (vehBackBumper > (*nextSurveillanceStn)->getOffsetDistance() + (*nextSurveillanceStn)->getZoneLength()))
	{
		TrafficSensor *sensor = (*nextSurveillanceStn)->getTrafficSensor(laneIdx);
		sensor->calculatePassingData(oldPos, parentDriver->getResource()->getLengthInM(), speed, acceleration);

		++nextSurveillanceStn;
	}
}

TravelMetric & DriverMovement::startTravelTimeMetric()
{
	std::string now((DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime()).getStrRepr());
	travelMetric.startTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	const Node* startNode = (*(pathMover.getPath().begin()))->getRoadSegment()->getParentLink()->getFromNode();
	travelMetric.origin = WayPoint(startNode);
	travelMetric.started = true;

	//cbd
	travelMetric.cbdTraverseType = parentDriver->parent->currSubTrip->cbdTraverseType;
	switch (travelMetric.cbdTraverseType)
	{
	case TravelMetric::CBD_ENTER:
		break;
	case TravelMetric::CBD_EXIT:
		travelMetric.cbdOrigin = travelMetric.origin;
		travelMetric.cbdStartTime = travelMetric.startTime;
		break;
	};
	return travelMetric;
}

TravelMetric& DriverMovement::finalizeTravelTimeMetric()
{
	if (!travelMetric.started)
	{
		return travelMetric;
	} //sanity check
	if (pathMover.getPath().empty())
	{
		Print() << "Person " << parentDriver->parent->getId() << " has no path\n";
		return travelMetric;
	}

	const SegmentStats * currSegStat = ((pathMover.getCurrSegStats() == nullptr) ? *(pathMover.getPath().rbegin()) : (pathMover.getCurrSegStats()));
	const Node* endNode = currSegStat->getRoadSegment()->getParentLink()->getToNode();
	travelMetric.destination = WayPoint(endNode);
	travelMetric.endTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
	travelMetric.travelTime = TravelMetric::getTimeDiffHours(travelMetric.endTime, travelMetric.startTime);
	travelMetric.finalized = true;

	//cbd
	switch(travelMetric.cbdTraverseType)
	{
	case TravelMetric::CBD_ENTER:
		travelMetric.cbdDestination = travelMetric.destination;
		travelMetric.cbdEndTime = travelMetric.endTime;
		travelMetric.cbdTravelTime = TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime, travelMetric.cbdStartTime);
		break;
	case TravelMetric::CBD_EXIT:
		break;
	};

	//	if(travelMetric.cbdTraverseType == TravelMetric::CBD_ENTER ||
	//			travelMetric.cbdTraverseType == TravelMetric::CBD_EXIT)
	//	{
	//		parent->serializeCBD_SubTrip(*travelMetric);
	//	}
	//	parent->addSubtripTravelMetrics(*travelMetric);
	return travelMetric;
}

TravelMetric& DriverMovement::processCBD_TravelMetrics(const Link* completedLink, const Link* nextLink)
{
	if (parentDriver->roleType == Role<Person_MT>::Type::RL_BUSDRIVER ||
	    parentDriver->roleType == Role<Person_MT>::Type::RL_TAXIDRIVER ||
	    parentDriver->roleType == Role<Person_MT>::Type::RL_ON_HAIL_DRIVER ||
	    parentDriver->roleType == Role<Person_MT>::Type::RL_ON_CALL_DRIVER)
	{
		travelMetric.cbdTraverseType = TravelMetric::CBD_NONE;
		return travelMetric;
	}
	
	//	the following conditions should hold in order to process CBD data
	travelMetric.cbdTraverseType = (*(parentDriver->parent->currSubTrip)).cbdTraverseType;
	bool proceed = (nextLink && !pathMover.isPathCompleted());
	
	if (!proceed)
	{
		return travelMetric;
	}

	std::string now((DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime()).getStrRepr());
	RestrictedRegion &cbd = RestrictedRegion::getInstance();

	//	update travel distance
	if (cbd.isInRestrictedZone(completedLink))
	{
		travelMetric.cbdDistance += completedLink->getLength();
	}

	//process either enter or exit
	switch (travelMetric.cbdTraverseType)
	{
	case TravelMetric::CBD_ENTER:
	{
		//search if you are about to enter CBD (we assume the trip started outside cbd and  is going to end inside cbd)
		if (!cbd.isInRestrictedZone(completedLink) && cbd.isInRestrictedZone(nextLink) && travelMetric.cbdEntered.check())
		{
			travelMetric.cbdOrigin = WayPoint(completedLink->getToNode());
			travelMetric.cbdStartTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
		}
		break;
	}
	case TravelMetric::CBD_EXIT:
	{
		//search if you are about to exit CBD(we assume the trip started inside cbd and is going to end outside cbd)
		if (cbd.isInRestrictedZone(completedLink) && !cbd.isInRestrictedZone(nextLink) && travelMetric.cbdExitted.check())
		{
			travelMetric.cbdDestination = WayPoint(completedLink->getToNode());
			travelMetric.cbdEndTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
			travelMetric.cbdTravelTime = TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime, travelMetric.cbdStartTime);
		}
		break;
	}
	case TravelMetric::CBD_PASS:
	{
		if(!cbd.isInRestrictedZone(completedLink) && cbd.isInRestrictedZone(nextLink))
		{
			travelMetric.cbdOrigin = WayPoint(completedLink->getToNode());
			travelMetric.cbdStartTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
		}
		if(cbd.isInRestrictedZone(completedLink)&&!cbd.isInRestrictedZone(nextLink))
		{
			travelMetric.cbdDestination = WayPoint(completedLink->getToNode());
			travelMetric.cbdEndTime = DailyTime(getParentDriver()->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime();
			travelMetric.cbdTravelTime = TravelMetric::getTimeDiffHours(travelMetric.cbdEndTime , travelMetric.cbdStartTime);
		}
		break;
	}
	}
	return travelMetric;
}

int DriverMovement::findReroutingPoints(const RoadSegment* rdSeg,
										std::map<const Node*, std::vector<const SegmentStats*> >& remaining) const
{

	//some variables and iterators before the Actual Operation
	const std::vector<const SegmentStats*> & path = getMesoPathMover().getPath(); //driver's current path
	const SegmentStats* incidentSegStat = Conflux::getConflux(rdSeg)->findSegStats(rdSeg, 1);
	std::vector<const SegmentStats*>::const_iterator startIt = std::find(path.begin(), path.end(), getMesoPathMover().getCurrSegStats()); //iterator to driver's current location
	std::vector<const SegmentStats*>::const_iterator endIt = std::find(path.begin(), path.end(), incidentSegStat); //iterator to incident segstat
	std::vector<const SegmentStats*> rem; //stats remaining from the current location to the re-routing point
	//Actual Operation : As you move from your current location towards the incident, store the intersections on your way + the segstats you travrsed until you reach that intersection.
	//	//debug
	//	pathsetLogger << "Original Path:" << std::endl;
	//	MesoPathMover::printPath(path);
	//	//debug...
	for (const Link * currLink = (*startIt)->getRoadSegment()->getParentLink(); startIt <= endIt; startIt++)
	{
		//record the remaining segstats
		rem.push_back(*startIt);
		//link changed?
		if (currLink != (*startIt)->getRoadSegment()->getParentLink())
		{
			//record
			remaining[currLink->getToNode()] = rem; //no need to clear rem!
			//last segment lies in the next link, remove it
			remaining[currLink->getToNode()].pop_back();
			//update the current iteration link
			currLink = (*startIt)->getRoadSegment()->getParentLink();
		}
	}
	//filter out no paths
	std::map<const Node*, std::vector<const SegmentStats*> >::iterator noPathIt = remaining.begin();
	while (noPathIt != remaining.end())
	{
		if (!(noPathIt->second.size()))
			remaining.erase(noPathIt++);
		else
			noPathIt++;
	}

	return remaining.size();
}

bool DriverMovement::canJoinPaths(std::vector<WayPoint> & newPath, std::vector<const SegmentStats*> & oldPath,
								   SubTrip &subTrip, std::set<const Link*> & excludeLink)
{
	throw std::runtime_error("DriverMovement::canJoinPaths not implemented for new road network");

//	const RoadSegment *from = (*oldPath.rbegin())->getRoadSegment(); //using .begin() or .end() makes no difference
//	const RoadSegment *to = newPath.begin()->roadSegment;
//	if (isConnectedToNextSeg(from, to))
//	{
//		return true;
//	}
//	//now try to find another path
//	//	MesoPathMover::printPath(oldPath);
//	//	printWPpath(newPath);
//
//	//exclude/blacklist the segment on the new path(first segment)
//	excludeLink.insert((*newPath.begin()).link);
//	//create a path using updated black list
//	//and then try again
//	//try to remove UTurn by excluding the segment (in the new part of the path) from the graph and regenerating pathset
//	//if no path, return false, if path found, return true
//	PrivateTrafficRouteChoice::getInstance()->getBestPath(newPath,subTrip, true, excludeLink,false,false,false,nullptr);
//	to = newPath.begin()->roadSegment;
//	bool res = isConnectedToNextSeg(from, to);
//	return res;
}

//todo put this in the utils(and code style!)
boost::mt19937 myOwngen;

int roll_die(int l, int r)
{
	boost::uniform_int<> dist(l, r);
	boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(myOwngen, dist);
	return die();
}

void DriverMovement::reroute()
{
	rerouter->reroute();
}

//step-1: can I rerout? if yes, what are my points of rerout?
//step-2: do I 'want' to reroute?
//step-3: get a new path from each candidate re-routing points
//step-4: In order to get to the detour point, some part of the original path should still be traveled. prepend that part to the new paths
//setp-5: setpath: assign the assembled path to pathmover
void DriverMovement::reroute(const InsertIncidentMessage &msg)
{
	//step-1
	std::map<const Node*, std::vector<const SegmentStats*> > deTourOptions; //< detour point, segments to travel before getting to the detour point>
	deTourOptions.clear(); // :)
	int numReRoute = findReroutingPoints(msg.affectedSegment, deTourOptions);
	if (!numReRoute)
	{
		return;
	}

	//step-2
	if (!wantReRoute())
	{
		return;
	}
	//pathsetLogger << numReRoute << "Rerouting Points were identified" << std::endl;
	//step-3:
	typedef std::map<const Node*, std::vector<const SegmentStats*> >::value_type DetourOption; //for 'deTourOptions' container
	std::set<const Link*> excludeLink = std::set<const Link*>();
	//	get a 'copy' of the person's current subtrip
	SubTrip subTrip = *(parentDriver->parent->currSubTrip);
	std::map<const Node*, std::vector<WayPoint> > newPaths; //stores new paths starting from the re-routing points

	BOOST_FOREACH(DetourOption detourNode, deTourOptions)
	{
		// change the origin
		//todo and the start time !!!-vahid
		subTrip.origin.node = detourNode.first;
		//	record the new paths using the updated subtrip. (including no paths)
		PrivateTrafficRouteChoice::getInstance()->getBestPath(newPaths[detourNode.first], subTrip,true, std::set<const Link*>(), false,false,false,nullptr);//partially excluded sections must be already added
	}

	/*step-4: prepend the old path to the new path
	 * old path: part of the originalpathsetLogger path from the agent's current position to the rerouting point
	 * new path:the path from the rerouting point to the destination
	 * Note: it is more efficient to do this within the above loop but code reading will become more tough*/
	//4.a: check if there is no path from the rerouting point, just discard it.
	//4.b: check and discard the rerouting point if the new and old paths can be joined
	//4.c convert waypoint to segstat and prepend(join) remaining oldpath to the new path
	typedef std::map<const Node*, std::vector<WayPoint> >::value_type NewPath;

	BOOST_FOREACH(NewPath &newPath, newPaths)
	{
		//4.a
		if (newPath.second.empty())
		{
			Warn() << "No path on Detour Candidate node " << newPath.first->getNodeId() << std::endl;
			deTourOptions.erase(newPath.first);
			continue;
		}
		//4.b
		// change the origin
		subTrip.origin.node = newPath.first;
		//		MesoPathMover::printPath(deTourOptions[newPath.first], newPath.first);
		//		printWPpath(newPath.second, newPath.first);
		//check if join possible
		bool canJoin = canJoinPaths(newPath.second, deTourOptions[newPath.first], subTrip, excludeLink);
		if (!canJoin)
		{
			//			printWPpath(newPath.second, newPath.first);
			deTourOptions.erase(newPath.first);
			continue;
		}
		//pathsetLogger << "Paths can Join" << std::endl;
		//4.c join
		initSegStatsPath(newPath.second, deTourOptions[newPath.first]);

		//step-4.d cancel similar paths
		//some newPath(s) can be subset of the other path(s).
		//This can be easily detected when the old part of path and the new path join: it can create a combination that has already been created
		//so let's look for 'same paths':
		std::vector<const SegmentStats*> & target = deTourOptions[newPath.first];

		BOOST_FOREACH(DetourOption &detourNode, deTourOptions)
		{
			//dont compare with yourself
			if (detourNode.first == newPath.first)
			{
				continue;
			}
			if (target == detourNode.second)
			{
				//pathsetLogger << "Discarding an already been created path:\n";
				//pathsetLogger << MesoPathMover::printPath(detourNode.second);
				//pathsetLogger << MesoPathMover::printPath(target);
				deTourOptions.erase(newPath.first);
			}
			//			//if they have a different size, they are definitely different,so leave this entry alone
			//			if(target.size() != detourNode.second.size()){continue;}
			//			typedef std::vector<const SegmentStats*>::const_iterator it_;
			//			std::pair<it_,it_> comp = std::mismatch(target.begin(),target.end(), detourNode.second.begin(), detourNode.second.end());
			//since the two containers have the same size, they are considered equal(same) if any element of the above pair is equal to the .end() of their corresponding containers
			//			if (comp.first == target.end())
			//			{
			//				pathsetLogger << "Discarding an already been created path:" << std::endl;
			//				MesoPathMover::printPath(detourNode.second);
			//				MesoPathMover::printPath(target);
			//				deTourOptions.erase(newPath.first);
			//			}
		}
	}
	//is there any place drivers can re-route or not?
	if (!deTourOptions.size())
	{
		return;
	}

	//step-5: now you may set the path using 'deTourOptions' container
	//todo, put a distribution function here. For testing now, give it the last new path for now
	std::map<const Node*, std::vector<const SegmentStats*> >::iterator it(deTourOptions.begin());

	int cnt = roll_die(0, deTourOptions.size() - 1);
	int dbgIndx = cnt;
	while (cnt)
	{
		it++;
		--cnt;
	}
	//debug
//	pathsetLogger << "----------------------------------\n"
//			"Original path:" << std::endl;
//	pathsetLogger << getMesoPathMover().printPath(getMesoPathMover().getPath());
//	pathsetLogger << "Detour option chosen[" << dbgIndx << "] : " << it->first->getNodeId() << std::endl;
//	pathsetLogger << getMesoPathMover().printPath(it->second);
//	pathsetLogger << "----------------------------------" << std::endl;
	//debug...
	getMesoPathMover().setPath(it->second);
}

Conflux* DriverMovement::getStartingConflux() const
{
	const SegmentStats* firstSegStats = pathMover.getCurrSegStats(); //first segstats of the remaining path.
	if(!firstSegStats)
	{
		return nullptr;
	}
	return firstSegStats->getParentConflux();
}

void DriverMovement::handleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case MSG_INSERT_INCIDENT:
	{
		const InsertIncidentMessage &msg = MSG_CAST(InsertIncidentMessage, message);
		PrivateTrafficRouteChoice::getInstance()->addPartialExclusion(msg.affectedSegment);
		reroute(msg);
		break;
	}
	}
}

const Link* DriverMovement::getNextLinkForLaneChoice(const SegmentStats* nextSegStats) const
{
	const Link* nextLink = nullptr;

	const SegmentStats* firstStatsInNextLink = pathMover.getFirstSegStatsInNextLink(nextSegStats);
	if (firstStatsInNextLink)
	{
		nextLink = firstStatsInNextLink->getRoadSegment()->getParentLink();
	}
	return nextLink;
}

bool DriverMovement::ifLoopedNode(unsigned int thisNodeId)
{
	auto loopNodesSet = RoadNetwork::getInstance()->getSetOfLoopNodesInNetwork();
	bool found = false;
	std::unordered_set<unsigned int>::const_iterator loopNodeItr;
	loopNodeItr = loopNodesSet.find(thisNodeId);
	if (loopNodeItr != loopNodesSet.end())
	{
		found = true;
	}
	return found;
}

} /* namespace medium */
} /* namespace sim_mob */


