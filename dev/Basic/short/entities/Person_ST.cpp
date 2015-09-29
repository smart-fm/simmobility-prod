//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_ST.hpp"
#include "entities/amodController/AMODController.hpp"

using namespace sim_mob;

namespace
{

Trip* MakePseudoTrip(const Person& ag, const std::string& mode)
{
	//Make sure we have something to work with
	if (!(ag.originNode.node_ && ag.destNode.node_))
	{
		std::stringstream msg;
		msg << "Can't make a pseudo-trip for an Agent with no origin and destination nodes: " << ag.originNode.node_ << " , " << ag.destNode.node_;
		throw std::runtime_error(msg.str().c_str());
	}

	//Make the trip itself
	Trip* res = new Trip();
	res->setPersonID(ag.getId());
	res->itemType = TripChainItem::getItemType("Trip");
	res->sequenceNumber = 1;
	res->startTime = DailyTime(ag.getStartTime()); //TODO: This may not be 100% correct
	res->endTime = res->startTime; //No estimated end time.
	res->tripID = "";
	res->origin = WayPoint(ag.originNode);
	res->originType = TripChainItem::getLocationType("node");
	res->destination = WayPoint(ag.destNode);
	res->destinationType = res->originType;
	res->travelMode = mode;

	//Make and assign a single sub-trip
	sim_mob::SubTrip subTrip;
	subTrip.setPersonID(-1);
	subTrip.itemType = TripChainItem::getItemType("Trip");
	subTrip.sequenceNumber = 1;
	subTrip.startTime = res->startTime;
	subTrip.endTime = res->startTime;
	subTrip.origin = res->origin;
	subTrip.originType = res->originType;
	subTrip.destination = res->destination;
	subTrip.destinationType = res->destinationType;
	subTrip.tripID = "";
	subTrip.mode = mode;
	subTrip.isPrimaryMode = true;
	subTrip.ptLineId = "";

	//Add it to the Trip; return this value.
	res->addSubTrip(subTrip);
	return res;
}
}

Person_ST::Person_ST(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID)
: Person(src, mtxStrat, id, databaseID), laneID(-1), boardingTimeSecs(0), alightingTimeSecs(0), amodId("-1"),
amodPickUpSegmentStr("-1"), amodPickUpOffset(0.0), initSegId(0), initDis(0), initSpeed(0), amodDropOffset(0)
{
}

Person_ST::Person_ST(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Person(src, mtxStrat, tc), laneID(-1), boardingTimeSecs(0), alightingTimeSecs(0), amodId("-1"),
amodPickUpSegmentStr("-1"), amodPickUpOffset(0.0), initSegId(0), initDis(0), initSpeed(0), amodDropOffset(0)
{
}

Person_ST::~Person_ST()
{
}

void Person_ST::setPath(std::vector<WayPoint>& path)
{
	if (path.size() == 0)
	{
		Print() << "Warning! Path size is zero!" << std::endl;
	}

	amodPath = path;
}

void Person_ST::setPersonCharacteristics()
{
	//Call the base class method
	Person::setPersonCharacteristics();
	
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::map<int, PersonCharacteristics>& personCharacteristics = config.personCharacteristicsParams.personCharacteristics;
	
	const int defaultLowerSecs = config.personCharacteristicsParams.DEFAULT_LOWER_SECS;
	const int defaultUpperSecs = config.personCharacteristicsParams.DEFAULT_UPPER_SECS;
	
	boost::mt19937 gen(static_cast<unsigned int> (getId() * getId()));
	
	boost::uniform_int<> BoardingTime(defaultLowerSecs, defaultUpperSecs);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varBoardingTime(gen, BoardingTime);
	boardingTimeSecs = varBoardingTime();

	boost::uniform_int<> AlightingTime(defaultLowerSecs, defaultUpperSecs);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAlightingTime(gen, AlightingTime);
	alightingTimeSecs = varAlightingTime();

	for (std::map<int, PersonCharacteristics>::const_iterator iter = personCharacteristics.begin(); iter != personCharacteristics.end(); ++iter)
	{
		if (age >= iter->second.lowerAge && age < iter->second.upperAge)
		{
			boost::uniform_int<> BoardingTime(iter->second.lowerSecs, iter->second.upperSecs);
			boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varBoardingTime(gen, BoardingTime);
			boardingTimeSecs = varBoardingTime();

			boost::uniform_int<> AlightingTime(iter->second.lowerSecs, iter->second.upperSecs);
			boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAlightingTime(gen, AlightingTime);
			alightingTimeSecs = varAlightingTime();
		}
	}
}

void Person_ST::handleAMODEvent(event::EventId id, event::Context ctxId, event::EventPublisher* sender, const AMOD::AMODEventArgs& args)
{
	if (id == event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH)
	{
		//role gets chance to handle event
		if (currRole)
		{
			currRole->onParentEvent(id, ctxId, sender, args);
		}
	}
}

void Person_ST::handleAMODArrival()
{
	//ask the AMODController to handle the arrival
	AMOD::AMODController::instance()->handleVHArrive(this);
}

void Person_ST::handleAMODPickup()
{
	//ask the AMODController to handle the arrival
	AMOD::AMODController::instance()->handleVHPickup(this);
}

void Person_ST::load(const map<string, string>& configProps)
{
	if (!tripChain.empty())
	{
		return; //already have a trip chain usually from generateFromTripChain, no need to load
	}

	//Make sure they have a mode specified for this trip
	map<string, string>::const_iterator it = configProps.find("#mode");
	if (it == configProps.end())
	{
		throw std::runtime_error("Cannot load person: no mode");
	}
	std::string mode = it->second;

	//Consistency check: specify both origin and dest
	if (configProps.count("originPos") != configProps.count("destPos"))
	{
		throw std::runtime_error("Agent must specify both originPos and destPos, or neither.");
	}

	std::map<std::string, std::string>::const_iterator lanepointer = configProps.find("lane");
	if (lanepointer != configProps.end())
	{
		try
		{
			int x = boost::lexical_cast<int>(lanepointer->second);
			laneID = x;
		}
		catch (boost::bad_lexical_cast const&)
		{
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initSegId
	std::map<std::string, std::string>::const_iterator itt = configProps.find("initSegId");
	if (itt != configProps.end())
	{
		try
		{
			int x = boost::lexical_cast<int>(itt->second);
			initSegId = x;
		}
		catch (boost::bad_lexical_cast const&)
		{
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initSegPer
	itt = configProps.find("initDis");
	if (itt != configProps.end())
	{
		try
		{
			int x = boost::lexical_cast<int>(itt->second);
			initDis = x;
		}
		catch (boost::bad_lexical_cast const&)
		{
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initPosSegPer
	itt = configProps.find("initSpeed");
	if (itt != configProps.end())
	{
		try
		{
			int x = boost::lexical_cast<int>(itt->second);
			initSpeed = x;
		}
		catch (boost::bad_lexical_cast const&)
		{
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}

	// node
	map<string, string>::const_iterator oriNodeIt = configProps.find("originNode");
	map<string, string>::const_iterator destNodeIt = configProps.find("destNode");
	if (oriNodeIt != configProps.end() && destNodeIt != configProps.end())
	{
		int originNodeId;
		int destNodeid;
		try
		{
			originNodeId = boost::lexical_cast<int>(oriNodeIt->second);
			destNodeid = boost::lexical_cast<int>(destNodeIt->second);
		}
		catch (boost::bad_lexical_cast const&)
		{
			Warn() << "Error: input string was not valid" << std::endl;
		}

		//Otherwise, make a trip chain for this Person.
		this->originNode = WayPoint(ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(originNodeId));
		this->destNode = WayPoint(ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(destNodeid));

		Trip* singleTrip = MakePseudoTrip(*this, mode);

		std::vector<TripChainItem*> trip_chain;
		trip_chain.push_back(singleTrip);

		//TODO: Some of this should be performed in a centralised place; e.g., "Agent::setTripChain"
		//TODO: This needs to go in a centralised place.
		this->originNode = singleTrip->origin;
		this->destNode = singleTrip->destination;
		this->setNextPathPlanned(false);
		this->setTripChain(trip_chain);
		this->initTripChain();
	}
	else
	{
		//Consistency check: are they requesting a pseudo-trip chain when they actually have one?
		map<string, string>::const_iterator origIt = configProps.find("originPos");
		map<string, string>::const_iterator destIt = configProps.find("destPos");
		
		if (origIt != configProps.end() && destIt != configProps.end())
		{
			//Double-check some potential error states.
			if (!tripChain.empty())
			{
				throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
			}
			if (this->originNode.node_ || this->destNode.node_)
			{
				throw std::runtime_error("Manual position specified for Agent with existing start and end of Trip Chain.");
			}

			//Otherwise, make a trip chain for this Person.
			this->originNode = WayPoint(ConfigManager::GetInstance().FullConfig().getNetwork().locateNode(parse_point(origIt->second), true));
			this->destNode = WayPoint(ConfigManager::GetInstance().FullConfig().getNetwork().locateNode(parse_point(destIt->second), true));

			Trip* singleTrip = MakePseudoTrip(*this, mode);

			std::vector<TripChainItem*> trip_chain;
			trip_chain.push_back(singleTrip);

			//TODO: Some of this should be performed in a centralized place; e.g., "Agent::setTripChain"
			//TODO: This needs to go in a centralized place.
			this->originNode = singleTrip->origin;
			this->destNode = singleTrip->destination;
			this->setNextPathPlanned(false);
			this->setTripChain(trip_chain);
			this->initTripChain();
		}
	}
}

void Person_ST::initTripChain()
{
	currTripChainItem = tripChain.begin();
	
	const std::string& src = getAgentSrc();
	if (src == "XML_TripChain" || src == "DAS_TripChain" || src == "AMOD_TripChain" || src == "BusController")
	{
		setStartTime((*currTripChainItem)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
	}
	else
	{
		setStartTime((*currTripChainItem)->startTime.getValue());
	}
	
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<sim_mob::Trip*> (*currTripChainItem))->getSubTripsRW()).begin();
		
		// if the first trip chain item is passenger, create waitBusActivityRole
		if (currSubTrip->mode == "BusTravel")
		{
			const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
			currRole = rf.createRole("waitBusActivity", this);
			nextRole = rf.createRole("passenger", this);
		}
		
		if (!updateOD(*currTripChainItem))
		{ 
			//Offer some protection
			throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
		}
	}

	setNextPathPlanned(false);
	isFirstTick = true;
}