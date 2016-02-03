//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_ST.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/ST_Config.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "event/args/ReRouteEventArgs.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "message/MessageBus.hpp"
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;

Person_ST::Person_ST(const std::string &src, const MutexStrategy &mtxStrat, int id, std::string databaseID)
: Person(src, mtxStrat, id, databaseID), startLaneIndex(-1), boardingTimeSecs(0), alightingTimeSecs(0), 
prevRole(NULL), currRole(NULL), nextRole(NULL), commEventRegistered(false), amodId("-1"),
amodPickUpSegmentStr("-1"), startSegmentId(-1), segmentStartOffset(0), initialSpeed(0),
amodSegmLength(0.0), amodSegmLength2(0.0), client_id(0), isPositionValid(false), isVehicleInLoadingQueue(false)
{
}

Person_ST::Person_ST(const std::string &src, const MutexStrategy &mtxStrat, const std::vector<TripChainItem *> &tc)
: Person(src, mtxStrat, tc), startLaneIndex(-1), boardingTimeSecs(0), alightingTimeSecs(0), 
prevRole(NULL), currRole(NULL), nextRole(NULL), commEventRegistered(false), amodId("-1"),
amodPickUpSegmentStr("-1"), startSegmentId(-1), segmentStartOffset(0), initialSpeed(0), amodSegmLength(0.0), amodSegmLength2(0.0)
{
	if (!tripChain.empty())
	{
		initTripChain();
	}
}

Person_ST::~Person_ST()
{
	//Un-register event listeners.
	if (commEventRegistered)
	{
		messaging::MessageBus::UnSubscribeEvent(event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT, this, this);
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
		if (this->getAge() >= iter->second.lowerAge && this->getAge() < iter->second.upperAge)
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
	Entity::setStartTime(value);
	if (currRole)
	{
		currRole->setArrivalTime(value + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
}

void Person_ST::load(const map<string, string> &configProps)
{
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
	
	if ((*currTripChainItem)->itemType == TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<Trip*> (*currTripChainItem))->getSubTripsRW()).begin();
		
		// if the first trip chain item is passenger, create waitBusActivityRole
		if (currSubTrip->travelMode == "BusTravel")
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

Entity::UpdateStatus Person_ST::checkTripChain(unsigned int currentTime)
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
	if ((*currTripChainItem)->itemType == TripChainItem::IT_TRIP)
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

	const TripChainItem* tci = *(this->nextTripChainItem);
	
	if (tci->itemType == TripChainItem::IT_TRIP)
	{
		nextRole = rf->createRole(tci, &(*nextSubTrip), this);
	}

	return true;
}

bool Person_ST::updatePersonRole()
{
	if (!nextRole)
	{
		const RoleFactory<Person_ST> *rf = RoleFactory<Person_ST>::getInstance();
		const TripChainItem *tci = *(this->currTripChainItem);
		const SubTrip* subTrip = NULL;

		if (tci->itemType == TripChainItem::IT_TRIP)
		{
			subTrip = &(*currSubTrip);
			(*currSubTrip).startTime = DailyTime(currTick.ms());
		}
		nextRole = rf->createRole(tci, subTrip, this);
	}

	changeRole();
	return true;
}

void Person_ST::changeRole()
{
	safe_delete_item(prevRole);
	prevRole = currRole;
	currRole = nextRole;
	nextRole = nullptr;
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

Entity::UpdateStatus Person_ST::frame_init(timeslice now)
{
	Entity::UpdateStatus result(Entity::UpdateStatus::RS_CONTINUE);
	
	//Call its "load" function
	load(configProperties);
	clearConfigProperties();

	messaging::MessageBus::RegisterHandler(this);
	currTick = now;

	//Reset the Region tracking data structures, if applicable.
	//regionAndPathTracker.reset();

	//Register for communication simulator messages, if applicable.
	if (!commEventRegistered && ST_Config::getInstance().commsim.enabled)
	{
		commEventRegistered = true;
		messaging::MessageBus::SubscribeEvent(event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT, this, this);
	}

	//Agents may be created with a null Role and a valid trip chain
	if (!currRole)
	{
		setStartTime(now.ms());

		result = checkTripChain();

		if (currRole)
		{
			currRole->setArrivalTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS());
		}

		//Nothing left to do?
		if (result.status == Entity::UpdateStatus::RS_DONE)
		{
			return result;
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
	
	return result;
}

Entity::UpdateStatus Person_ST::frame_tick(timeslice now)
{
    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();

	currTick = now;
	//TODO: Here is where it gets risky.
	if (isResetParamsRequired())
	{
		currRole->make_frame_tick_params(now);
		setResetParamsRequired(false);
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
        setStartTime(now.ms() + config.baseGranMS());

		retVal = checkTripChain();

		if (currTripChainItem != tripChain.end())
		{
			TripChainItem* tcItem = *currTripChainItem;
			if (tcItem) // if currTripChain not end and has value, call frame_init and switching roles
			{
				if (tcItem->itemType == TripChainItem::IT_ACTIVITY)
				{
					//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
					//since start time of the activity is usually later than what is configured initially,
					//we have to make adjustments so that the person waits for exact amount of time
					ActivityPerformer<Person_ST> *ap = dynamic_cast<ActivityPerformer<Person_ST>* > (currRole);
                    ap->setActivityStartTime(DailyTime(now.ms() + config.baseGranMS()));
                    ap->setActivityEndTime(DailyTime(now.ms() + config.baseGranMS() + (tcItem->endTime.getValue() - tcItem->startTime.getValue())));
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
		LogOut(currRole->Movement()->frame_tick_output());
	}

	setResetParamsRequired(true);
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
	if ((*currTripChainItem)->itemType == TripChainItem::IT_TRIP)
	{
		//don't advance to next tripchainItem immediately, check the subtrip first
		bool res = advanceCurrentSubTrip();

		//subtrip advanced successfully, no need to advance currTripChainItem
		if (res)
		{
			return res;
		}
	}

	//do the increment
	++currTripChainItem;

	if (currTripChainItem == tripChain.end())
	{
		//but tripchain items are also over, get out !
		return false;
	}

	//so far, advancing the tripchainitem has been successful
	//Also set the currSubTrip to the beginning of trip , just in case
	if ((*currTripChainItem)->itemType == TripChainItem::IT_TRIP)
	{
		currSubTrip = resetCurrSubTrip();
	}

	return true;
}

void Person_ST::onEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
//	//Some events only matter if they are for us.
//	if (ctxId == this)
//	{
//		if (eventId == event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT)
//		{
//			//Was communication simulator enabled for us? If so, start tracking Regions.
//			Print() << "Enabling Region support for agent: " << this << "\n";
//			enableRegionSupport();
//
//			//This requires us to now listen for a new set of events.
//			messaging::MessageBus::SubscribeEvent(event::EVT_CORE_COMMSIM_REROUTING_REQUEST,
//												this, //Only when we are the Agent being requested to re-route..
//												this //Return this event to us (the agent).
//												);
//		}
//		else if (eventId == event::EVT_CORE_COMMSIM_REROUTING_REQUEST)
//		{
//			//Were we requested to re-route?
//			const event::ReRouteEventArgs &rrArgs = MSG_CAST(event::ReRouteEventArgs, args);
//			const std::map<int, RoadRunnerRegion>& regions = ConfigManager::GetInstance().FullConfig().getNetwork().roadRunnerRegions;
//			std::map<int, RoadRunnerRegion>::const_iterator it = regions.find(boost::lexical_cast<int>(rrArgs.getBlacklistRegion()));
//			if (it != regions.end())
//			{
//				std::vector<const RoadSegment*> blacklisted = StreetDirectory::instance().getSegmentsFromRegion(it->second);
//				rerouteWithBlacklist(blacklisted);
//			}
//		}
//	}

	if (currRole)
	{
		currRole->onParentEvent(eventId, ctxId, sender, args);
	}
}

void Person_ST::rerouteWithBlacklist(const std::vector<const Link *> &blacklisted)
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

void Person_ST::handleAMODEvent(event::EventId id, event::Context ctxId, event::EventPublisher *sender, const amod::AMODEventArgs &args)
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
	sim_mob::amod::AMODController::getInstance()->handleVHArrive(this);

}

void Person_ST::handleAMODPickup()
{
	//ask the AMODController to handle the arrival
	sim_mob::amod::AMODController::getInstance()->handleVHPickup(this);
}
