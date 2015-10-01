//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_ST.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "event/args/ReRouteEventArgs.hpp"

using namespace sim_mob;

namespace
{

Trip* MakePseudoTrip(const Person &ag, const std::string &mode)
{
	//Make sure we have something to work with
	if (!(ag.originNode.node_ && ag.destNode.node_))
	{
		std::stringstream msg;
		msg << "Can't make a pseudo-trip for an Agent with no origin and destination nodes: " << ag.originNode.node_ << " , " << ag.destNode.node_;
		throw std::runtime_error(msg.str().c_str());
	}

	//Make the trip itself
	Trip *res = new Trip();
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

Person_ST::Person_ST(const std::string &src, const MutexStrategy &mtxStrat, int id, std::string databaseID)
: Person(src, mtxStrat, id, databaseID), laneID(-1), boardingTimeSecs(0), alightingTimeSecs(0), 
prevRole(NULL), currRole(NULL), nextRole(NULL), commEventRegistered(false), amodId("-1"),
amodPickUpSegmentStr("-1"), amodPickUpOffset(0.0), initSegId(0), initDis(0), initSpeed(0), amodDropOffset(0)
{
}

Person_ST::Person_ST(const std::string &src, const MutexStrategy &mtxStrat, const std::vector<TripChainItem *> &tc)
: Person(src, mtxStrat, tc), laneID(-1), boardingTimeSecs(0), alightingTimeSecs(0), 
prevRole(NULL), currRole(NULL), nextRole(NULL), commEventRegistered(false), amodId("-1"),
amodPickUpSegmentStr("-1"), amodPickUpOffset(0.0), initSegId(0), initDis(0), initSpeed(0), amodDropOffset(0)
{
}

Person_ST::~Person_ST()
{
	//Un-register event listeners.
	if (commEventRegistered)
	{
		messaging::MessageBus::UnSubscribeEvent(sim_mob::event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT, this, this);
	}
	
	safe_delete_item(prevRole);
	safe_delete_item(currRole);
	safe_delete_item(nextRole);
}

void Person_ST::setPath(std::vector<WayPoint> &path)
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
	
	const ConfigParams &config = ConfigManager::GetInstance().FullConfig();
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

void Person_ST::setStartTime(unsigned int value)
{
	sim_mob::Entity::setStartTime(value);
	if (currRole)
	{
		currRole->setArrivalTime(value + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
}

void Person_ST::load(const map<string, string> &configProps)
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

	//Consistency check: specify both origin and destination
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

		this->originNode = singleTrip->origin;
		this->destNode = singleTrip->destination;
		this->setNextPathPlanned(false);
		this->setTripChain(trip_chain);
		this->initTripChain();
	}
	else
	{
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
			const RoleFactory<Person_ST> *rf = RoleFactory<Person_ST>::getInstance();
			currRole = rf->createRole("waitBusActivity", this);
			nextRole = rf->createRole("passenger", this);
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

UpdateStatus Person_ST::checkTripChain()
{
	if (tripChain.empty())
	{
		return UpdateStatus::Done;
	}

	//advance the trip, sub-trip or activity....
	if (!isFirstTick)
	{
		if (!(advanceCurrentTripChainItem()))
		{
			return UpdateStatus::Done;
		}
	}
	
	//must be set to false whenever trip chain item changes. And it has to happen before a probable creation of (or changing to) a new role
	setNextPathPlanned(false);

	//Create a new Role based on the trip chain type
	updatePersonRole();

	//Update our origin/destination pair.
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{ 
		//put if to avoid & evade bus trips, can be removed when everything is ok
		updateOD(*currTripChainItem, &(*currSubTrip));
	}

	//currentTipchainItem or current sub-trip are changed
	//so OD will be changed too,
	//therefore we need to call frame_init regardless of change in the role
	unsetInitialized();

	//Create a return type based on the differences in these Roles
	vector<BufferedBase*> prevParams;
	vector<BufferedBase*> currParams;
	
	if (prevRole)
	{
		prevParams = prevRole->getSubscriptionParams();
	}
	
	if (currRole)
	{
		currParams = currRole->getSubscriptionParams();
	}
	
	if (isFirstTick && currRole)
	{
		currRole->setArrivalTime(startTime + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
	
	isFirstTick = false;

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return UpdateStatus(UpdateStatus::RS_CONTINUE, prevParams, currParams);
}

bool Person_ST::findPersonNextRole()
{
	if (!updateNextTripChainItem())
	{
		safe_delete_item(nextRole);
		return false;
	}

	//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
	//it's better to avoid possible errors (e.g., if the equality operator is defined)
	//by saving it until the next time tick.
	//safe_delete_item(prevRole);
	safe_delete_item(nextRole);
	const RoleFactory<Person_ST> *rf = RoleFactory<Person_ST>::getInstance();

	const sim_mob::TripChainItem* tci = *(this->nextTripChainItem);
	
	if (tci->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		nextRole = rf->createRole(tci, &(*nextSubTrip), this);
	}

	return true;
}

bool Person_ST::updatePersonRole()
{
	//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
	//it's better to avoid possible errors (e.g., if the equality operator is defined)
	//by saving it until the next time tick.
	safe_delete_item(prevRole);
	
	const RoleFactory<Person_ST> *rf = RoleFactory<Person_ST>::getInstance();
	const TripChainItem *tci = *(this->currTripChainItem);
	const SubTrip* subTrip = NULL;

	if (tci->itemType == TripChainItem::IT_TRIP)
	{
		subTrip = &(*currSubTrip);
	}

	if (!nextRole)
	{
		nextRole = rf->createRole(tci, subTrip, this);
	}

	changeRole();
	return true;
}

void Person_ST::changeRole()
{
	if (currRole)
	{
		currRole->setParent(nullptr);
		if (this->currWorkerProvider)
		{
			this->currWorkerProvider->stopManaging(currRole->getSubscriptionParams());
			this->currWorkerProvider->stopManaging(currRole->getDriverRequestParams().asVector());
		}
	}

	safe_delete_item(prevRole);
	prevRole = currRole;
	currRole = nextRole;
	nextRole = nullptr;

	if (currRole)
	{
		currRole->setParent(this);
		if (this->currWorkerProvider)
		{
			this->currWorkerProvider->beginManaging(currRole->getSubscriptionParams());
			this->currWorkerProvider->beginManaging(currRole->getDriverRequestParams().asVector());
		}
	}
}

vector<BufferedBase *> Person_ST::buildSubscriptionList()
{
	//First, add the x and y co-ordinates
	vector<BufferedBase *> subsList;
	subsList.push_back(&xPos);
	subsList.push_back(&yPos);

	//Now, add our own properties.
	if (this->getRole())
	{
		vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();

		//Append the subsList with all elements in roleParams
		subsList.insert(subsList.end(), roleParams.begin(), roleParams.end());
	}

	return subsList;
}

bool Person_ST::frame_init(timeslice now)
{
	messaging::MessageBus::RegisterHandler(this);
	currTick = now;

	//Reset the Region tracking data structures, if applicable.
	//regionAndPathTracker.reset();

	//Register for communication simulator messages, if applicable.
	if (!commEventRegistered && ConfigManager::GetInstance().XmlConfig().system.simulation.commsim.enabled)
	{
		commEventRegistered = true;
		messaging::MessageBus::SubscribeEvent(event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT, this, this);
	}

	//Agents may be created with a null Role and a valid trip chain
	if (!currRole)
	{
		setStartTime(now.ms());

		UpdateStatus res = checkTripChain();

		if (currRole)
		{
			currRole->setArrivalTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS());
		}

		//Nothing left to do?
		if (res.status == UpdateStatus::RS_DONE)
		{
			return false;
		}
	}

	//Failsafe
	if (!currRole)
	{
		std::ostringstream txt;
		txt << "Person " << this->getId() << " has no Role.";
		throw std::runtime_error(txt.str());
	}

	//Get an UpdateParams instance.
	currRole->make_frame_tick_params(now);

	//Now that the Role has been fully constructed, initialise it.
	if ((*currTripChainItem))
	{
		currRole->Movement()->frame_init();
	}
	
	return true;
}

Entity::UpdateStatus Person_ST::frame_tick(timeslice now)
{
	currTick = now;
	//TODO: Here is where it gets risky.
	if (resetParamsRequired)
	{
		currRole->make_frame_tick_params(now);
		resetParamsRequired = false;
	}

	Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

	if (!isToBeRemoved())
	{
		currRole->Movement()->frame_tick();
	}

	//If we're "done", try checking to see if we have any more items in our Trip Chain.
	// This is not strictly the right way to do things (we shouldn't use "isToBeRemoved()"
	// in this manner), but it's the easiest solution that uses the current API.
	//TODO: This section should technically go after frame_output(), but doing that
	//      (by overriding Person::update() and calling Agent::update() then inserting this code)
	//      will bring us outside the bounds of our try {} catch {} statement. We might move this
	//      statement into the worker class, but I don't want to change too many things
	//      about Agent/Person at once. ~Seth
	if (isToBeRemoved())
	{
		//Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
		setStartTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS());

		retVal = checkTripChain();

		if (currTripChainItem != tripChain.end())
		{
			sim_mob::TripChainItem* tcItem = *currTripChainItem;
			if (tcItem) // if currTripChain not end and has value, call frame_init and switching roles
			{
				if (tcItem->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
				{
					//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
					//since start time of the activity is usually later than what is configured initially,
					//we have to make adjustments so that the person waits for exact amount of time
					sim_mob::ActivityPerformer* ap = dynamic_cast<sim_mob::ActivityPerformer*> (currRole);
					ap->setActivityStartTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS()));
					ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS() + (tcItem->endTime.getValue() - tcItem->startTime.getValue())));
				}
				if (!isInitialized())
				{
					currRole->Movement()->frame_init();
					setInitialized(true); // set to be false so later no need to frame_init later
				}
			}
		}
	}

	return retVal;
}

void Person_ST::frame_output(timeslice now)
{
	//Save the output
	if (!isToBeRemoved())
	{
		currRole->Movement()->frame_tick_output();
	}

	resetParamsRequired = true;
}

bool Person_ST::advanceCurrentTripChainItem()
{
	if (currTripChainItem == tripChain.end()) /*just a harmless basic check*/
	{
		return false;
	}

	// current role (activity or sub-trip level role)[for now: only subtrip] is about to change, time to collect its movement metrics(even activity performer)
	if (currRole != nullptr)
	{
		TravelMetric currRoleMetrics = currRole->Movement()->finalizeTravelTimeMetric();
		currRole->Movement()->resetTravelTimeMetric(); //sorry for manual reset, just a precaution for now
		serializeSubTripChainItemTravelTimeMetrics(currRoleMetrics, currTripChainItem, currSubTrip);
	}

	//first check if you just need to advance the subtrip
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//don't advance to next tripchainItem immediately, check the subtrip first
		bool res = advanceCurrentSubTrip();

		//subtrip advanced successfully, no need to advance currTripChainItem
		if (res)
		{
			return res;
		}
	}

	//Trip is about the change, collect the Metrics	
	//if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	//{
	//	aggregateSubTripMetrics();
	//}
	//
	//serializeTripChainItem(currTripChainItem); 

	//do the increment
	++currTripChainItem;

	if (currTripChainItem == tripChain.end())
	{
		//but tripchain items are also over, get out !
		return false;
	}

	//so far, advancing the tripchainitem has been successful
	//Also set the currSubTrip to the beginning of trip , just in case
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = resetCurrSubTrip();
	}

	return true;
}

void Person_ST::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
	//Some events only matter if they are for us.
	if (ctxId == this)
	{
		if (eventId == event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT)
		{
			//Was communication simulator enabled for us? If so, start tracking Regions.
			Print() << "Enabling Region support for agent: " << this << "\n";
			enableRegionSupport();

			//This requires us to now listen for a new set of events.
			messaging::MessageBus::SubscribeEvent(sim_mob::event::EVT_CORE_COMMSIM_REROUTING_REQUEST,
												this, //Only when we are the Agent being requested to re-route..
												this //Return this event to us (the agent).
												);
		}
		else if (eventId == event::EVT_CORE_COMMSIM_REROUTING_REQUEST)
		{
			//Were we requested to re-route?
			const ReRouteEventArgs &rrArgs = MSG_CAST(ReRouteEventArgs, args);
			const std::map<int, sim_mob::RoadRunnerRegion>& regions = ConfigManager::GetInstance().FullConfig().getNetwork().roadRunnerRegions;
			std::map<int, sim_mob::RoadRunnerRegion>::const_iterator it = regions.find(boost::lexical_cast<int>(rrArgs.getBlacklistRegion()));
			if (it != regions.end())
			{
				std::vector<const sim_mob::RoadSegment*> blacklisted = StreetDirectory::instance().getSegmentsFromRegion(it->second);
				rerouteWithBlacklist(blacklisted);
			}
		}
	}

	if (currRole)
	{
		currRole->onParentEvent(eventId, ctxId, sender, args);
	}
}

void Person_ST::rerouteWithBlacklist(const std::vector<const RoadSegment *> &blacklisted)
{
	//This requires the Role's intervention.
	if (currRole)
	{
		currRole->rerouteWithBlacklist(blacklisted);
	}
}


void Person_ST::HandleMessage(messaging::Message::MessageType type, const messaging::Message &message)
{
	if (currRole)
	{
		currRole->HandleParentMessage(type, message);
	}
}

void Person_ST::handleAMODEvent(event::EventId id, event::Context ctxId, event::EventPublisher *sender, const AMOD::AMODEventArgs &args)
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