//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person.hpp"

#include <algorithm>
#include <sstream>

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

//For debugging
#include "roles/Role.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"
#include "geospatial/Node.hpp"
#include "entities/misc/TripChain.hpp"
#include "event/args/ReRouteEventArgs.hpp"
#include "workers/Worker.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "message/MessageBus.hpp"
#include "entities/amodController/AMODController.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
#include "path/PT_RouteChoiceLuaProvider.hpp"
#include "entities/params/PT_NetworkEntities.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/misc/TripChain.hpp"
#endif
//
using std::map;
using std::string;
using std::vector;
using namespace sim_mob;
using namespace sim_mob::event;
typedef Entity::UpdateStatus UpdateStatus;


namespace
{
// default lowest age
const int DEFAULT_LOWEST_AGE = 20;
// default highest age
const int DEFAULT_HIGHEST_AGE = 60;
} //End unnamed namespace

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID)
: Agent(mtxStrat, id), databaseID(databaseID), prevRole(nullptr), currRole(nullptr), nextRole(nullptr), agentSrc(src),
tripChain(nullptr), age(0), resetParamsRequired(false), isFirstTick(true), nextPathPlanned(false), commEventRegistered(false), originNode(),
destNode(), currLinkTravelStats(nullptr, 0.0), currRdSegTravelStats(nullptr)
{
}

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Agent(mtxStrat), databaseID(tc.front()->getPersonID()), prevRole(nullptr), currRole(nullptr), nextRole(nullptr), 
agentSrc(src), tripChain(tc), age(0), resetParamsRequired(false), isFirstTick(true), nextPathPlanned(false), commEventRegistered(false), 
originNode(), destNode(), currLinkTravelStats(nullptr, 0.0), currRdSegTravelStats(nullptr)
{
	if (!tripChain.empty())
	{
		initTripChain();
	}
}

sim_mob::Person::~Person()
{
	safe_delete_item(prevRole);
	safe_delete_item(currRole);

	//Un-register event listeners.
	if (commEventRegistered)
	{
		messaging::MessageBus::UnSubscribeEvent(sim_mob::event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT, this, this);
	}

	//safe_delete_item(nextRole);
	//last chance to collect travel time metrics(if any)
	//aggregateSubTripMetrics();
	//serialize them
	//serializeTripTravelTimeMetrics();
}

void sim_mob::Person::setTripChain(const vector<TripChainItem *>& tripChain)
{
	//delete the previous trip chain
	clear_delete_vector(this->tripChain);

	this->tripChain = tripChain;

	//Initialise the trip chain - as the references to current trip chain and the
	//current sub trip need to be updated
	initTripChain();
}

void sim_mob::Person::load(const map<string, string>& configProps)
{
}

void Person::rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted)
{
	//This requires the Role's intervention.
	if (currRole)
	{
		currRole->rerouteWithBlacklist(blacklisted);
	}
}

bool sim_mob::Person::frame_init(timeslice now)
{
	messaging::MessageBus::RegisterHandler(this);
	currTick = now;

	//Reset the Region tracking data structures, if applicable.
	//regionAndPathTracker.reset();

	//Register for commsim messages, if applicable.
	if (!commEventRegistered && ConfigManager::GetInstance().XmlConfig().system.simulation.commsim.enabled)
	{
		commEventRegistered = true;
		messaging::MessageBus::SubscribeEvent(
											sim_mob::event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT,
											this, //Only when we are the Agent having commsim enabled.
											this //Return this event to us (the agent).
											);
	}

	//Agents may be created with a null Role and a valid trip chain
	if (!currRole)
	{
		//TODO: This UpdateStatus has a "prevParams" and "currParams" that should
		//      (one would expect) be dealt with. Where does this happen?
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

	//Failsafe: no Role at all?
	if (!currRole)
	{
		std::ostringstream txt;
		txt << "Person " << this->getId() << " has no Role.";
		throw std::runtime_error(txt.str());
	}

	//Get an UpdateParams instance.
	//TODO: This is quite unsafe, but it's a relic of how Person::update() used to work.
	//      We should replace this eventually (but this will require a larger code cleanup).
	currRole->make_frame_tick_params(now);

	//Now that the Role has been fully constructed, initialise it.
	if ((*currTripChainItem))
	{
		currRole->Movement()->frame_init();
	}
	return true;
}

void sim_mob::Person::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
	//Some events only matter if they are for us.
	if (ctxId == this)
	{
		if (eventId == event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT)
		{
			//Was commsim enabled for us? If so, start tracking Regions.
			Print() << "Enabling Region support for agent: " << this << "\n";
			enableRegionSupport();

			//This requires us to now listen for a new set of events.
			messaging::MessageBus::SubscribeEvent(
												sim_mob::event::EVT_CORE_COMMSIM_REROUTING_REQUEST,
												this, //Only when we are the Agent being requested to re-route..
												this //Return this event to us (the agent).
												);
		}
		else if (eventId == event::EVT_CORE_COMMSIM_REROUTING_REQUEST)
		{
			//Were we requested to re-route?
			const ReRouteEventArgs& rrArgs = MSG_CAST(ReRouteEventArgs, args);
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

void sim_mob::Person::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	if (currRole)
	{
		currRole->HandleParentMessage(type, message);
	}
}

Entity::UpdateStatus sim_mob::Person::frame_tick(timeslice now)
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

void sim_mob::Person::frame_output(timeslice now)
{
	//Save the output
	if (!isToBeRemoved())
	{
		currRole->Movement()->frame_tick_output();
	}

	//avoiding logical errors while improving the code
	resetParamsRequired = true;
}

bool sim_mob::Person::updateOD(sim_mob::TripChainItem * tc, const sim_mob::SubTrip *subtrip)
{
	return tc->setPersonOD(this, subtrip);
}

bool sim_mob::Person::findPersonNextRole()
{
	if (!updateNextTripChainItem())
	{
		safe_delete_item(nextRole);
		return false;
	}

	//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
	// it's better to avoid possible errors (e.g., if the equality operator is defined)
	// by saving it until the next time tick.
	//safe_delete_item(prevRole);
	safe_delete_item(nextRole);
	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();

	const sim_mob::TripChainItem* tci = *(this->nextTripChainItem);
	if (tci->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		nextRole = rf.createRole(tci, &(*nextSubTrip), this);
	}

	return true;
}

bool sim_mob::Person::updatePersonRole(sim_mob::Role* newRole)
{
	//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
	// it's better to avoid possible errors (e.g., if the equality operator is defined)
	// by saving it until the next time tick.
	safe_delete_item(prevRole);
	const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
	const sim_mob::TripChainItem* tci = *(this->currTripChainItem);
	const sim_mob::SubTrip* subTrip = nullptr;

	if (tci->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		subTrip = &(*currSubTrip);
	}

	if (!newRole)
	{
		newRole = rf.createRole(tci, subTrip, this);
	}

	changeRole(newRole);
	return true;
}

void sim_mob::Person::setStartTime(unsigned int value)
{
	sim_mob::Entity::setStartTime(value);
	if (currRole)
	{
		currRole->setArrivalTime(value + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
}

UpdateStatus sim_mob::Person::checkTripChain()
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
	updatePersonRole(nextRole);

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

std::vector<sim_mob::SubTrip>::iterator sim_mob::Person::resetCurrSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);

	if (!trip)
	{
		throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
	}

	return trip->getSubTripsRW().begin();
}

bool sim_mob::Person::makeODsToTrips(SubTrip* curSubTrip, std::vector<sim_mob::SubTrip>& newSubTrips, const std::vector<sim_mob::OD_Trip>& matchedTrips)
{
	bool ret = true;
	bool invalidFlag = false;
	if (matchedTrips.size() > 0)
	{
		std::vector<sim_mob::OD_Trip>::const_iterator it = matchedTrips.begin();
		while (it != matchedTrips.end())
		{
			sim_mob::SubTrip subTrip;
			WayPoint source;
			WayPoint dest;
			std::string sSrc = (*it).startStop;
			std::string sEnd = (*it).endStop;
			std::string srcType;
			std::string endType;
			int sType = (*it).sType;
			int eType = (*it).eType;
			switch (eType)
			{
			case 0:
			{
				endType = "NODE";
				int id = boost::lexical_cast<unsigned int>(sEnd);
				sim_mob::Node* node = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(id);
				if (node)
				{
					dest = WayPoint(node);
				}
				break;
			}
			case 1:
			{
				endType = "BUS_STOP";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sEnd);
				if (stop)
				{
					dest = WayPoint(stop);
				}
				break;
			}
			case 2:
			{
				endType = "MRT_STOP";
				sim_mob::MRT_Stop* stop = sim_mob::PT_Network::getInstance().findMRT_Stop(sEnd);
				if (stop)
				{
					dest = WayPoint(stop);
				}
				break;
			}
			}

			switch (sType)
			{
			case 0:
			{
				srcType = "NODE";
				int id = boost::lexical_cast<unsigned int>(sSrc);
				sim_mob::Node* node = ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(id);
				if (node)
				{
					source = WayPoint(node);
				}
				break;
			}
			case 1:
			{
				srcType = "BUS_STOP";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sSrc);
				if (stop)
				{
					source = WayPoint(stop);
				}
				break;
			}
			case 2:
			{
				srcType = "MRT_STOP";
				sim_mob::MRT_Stop* stop = sim_mob::PT_Network::getInstance().findMRT_Stop(sSrc);
				if (stop)
				{
					source = WayPoint(stop);
				}
				break;
			}
			}
			if (it->tType == "Bus" && (sType != 1 || eType != 1))
			{
				invalidFlag = true;
			}
			if (it->tType == "RTS" && (sType != 2 || eType != 2))
			{
				invalidFlag = true;
			}

			if (source.type_ != WayPoint::INVALID && dest.type_ != WayPoint::INVALID && !invalidFlag)
			{
				subTrip.setPersonID(-1);
				subTrip.itemType = TripChainItem::getItemType("Trip");
				subTrip.sequenceNumber = 1;
				subTrip.startTime = curSubTrip->endTime;
				subTrip.endTime = DailyTime((*it).travelTime * 1000.0);
				subTrip.origin = source;
				subTrip.startLocationId = sSrc;
				subTrip.startLocationType = srcType;
				subTrip.endLocationId = sEnd;
				subTrip.endLocationType = endType;
				if (source.type_ == WayPoint::BUS_STOP)
				{
					subTrip.originType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
				}
				else
				{
					subTrip.originType = TripChainItem::LT_NODE;
				}
				subTrip.destination = dest;
				if (dest.type_ == WayPoint::BUS_STOP)
				{
					subTrip.destinationType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
				}
				else
				{
					subTrip.destinationType = TripChainItem::LT_NODE;
				}
				subTrip.tripID = "";
				subTrip.mode = (*it).tType;
				if ((*it).tType.find("Walk") != std::string::npos)
				{
					subTrip.mode = "Walk";
					subTrip.isPT_Walk = true;
					subTrip.walkTime = (*it).walkTime;
				}
				else if ((*it).tType.find("Bus") != std::string::npos)
				{
					subTrip.mode = "BusTravel";
				}
				else
				{
					subTrip.mode = "MRT";
				}
				subTrip.isPrimaryMode = true;
				subTrip.ptLineId = it->serviceLines;
				newSubTrips.push_back(subTrip);
			}
			else
			{
				Print() << "[PT pathset] make trip failed:[" << sSrc << "]|[" << sEnd << "]" << std::endl;
				ret = false;
				break;
			}
			++it;
		}
	}
	return ret;
}



bool sim_mob::Person::updateNextSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);

	if (!trip)
	{
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end())
	{
		return false;
	}

	nextSubTrip = currSubTrip + 1;

	if (nextSubTrip == trip->getSubTrips().end())
	{
		return false;
	}

	return true;
}

bool sim_mob::Person::updateNextTripChainItem()
{
	bool res = false;

	if (currTripChainItem == tripChain.end())
	{
		return false;
	}

	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//check for next sub-trip first
		res = updateNextSubTrip();
	}

	if (res)
	{
		nextTripChainItem = currTripChainItem;
		return res;
	}

	//no, it is not the sub-trip we need to advance, it is the trip chain item
	nextTripChainItem = currTripChainItem + 1;
	if (nextTripChainItem == tripChain.end())
	{
		return false;
	}

	//Also set the nextSubTrip to the beginning of trip , just in case
	if ((*nextTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*nextTripChainItem); //easy reading
		if (!trip)
		{
			throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
		}
		nextSubTrip = trip->getSubTrips().begin();
	}

	return true;
}

bool sim_mob::Person::advanceCurrentSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);
	
	if (!trip)
	{
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end()) /*just a routine check*/
	{
		return false;
	}

	//get metric
	/*TravelMetric currRoleMetrics;
	if(currRole != nullptr)
	{
		currRoleMetrics = currRole->Movement()->finalizeTravelTimeMetric();
		//serializeSubTripChainItemTravelTimeMetrics(*currRoleMetrics,currTripChainItem,currSubTrip);
	}*/

	++currSubTrip;

	if (currSubTrip == trip->getSubTrips().end())
	{
		return false;
	}
	return true;
}

bool sim_mob::Person::advanceCurrentTripChainItem()
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

vector<BufferedBase *> sim_mob::Person::buildSubscriptionList()
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

void sim_mob::Person::changeRole(sim_mob::Role* newRole)
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
	currRole = newRole;

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

void sim_mob::Person::setPersonCharacteristics()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::map<int, PersonCharacteristics>& personCharacteristics = config.personCharacteristicsParams.personCharacteristics;

	int lowestAge = DEFAULT_LOWEST_AGE;
	int highestAge = DEFAULT_HIGHEST_AGE;

	// if no personCharacteristics item in the configuration file, introduce default lowestAge and highestAge
	if (!personCharacteristics.empty())
	{
		lowestAge = config.personCharacteristicsParams.lowestAge;
		highestAge = config.personCharacteristicsParams.highestAge;
	}

	boost::mt19937 gen(static_cast<unsigned int> (getId() * getId()));
	boost::uniform_int<> ageRange(lowestAge, highestAge);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAge(gen, ageRange);
	age = (unsigned int) varAge();	
}


/********************************************************************
 * ************* Collection and presentation of metrics *************
 * ******************************************************************
 */

namespace
{
sim_mob::OneTimeFlag titleSubPredayTT;
}

void sim_mob::Person::serializeSubTripChainItemTravelTimeMetrics(const TravelMetric& subtripMetrics, std::vector<TripChainItem*>::iterator currTripChainItem, 
																 std::vector<SubTrip>::iterator currSubTrip) const
{
	sim_mob::BasicLogger& csv = sim_mob::Logger::log(ConfigManager::GetInstance().FullConfig().pathSet().subTripOP);
	if (!(subtripMetrics.finalized && subtripMetrics.started))
	{
		return;
	} //sanity check
	if (titleSubPredayTT.check())
	{
		csv << "person_id,trip_id,subtrip_id,origin,destination,mode,start_time,end_time,travel_time,total_distance,ptt_wt,pt_walk,cbd_entry_node,cbd_exit_node,cbd_entry_time,cbd_exit_time,cbd_travel_time,non_cbd_travel_time,cbd_distance,non_cbd_distance\n";
	}

	sim_mob::SubTrip &st = (*currSubTrip); //easy reading
	// restricted area which is to be appended at the end of the csv line
	std::stringstream restrictedRegion("");
	if (st.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER || st.cbdTraverseType == sim_mob::TravelMetric::CBD_EXIT)
	{
		//sanity check
		if (!(subtripMetrics.cbdOrigin.node_ && subtripMetrics.cbdDestination.node_))
		{
			restrictedRegion <<
					subtripMetrics.origin.node_->getID() << "," <<
					subtripMetrics.destination.node_->getID() <<
					(st.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER ? " ,Enter" : " ,Exit") << " has null values " <<
					(subtripMetrics.cbdOrigin.node_ != nullptr ? subtripMetrics.cbdOrigin.node_->getID() : 0) << "," <<
					(subtripMetrics.cbdDestination.node_ != nullptr ? subtripMetrics.cbdDestination.node_->getID() : 0) << "\n";
		}
		else //valid scenario:
		{
			restrictedRegion <<
					subtripMetrics.cbdOrigin.node_->getID() << "," << //	cbd_entry_node
					subtripMetrics.cbdDestination.node_->getID() << "," << //	cbd_exit_node
					subtripMetrics.cbdStartTime.getStrRepr() << "," << //	cbd_entry_time
					subtripMetrics.cbdEndTime.getStrRepr() << "," << //	cbd_exit_time
					subtripMetrics.cbdTravelTime << "," << //	cbd_travel_time
					(subtripMetrics.travelTime - subtripMetrics.cbdTravelTime) << "," << //	non_cbd_travel_time
					subtripMetrics.cbdDistance << "," << //	cbd_distance
					(subtripMetrics.distance - subtripMetrics.cbdDistance); //	non_cbd_distance
		}
	}
	else// if Agent never entered or exited CBD
	{
		restrictedRegion <<
				"0" << "," << //	cbd_entry_node
				"0" << "," << //	cbd_exit_node
				"00:00:00" << "," << //	cbd_entry_time
				"00:00:00" << "," << //	cbd_exit_time
				"0" << "," << //	cbd_travel_time
				subtripMetrics.travelTime << "," << //	non_cbd_travel_time
				"0" << "," << //	cbd_distance
				subtripMetrics.distance; //	non_cbd_distance
	}

	int origiNode = 0, destNode = 0, cbdStartNode = 0, cbdEndNode = 0;
	if (subtripMetrics.origin.type_ == WayPoint::NODE)
	{
		origiNode = subtripMetrics.origin.node_->getID();
	}
	if (subtripMetrics.destination.type_ == WayPoint::NODE)
	{
		destNode = subtripMetrics.destination.node_->getID();
	}
	std::stringstream res("");
	// actual writing
	res <<
			this->getId() << "," << //	person_id
			(static_cast<Trip*> (*currTripChainItem))->tripID << "," << //	trip_id
			st.tripID << "," << //	subtrip_id
			origiNode << "," << //	origin
			destNode << "," << //	destination
			st.mode << "," << //	mode
			subtripMetrics.startTime.getStrRepr() << "," << //	start_time
			subtripMetrics.endTime.getStrRepr() << "," << //	end_time
			//			 TravelMetric::getTimeDiffHours(subtripMetrics.endTime, subtripMetrics.startTime)  << ","		//	travel_time### commented
			subtripMetrics.travelTime << "," << //	travel_time
			subtripMetrics.distance << "," << //	total_distance
			"0" << "," << //placeholder for public transit's waiting time								//	ptt_wt
			"0" << "," << //placeholder for public transit's walk time									//	pt_walk
			restrictedRegion.str() << "\n"; /* MIXED CBD Information */

	csv << res.str();

	if (subtripMetrics.cbdOrigin.type_ == WayPoint::NODE)
	{
		cbdStartNode = subtripMetrics.cbdOrigin.node_->getID();
	}
	if (subtripMetrics.cbdDestination.type_ == WayPoint::NODE)
	{
		cbdEndNode = subtripMetrics.cbdDestination.node_->getID();
	}
	if (subtripMetrics.cbdTraverseType == TravelMetric::CBD_NONE || cbdStartNode == 0 || cbdEndNode == 0)
	{
		return;
	}
	std::stringstream ret("");
	ret << this->getId() << "," << origiNode << "," << destNode << ","
			<< cbdStartNode << "," << cbdEndNode << ","
			<< subtripMetrics.cbdStartTime.getStrRepr() << ","
			<< subtripMetrics.cbdEndTime.getStrRepr() << ","
			<< st.mode << ","
			<< subtripMetrics.cbdTraverseType << std::endl;
	sim_mob::BasicLogger& cbd = sim_mob::Logger::log("cdb.csv");
	cbd << ret.str();
}

/********************************************************************
 * ************* Collection and presentation of metrics *************
 * ****************         UNUSED     ******************************
 * ******************************************************************
 */

/*
 * each entry line is divided into multiple groups of columns,
 * first create most of those groups and then interate them.
 */
void sim_mob::Person::serializeCBD_SubTrip(const TravelMetric &metric)
{
	//sanity checks
	if (metric.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER
			|| metric.cbdTraverseType == sim_mob::TravelMetric::CBD_EXIT)
	{
		return;
	}

	sim_mob::Trip * trip = dynamic_cast<Trip*> (*currTripChainItem);
	if (!trip)
	{
		throw std::runtime_error("Invalid Tripchain Item supplied, expected a Trip");
	}

	std::stringstream tripStrm_1("");
	//step-1 trip ,part 1
	tripStrm_1 <<
			this->getId() << "," <<
			trip->sequenceNumber << "," <<
			"Trip" << "," <<
			this->getId() << trip->sequenceNumber << "," << //tripid
			trip->origin.node_->getID() << "," <<
			(trip->originType == TripChainItem::LT_NODE ? "node" : "stop") << "," <<
			trip->destination.node_->getID() << "," <<
			(trip->destinationType == TripChainItem::LT_NODE ? "node" : "stop") << ",";

	//step-2 subtrip information part 1
	const sim_mob::SubTrip &st = *currSubTrip;
	std::stringstream subTripStrm_1("");
	subTripStrm_1 <<
			st.tripID << "," <<
			metric.cbdOrigin.node_->getID() << "," << //replace original subtrip
			"Node" << "," <<
			metric.cbdDestination.node_->getID() << "," <<
			"Node" << "," <<
			st.mode << "," <<
			(st.isPrimaryMode ? "TRUE" : "FALSE") << "," <<
			st.startTime.getStrRepr() << "," <<
			st.ptLineId;

	//step-3 activity, part 1
	std::stringstream activity_1("");
	activity_1 << ","
			<< "0" << "," //activity_id
			<< "dummy" << "," //activity_type
			<< "FALSE" << "," //is_primary_activity
			<< "FALSE" << "," //flexible_activity
			<< "TRUE" << "," //mandatory_activity
			<< "0" << "," //mandatory_activity
			<< "0" << "," //activity_location
			<< "node" << "," //activity_loc_type
			<< "," //activity_start_time
			<< "" //activity_end_time
			;


	//step-4 INTEGRATION with sub trip:
	//order:
	//trip information part 1 :  tripStrm_1
	//subtrip 'custom info' : subTripStrm_1
	//activity part 1 : activity_1
	std::stringstream line("");
	line <<
			tripStrm_1.str() << "," <<
			subTripStrm_1.str() << "," <<
			activity_1.str();
	sim_mob::Logger::log("tripchain_info_for_short_term.csv") << line.str();
}

void sim_mob::Person::serializeCBD_Activity(const TravelMetric &metric)
{
	//sanity checks

	sim_mob::Activity * activity = dynamic_cast<Activity*> (*currTripChainItem);
	if (!activity)
	{
		throw std::runtime_error("Invalid Tripchain Item supplied, expected an Activity");
	}

	std::stringstream tripStrm_1("");
	//step-1 trip ,part 1
	tripStrm_1 << this->getId() << "," <<
			activity->sequenceNumber << "," <<
			"Activity" << "," <<
			"0" << "," /*<< //tripid
		 activity->fromLocation.node_->getID() << "," <<
		(activity->fromLocationType == TripChainItem::LT_NODE ? "Node" : "Stop") << "," <<
		activity->toLocation.node_->getID()<< ","   <<
		(activity->toLocationType == TripChainItem::LT_NODE ? "Node" : "Stop") << "," */;
}

void sim_mob::Person::aggregateSubTripMetrics()
{
	if (subTripTravelMetrics.begin() == subTripTravelMetrics.end())
	{
		return;
	}
	TravelMetric newTripMetric;
	std::vector<TravelMetric>::iterator item(subTripTravelMetrics.begin());
	newTripMetric.startTime = item->startTime; //first item
	newTripMetric.origin = item->origin;
	for (; item != subTripTravelMetrics.end(); ++item)
	{
		newTripMetric.travelTime += item->travelTime;
	}
	newTripMetric.endTime = subTripTravelMetrics.rbegin()->endTime;
	newTripMetric.destination = subTripTravelMetrics.rbegin()->destination;
	subTripTravelMetrics.clear();
	tripTravelMetrics.push_back(newTripMetric);
}

void sim_mob::Person::addSubtripTravelMetrics(TravelMetric &value)
{
	subTripTravelMetrics.push_back(value);
}

/**
 * Serializer for Trip level travel time
 */
void sim_mob::Person::serializeTripTravelTimeMetrics()
{
	if (tripTravelMetrics.empty())
	{
		return;
	}
	sim_mob::BasicLogger & csv = sim_mob::Logger::log("trip_level_travel_time.csv");

	BOOST_FOREACH(TravelMetric &item, tripTravelMetrics)
	{
		csv << this->getId() << "," 
			<< item.origin.node_->getID() << ","
			<< item.destination.node_->getID() << ","
			<< item.startTime.getStrRepr() << ","
			<< item.endTime.getStrRepr() << ","
			<< (item.endTime - item.startTime).getStrRepr()
			<< "\n";
	}
	tripTravelMetrics.clear();
}

void sim_mob::Person::addToLinkTravelStatsMap(LinkTravelStats ts, double exitTime)
{
	std::map<double, LinkTravelStats>& travelMap = linkTravelStatsMap;
	travelMap.insert(std::make_pair(exitTime, ts));
}

