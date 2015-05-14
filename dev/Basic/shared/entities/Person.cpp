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
#include "workers/Worker.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "message/MessageBus.hpp"
#include "entities/amodController/AMODController.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
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
typedef Entity::UpdateStatus UpdateStatus;


namespace {
	// default lowest age
	const int DEFAULT_LOWEST_AGE = 20;
	// default highest age
	const int DEFAULT_HIGHEST_AGE = 60;

Trip* MakePseudoTrip(const Person& ag, const std::string& mode)
{
	//Make sure we have something to work with
	if (!(ag.originNode.node_&& ag.destNode.node_)) {
		std::stringstream msg;
		msg <<"Can't make a pseudo-trip for an Agent with no origin and destination nodes: " <<ag.originNode.node_ <<" , " <<ag.destNode.node_;
		throw std::runtime_error(msg.str().c_str());
	}

	//Make the trip itself
	Trip* res = new Trip();
	res->setPersonID(ag.getId());
	res->itemType = TripChainItem::getItemType("Trip");
	res->sequenceNumber = 1;
	res->startTime = DailyTime(ag.getStartTime());  //TODO: This may not be 100% correct
	res->endTime = res->startTime; //No estimated end time.
	res->tripID = "";
	res->fromLocation = WayPoint(ag.originNode);
	res->fromLocationType = TripChainItem::getLocationType("node");
	res->toLocation = WayPoint(ag.destNode);
	res->toLocationType = res->fromLocationType;
	res->travelMode = mode;

	//Make and assign a single sub-trip
	sim_mob::SubTrip subTrip;
	subTrip.setPersonID(-1);
	subTrip.itemType = TripChainItem::getItemType("Trip");
	subTrip.sequenceNumber = 1;
	subTrip.startTime = res->startTime;
	subTrip.endTime = res->startTime;
	subTrip.fromLocation = res->fromLocation;
	subTrip.fromLocationType = res->fromLocationType;
	subTrip.toLocation = res->toLocation;
	subTrip.toLocationType = res->toLocationType;
	subTrip.tripID = "";
	subTrip.mode = mode;
	subTrip.isPrimaryMode = true;
	subTrip.ptLineId = "";

	//Add it to the Trip; return this value.
	res->addSubTrip(subTrip);
	return res;
}

}  //End unnamed namespace

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID) : Agent(mtxStrat, id),
	prevRole(nullptr), currRole(nullptr), nextRole(nullptr), agentSrc(src), currTripChainSequenceNumber(0), remainingTimeThisTick(0.0),
	requestedNextSegStats(nullptr), canMoveToNextSegment(NONE), databaseID(databaseID), debugMsgs(std::stringstream::out), tripchainInitialized(false), laneID(-1),
	age(0), boardingTimeSecs(0), alightingTimeSecs(0), client_id(-1), resetParamsRequired(false), nextLinkRequired(nullptr), currSegStats(nullptr),amodId("-1"),amodPickUpSegmentStr("-1"),amodSegmLength(0.0),
	initSegId(0), initDis(0), initSpeed(0), amodSegmLength2(0), currStatus(IN_CAR_PARK), firstTick(true), currLane(NULL)
{
}

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
	: Agent(mtxStrat), remainingTimeThisTick(0.0), requestedNextSegStats(nullptr), canMoveToNextSegment(NONE),
	  databaseID(tc.front()->getPersonID()), debugMsgs(std::stringstream::out), prevRole(nullptr), currRole(nullptr),
	  nextRole(nullptr), laneID(-1), agentSrc(src), tripChain(tc), tripchainInitialized(false), age(0), boardingTimeSecs(0), alightingTimeSecs(0),
	  client_id(-1),amodPath( std::vector<WayPoint>() ), nextLinkRequired(nullptr), currSegStats(nullptr),amodId("-1"),amodPickUpSegmentStr("-1"),
	  amodSegmLength(0.0)
{
	if(ConfigManager::GetInstance().FullConfig().RunningMidSupply()){
		convertODsToTrips();
		insertWaitingActivityToTrip();
	}

	if(!tripChain.empty()) { initTripChain(); }
}
void sim_mob::Person::initTripChain(){
	currTripChainItem = tripChain.begin();
	//TODO: Check if short term is okay with this approach of checking agent source
	const std::string& src = getAgentSrc();
	if(src == "XML_TripChain" || src == "DAS_TripChain" || src == "AMOD_TripChain" || src == "BusController")
	{
		setStartTime((*currTripChainItem)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
	}
	else
	{
		setStartTime((*currTripChainItem)->startTime.getValue());
	}
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP || (*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_FMODSIM)
	{
		vector<SubTrip>::iterator subTripEnd = ((dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW()).end();
		currSubTrip = ((dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW()).begin();
		// if the first tripchain item is passenger, create waitBusActivityRole
		if(currSubTrip->mode == "BusTravel") {
			const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
			currRole = rf.createRole("waitBusActivity", this);
			nextRole = rf.createRole("passenger", this);
		}
		//consider putting this in IT_TRIP clause
		if(!updateOD(*currTripChainItem)){ //Offer some protection
				throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
		}
	}

	setNextPathPlanned(false);
	firstTick = true;
	tripchainInitialized = true;
}

sim_mob::Person::~Person() {
	safe_delete_item(prevRole);
	safe_delete_item(currRole);
	//safe_delete_item(nextRole);
	//last chance to collect travel time metrics(if any)
	//aggregateSubTripMetrics();
	//serialize them
	//serializeTripTravelTimeMetrics();
}

void sim_mob::Person::setTripChain(const vector<TripChainItem *>& tripChain)
{
	//delete the previous tripchain
	clear_delete_vector(this->tripChain);

	this->tripChain = tripChain;

	//Initialize the trip chain - as the references to current trip chain and the
	//current sub trip need to be updated
	initTripChain();
}

void sim_mob::Person::load(const map<string, string>& configProps)
{
	if(!tripChain.empty()) {
		return;// already have a tripchain usually from generateFromTripChain, no need to load
	}
	//Make sure they have a mode specified for this trip
	map<string, string>::const_iterator it = configProps.find("#mode");
	if (it==configProps.end()) {
		throw std::runtime_error("Cannot load person: no mode");
	}
	std::string mode = it->second;

	//Consistency check: specify both origin and dest
	if (configProps.count("originPos") != configProps.count("destPos")) {
		throw std::runtime_error("Agent must specify both originPos and destPos, or neither.");
	}


	std::map<std::string, std::string>::const_iterator lanepointer = configProps.find("lane");
	if(lanepointer != configProps.end())
	{
		try {
		    int x = boost::lexical_cast<int>( lanepointer->second );
		    laneID = x;
		} catch( boost::bad_lexical_cast const& ) {
		    Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initSegId
	std::map<std::string, std::string>::const_iterator itt = configProps.find("initSegId");
	if(itt != configProps.end())
	{
		try {
			int x = boost::lexical_cast<int>( itt->second );
			initSegId = x;
		} catch( boost::bad_lexical_cast const& ) {
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initSegPer
	itt = configProps.find("initDis");
	if(itt != configProps.end())
	{
		try {
			int x = boost::lexical_cast<int>( itt->second );
			initDis = x;
		} catch( boost::bad_lexical_cast const& ) {
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}
	// initPosSegPer
	itt = configProps.find("initSpeed");
	if(itt != configProps.end())
	{
		try {
			int x = boost::lexical_cast<int>( itt->second );
			initSpeed = x;
		} catch( boost::bad_lexical_cast const& ) {
			Warn() << "Error: input string was not valid" << std::endl;
		}
	}

	// node
	map<string, string>::const_iterator oriNodeIt = configProps.find("originNode");
	map<string, string>::const_iterator destNodeIt = configProps.find("destNode");
	if(oriNodeIt!=configProps.end() && destNodeIt!=configProps.end()) {
		int originNodeId;
		int destNodeid;
		try {
			originNodeId = boost::lexical_cast<int>( oriNodeIt->second );
			destNodeid = boost::lexical_cast<int>( destNodeIt->second );
		} catch( boost::bad_lexical_cast const& ) {
			Warn() << "Error: input string was not valid" << std::endl;
		}

		//Otherwise, make a trip chain for this Person.
		this->originNode = WayPoint( ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(originNodeId) );
		this->destNode = WayPoint( ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(destNodeid) );

		Trip* singleTrip = MakePseudoTrip(*this, mode);

		std::vector<TripChainItem*> trip_chain;
		trip_chain.push_back(singleTrip);

		//TODO: Some of this should be performed in a centralized place; e.g., "Agent::setTripChain"
		//TODO: This needs to go in a centralized place.
		this->originNode = singleTrip->fromLocation;
		this->destNode = singleTrip->toLocation;
		this->setNextPathPlanned(false);
		this->setTripChain(trip_chain);
		this->initTripChain();
	}
	else {
		//Consistency check: are they requesting a pseudo-trip chain when they actually have one?
		map<string, string>::const_iterator origIt = configProps.find("originPos");
		map<string, string>::const_iterator destIt = configProps.find("destPos");
		if (origIt!=configProps.end() && destIt!=configProps.end()) {
			//Double-check some potential error states.
			if (!tripChain.empty()) {
				throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
			}
			if (this->originNode.node_ || this->destNode.node_ ) {
				throw std::runtime_error("Manual position specified for Agent with existing start and end of Trip Chain.");
			}

			//Otherwise, make a trip chain for this Person.
			this->originNode = WayPoint( ConfigManager::GetInstance().FullConfig().getNetwork().locateNode(parse_point(origIt->second), true));
			this->destNode = WayPoint( ConfigManager::GetInstance().FullConfig().getNetwork().locateNode(parse_point(destIt->second), true) );

			Trip* singleTrip = MakePseudoTrip(*this, mode);

			std::vector<TripChainItem*> trip_chain;
			trip_chain.push_back(singleTrip);

			//TODO: Some of this should be performed in a centralized place; e.g., "Agent::setTripChain"
			//TODO: This needs to go in a centralized place.
			this->originNode = singleTrip->fromLocation;
			this->destNode = singleTrip->toLocation;
			this->setNextPathPlanned(false);
			this->setTripChain(trip_chain);
			this->initTripChain();
		}
	}
}


void Person::rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment*>& blacklisted)
{
	//This requires the Role's intervention.
	if (currRole) {
		currRole->rerouteWithBlacklist(blacklisted);
	}
}
void sim_mob::Person::handleAMODEvent(sim_mob::event::EventId id,
            sim_mob::event::Context ctxId,
            sim_mob::event::EventPublisher* sender,
            const AMOD::AMODEventArgs& args)
{
	if(id == event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH)
	{
		//role gets chance to handle event
		if(currRole){
			currRole->onParentEvent(id, ctxId, sender, args);
		}
	}
}



bool sim_mob::Person::frame_init(timeslice now)
{
	messaging::MessageBus::RegisterHandler(this);
	currTick = now;
	//Agents may be created with a null Role and a valid trip chain
	if (!currRole) {
		//TODO: This UpdateStatus has a "prevParams" and "currParams" that should
		//      (one would expect) be dealt with. Where does this happen?
		setStartTime(now.ms());

		UpdateStatus res =	checkTripChain();

		if(currRole) {
			currRole->setArrivalTime(now.ms()+ConfigManager::GetInstance().FullConfig().baseGranMS());
		}

		//Nothing left to do?
		if (res.status == UpdateStatus::RS_DONE) {
			return false;
		}
	}

	//Failsafe: no Role at all?
	if (!currRole) {
		std::ostringstream txt ;
		txt << "Person " << this->getId() <<  " has no Role.";
		throw std::runtime_error(txt.str());
	}

	//Get an UpdateParams instance.
	//TODO: This is quite unsafe, but it's a relic of how Person::update() used to work.
	//      We should replace this eventually (but this will require a larger code cleanup).
	currRole->make_frame_tick_params(now);

	//Now that the Role has been fully constructed, initialize it.
	if((*currTripChainItem)) {
		currRole->Movement()->frame_init();
	}
	return true;
}
void sim_mob::Person::setPath(std::vector<WayPoint>& path)
{
	if (path.size() == 0) {
		Print() << "Warning! Path size is zero!" << std::endl;
	}

	amodPath = path;
}

void sim_mob::Person::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	Agent::onEvent(eventId, ctxId, sender, args);
	if(currRole){
		currRole->onParentEvent(eventId, ctxId, sender, args);
	}
}

 void sim_mob::Person::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
 {
	 if(currRole){
		 currRole->HandleParentMessage(type, message);
	 }
 }

 void sim_mob::Person::handleAMODArrival() {
		sim_mob::AMOD::AMODController *a = sim_mob::AMOD::AMODController::instance();

		//ask the AMODController to handle the arrival
		a->handleVHArrive(this);
 }

 void sim_mob::Person::handleAMODPickup() {
		sim_mob::AMOD::AMODController *a = sim_mob::AMOD::AMODController::instance();

		//ask the AMODController to handle the arrival
		a->handleVHPickup(this);
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

	if (!isToBeRemoved()) { currRole->Movement()->frame_tick(); }

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
		setStartTime(now.ms()+ConfigManager::GetInstance().FullConfig().baseGranMS());

		retVal = checkTripChain();

		if(currTripChainItem != tripChain.end())
		{
			sim_mob::TripChainItem* tcItem =  *currTripChainItem;
			if(tcItem) // if currTripChain not end and has value, call frame_init and switching roles
			{
				if(tcItem->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
				{
					//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
					//since start time of the activity is usually later than what is configured initially,
					//we have to make adjustments so that the person waits for exact amount of time
					sim_mob::ActivityPerformer* ap = dynamic_cast<sim_mob::ActivityPerformer*>(currRole);
					ap->setActivityStartTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS()));
					ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigManager::GetInstance().FullConfig().baseGranMS() + (tcItem->endTime.getValue() - tcItem->startTime.getValue())));
				}
				if(!isInitialized())
				{
					currRole->Movement()->frame_init();
					setInitialized(true);// set to be false so later no need to frame_init later
				}
			}
		}
	}

	return retVal;
}



void sim_mob::Person::frame_output(timeslice now)
{
	//Save the output
	if (!isToBeRemoved()) {
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
	if(!updateNextTripChainItem())
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
	if(tci->itemType == sim_mob::TripChainItem::IT_TRIP)
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
	if( tci->itemType==sim_mob::TripChainItem::IT_TRIP || tci->itemType==sim_mob::TripChainItem::IT_FMODSIM )
	{
		subTrip = &(*currSubTrip);
	}
	if(!newRole) { newRole = rf.createRole(tci, subTrip, this); }
	changeRole(newRole);
	return true;
}

UpdateStatus sim_mob::Person::checkTripChain() {
	//some normal checks
	if(tripChain.empty()) {
		return UpdateStatus::Done;
	}

	//advance the trip, subtrip or activity....
	if(!firstTick) {
		if(!(advanceCurrentTripChainItem())) {
			return UpdateStatus::Done;
		}
	}
	//must be set to false whenever tripchainitem changes. And it has to happen before a probable creation of (or changing to) a new role
	setNextPathPlanned(false);

	//Create a new Role based on the trip chain type
	updatePersonRole(nextRole);

	//Update our origin/dest pair.
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP) { //put if to avoid & evade bustrips, can be removed when everything is ok
		updateOD(*currTripChainItem, &(*currSubTrip));
	}

	//currentTipchainItem or current subtrip are changed
	//so OD will be changed too,
	//therefore we need to call frame_init regardless of change in the role
	resetFrameInit();

	//Create a return type based on the differences in these Roles
	vector<BufferedBase*> prevParams;
	vector<BufferedBase*> currParams;
	if (prevRole) {
		prevParams = prevRole->getSubscriptionParams();
	}
	if (currRole) {
		currParams = currRole->getSubscriptionParams();
	}
	if(firstTick&&currRole) {
		currRole->setArrivalTime(startTime+ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
	firstTick = false;

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return UpdateStatus(UpdateStatus::RS_CONTINUE, prevParams, currParams);
}

//sets the current subtrip to the first subtrip of the provided trip(provided trip is usually the current tripChianItem)
std::vector<sim_mob::SubTrip>::iterator sim_mob::Person::resetCurrSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
	if(!trip) {
		throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
	}
	return trip->getSubTripsRW().begin();
}

void sim_mob::Person::insertWaitingActivityToTrip() {
	std::vector<TripChainItem*>::iterator tripChainItem;
	for (tripChainItem = tripChain.begin(); tripChainItem != tripChain.end();
			++tripChainItem) {
		if ((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP) {
			std::vector<SubTrip>::iterator itSubTrip[2];
			std::vector<sim_mob::SubTrip>& subTrips =
					(dynamic_cast<sim_mob::Trip*>(*tripChainItem))->getSubTripsRW();

			itSubTrip[1] = subTrips.begin();
			itSubTrip[0] = subTrips.begin();
			while (itSubTrip[1] != subTrips.end()) {
				if (itSubTrip[1]->mode == "BusTravel"
						&& itSubTrip[0]->mode != "WaitingBusActivity") {
					if(itSubTrip[1]->fromLocation.type_==WayPoint::BUS_STOP){
						sim_mob::SubTrip subTrip;
						subTrip.itemType = TripChainItem::getItemType(
								"WaitingBusActivity");
						subTrip.fromLocation = itSubTrip[1]->fromLocation;
						subTrip.fromLocationType = itSubTrip[1]->fromLocationType;
						subTrip.toLocation = itSubTrip[1]->toLocation;
						subTrip.toLocationType = itSubTrip[1]->toLocationType;
						subTrip.startLocationId = itSubTrip[1]->fromLocation.busStop_->getBusstopno_();
						subTrip.endLocationId = itSubTrip[1]->fromLocation.busStop_->getBusstopno_();
						subTrip.startLocationType = "BUS_STOP";
						subTrip.endLocationType = "BUS_STOP";
						subTrip.mode = "WaitingBusActivity";
						subTrip.ptLineId = itSubTrip[1]->ptLineId;
						itSubTrip[1] = subTrips.insert(itSubTrip[1], subTrip);
					}
				}

				itSubTrip[0] = itSubTrip[1];
				itSubTrip[1]++;
			}
		}
	}
}

bool sim_mob::Person::makeODsToTrips(SubTrip* curSubTrip,
		std::vector<sim_mob::SubTrip>& newSubTrips,
		const std::vector<sim_mob::OD_Trip>& matchedTrips) {

	bool ret = true;
	bool invalidFlag = false;
	if (matchedTrips.size() > 0) {
		std::vector<sim_mob::OD_Trip>::const_iterator it = matchedTrips.begin();
		while (it != matchedTrips.end()) {
			sim_mob::SubTrip subTrip;
			WayPoint source;
			WayPoint dest;
			std::string sSrc = (*it).startStop;
			std::string sEnd = (*it).endStop;
			std::string srcType;
			std::string endType;
			int sType = (*it).sType;
			int eType = (*it).eType;
			switch (eType) {
			case 0: {
				endType = "NODE";
				int id = boost::lexical_cast<unsigned int>(sEnd);
				sim_mob::Node* node =
						ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(id);
				if (node) {
					dest = WayPoint(node);
				}
				break;
			}
			case 1: {
				endType = "BUS_STOP";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sEnd);
				if (stop) {
					dest = WayPoint(stop);
				}
				break;
			}
			case 2: {
				endType = "MRT_STOP";
				sim_mob::MRT_Stop* stop =
						sim_mob::PT_Network::getInstance().findMRT_Stop(sEnd);
				if (stop) {
					dest = WayPoint(stop);
				}
				break;
			}
			}

			switch (sType) {
			case 0: {
				srcType = "NODE";
				int id = boost::lexical_cast<unsigned int>(sSrc);
				sim_mob::Node* node =
						ConfigManager::GetInstanceRW().FullConfig().getNetworkRW().getNodeById(id);
				if (node) {
					source = WayPoint(node);
				}
				break;
			}
			case 1: {
				srcType = "BUS_STOP";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sSrc);
				if (stop) {
					source = WayPoint(stop);
				}
				break;
			}
			case 2: {
				srcType = "MRT_STOP";
				sim_mob::MRT_Stop* stop =
						sim_mob::PT_Network::getInstance().findMRT_Stop(sSrc);
				if (stop) {
					source = WayPoint(stop);
				}
				break;
			}
			}
			if(it->tType == "Bus" and (sType != 1 or eType != 1))
			{
				invalidFlag = true;
			}
			if(it->tType == "RTS" and (sType != 2 or eType != 2))
			{
				invalidFlag = true;
			}

			if (source.type_ != WayPoint::INVALID
					&& dest.type_ != WayPoint::INVALID and !invalidFlag) {
				subTrip.setPersonID(-1);
				subTrip.itemType = TripChainItem::getItemType("Trip");
				subTrip.sequenceNumber = 1;
				subTrip.startTime = curSubTrip->endTime;
				subTrip.endTime = DailyTime((*it).travelTime*1000.0);
				subTrip.fromLocation = source;
				subTrip.startLocationId = sSrc;
				subTrip.startLocationType = srcType;
				subTrip.endLocationId = sEnd;
				subTrip.endLocationType = endType;
				if (source.type_ == WayPoint::BUS_STOP) {
					subTrip.fromLocationType =
							TripChainItem::LT_PUBLIC_TRANSIT_STOP;
				} else {
					subTrip.fromLocationType = TripChainItem::LT_NODE;
				}
				subTrip.toLocation = dest;
				if (dest.type_ == WayPoint::BUS_STOP) {
					subTrip.toLocationType =
							TripChainItem::LT_PUBLIC_TRANSIT_STOP;
				} else {
					subTrip.toLocationType = TripChainItem::LT_NODE;
				}
				subTrip.tripID = "";
				subTrip.mode = (*it).tType;
				if((*it).tType.find("Walk")!=std::string::npos){
					subTrip.mode = "Walk";
					subTrip.isPT_Walk=true;
					subTrip.walkTime = (*it).walkTime;
				} else if((*it).tType.find("Bus")!=std::string::npos){
					subTrip.mode = "BusTravel";
				} else {
					subTrip.mode = "MRT";
				}
				subTrip.isPrimaryMode = true;
				subTrip.ptLineId = it->serviceLines;
				newSubTrips.push_back(subTrip);
			} else {
				Print()
						<< "[PT pathset]make trip failed:["
						<< sSrc
						<< "]|["
						<< sEnd
						<< "]"<<std::endl;
				ret = false;
				break;
			}
			++it;
		}
	}
	return ret;
}

void sim_mob::Person::convertODsToTrips() {
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	std::vector<TripChainItem*>::iterator tripChainItem;
	bool brokenBusTravel = false;
	std::vector<TripChainItem*>::iterator brokenBusTravelItem;
	for (tripChainItem = tripChain.begin(); tripChainItem != tripChain.end(); ++tripChainItem)
	{
		if(brokenBusTravel) { break; }
		if ((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			Trip* trip = dynamic_cast<Trip*>(*tripChainItem);
			std::vector<sim_mob::OD_Trip> odTrips;
			std::string originId = boost::lexical_cast<std::string>(
					trip->fromLocation.node_->getID());
			std::string destId = boost::lexical_cast<std::string>(
					trip->toLocation.node_->getID());
			(*tripChainItem)->startLocationId = originId;
			(*tripChainItem)->endLocationId = destId;
			std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*>(*tripChainItem))->getSubTripsRW();
			std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
			std::vector<sim_mob::SubTrip> newSubTrips;
			while (itSubTrip != subTrips.end())
			{
				if (itSubTrip->fromLocation.type_ == WayPoint::NODE
						&& itSubTrip->toLocation.type_ == WayPoint::NODE
						&& (itSubTrip->mode == "BusTravel"||itSubTrip->mode == "MRT"))
				{
					std::vector<sim_mob::OD_Trip> odTrips;
					std::string originId = boost::lexical_cast<std::string>(
							itSubTrip->fromLocation.node_->getID());
					std::string destId = boost::lexical_cast<std::string>(
							itSubTrip->toLocation.node_->getID());

					bool ret = sim_mob::PT_RouteChoiceLuaModel::Instance()->GetBestPT_Path(originId, destId, odTrips);

					if (ret) {
						ret = makeODsToTrips(&(*itSubTrip), newSubTrips, odTrips);
					}
					if(!ret){
						tripChain.clear();
						return;
					}
				}
				++itSubTrip;
			}

			if (!newSubTrips.empty())
			{
				subTrips.clear();
				subTrips = newSubTrips;
			}
		}
	}

	if(brokenBusTravel)
	{
		tripChainItem = brokenBusTravelItem;
		while(tripChainItem!=tripChain.end())
		{
			delete *tripChainItem;
			tripChainItem = tripChain.erase(tripChainItem);
		}
	}
}

void sim_mob::Person::simplyModifyTripChain(std::vector<TripChainItem*>& tripChain)
{
	std::vector<TripChainItem*>::iterator tripChainItem;
	for(tripChainItem = tripChain.begin(); tripChainItem != tripChain.end(); ++tripChainItem)
	{
		if((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP )
		{
			std::vector<SubTrip>::iterator subChainItem1, subChainItem2;
			std::vector<sim_mob::SubTrip>& subtrip = (dynamic_cast<sim_mob::Trip*>(*tripChainItem))->getSubTripsRW();

			subChainItem2 = subChainItem1 = subtrip.begin();
			++subChainItem2;
			vector<SubTrip> newsubchain;
			newsubchain.push_back(*subChainItem1);
			while(subChainItem1!=subtrip.end() && subChainItem2!=subtrip.end() )
			{
				WayPoint source, destination;
				if( (subChainItem1->mode=="Walk") && (subChainItem2->mode=="BusTravel") )
				{
					BusStopFinder finder(subChainItem2->fromLocation.node_, subChainItem2->toLocation.node_);
					if(finder.getSourceBusStop())
					{
						source = subChainItem1->toLocation;
						destination = WayPoint(finder.getSourceBusStop());
					}
				}
				else if((subChainItem2->mode=="Walk") && (subChainItem1->mode=="BusTravel"))
				{
					BusStopFinder finder(subChainItem1->fromLocation.node_, subChainItem1->toLocation.node_);
					if(finder.getSourceBusStop() && finder.getDestinationBusStop())
					{
						source = WayPoint(finder.getDestinationBusStop());
						destination = subChainItem2->fromLocation;
					}
				}
				if(source.type_!=WayPoint::INVALID && destination.type_!=WayPoint::INVALID )
				{
					sim_mob::SubTrip subTrip;
					subTrip.setPersonID(-1);
					subTrip.itemType = TripChainItem::getItemType("Trip");
					subTrip.sequenceNumber = 1;
					subTrip.startTime = subChainItem1->endTime;
					subTrip.endTime = subChainItem1->endTime;
					subTrip.fromLocation = source;
					subTrip.fromLocationType = subChainItem1->fromLocationType;
					subTrip.toLocation = destination;
					subTrip.toLocationType = subChainItem2->toLocationType;
					subTrip.tripID = "";
					subTrip.mode = "Walk";
					subTrip.isPrimaryMode = true;
					subTrip.ptLineId = "";

					if(destination.type_==WayPoint::BUS_STOP )
						subChainItem1->toLocation = destination;
					else if(source.type_==WayPoint::BUS_STOP)
						subChainItem2->fromLocation = source;

					newsubchain.push_back(subTrip);
				}

				newsubchain.push_back(*subChainItem2);
				subChainItem1 = subChainItem2;
				++subChainItem2;

				if(subChainItem1==subtrip.end() || subChainItem2==subtrip.end())
					break;
			}
		}
	}
}

bool sim_mob::Person::insertTripBeforeCurrentTrip(Trip* newone)
{
	bool ret = false;
	if(dynamic_cast<sim_mob::Trip*>(*currTripChainItem))
	{
		currTripChainItem = tripChain.insert(currTripChainItem, newone);
		ret = true;
	}
	return ret;
}

bool sim_mob::Person::insertSubTripBeforeCurrentSubTrip(SubTrip* newone)
{
	bool ret = false;
	if(dynamic_cast<sim_mob::Trip*>(*currTripChainItem))
	{
		std::vector<sim_mob::SubTrip>& subtrip = (dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW();
		currSubTrip = subtrip.insert(currSubTrip, *newone);
		ret = true;
	}
	return ret;
}

//only affect items after current trip chain item
bool sim_mob::Person::insertATripChainItem(TripChainItem* before, TripChainItem* newone)
{
	bool ret = false;
	if((dynamic_cast<SubTrip*>(newone)))
	{
		sim_mob::SubTrip* before2 = dynamic_cast<sim_mob::SubTrip*> (before);
		if(before2)
		{
			std::vector<sim_mob::SubTrip>& subtrip = (dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW();
			++currSubTrip;
			std::vector<SubTrip>::iterator itfinder2 = std::find(currSubTrip, subtrip.end(), *before2);
			if( itfinder2 != subtrip.end())
			{
				sim_mob::SubTrip* newone2 = dynamic_cast<sim_mob::SubTrip*> (newone);
				subtrip.insert(itfinder2, *newone2);
			}

		}
	}
	else if((dynamic_cast<Trip*>(newone)))
	{
		std::vector<TripChainItem*>::iterator itfinder;
		itfinder = std::find(currTripChainItem, tripChain.end(), before);
		if(itfinder!=currTripChainItem && itfinder!=tripChain.end())
		{
			tripChain.insert(itfinder, newone );
			ret = true;
		}
	}

	return ret;
}

bool sim_mob::Person::deleteATripChainItem(TripChainItem* del)
{
	bool ret = false;
	std::vector<TripChainItem*>::iterator itfinder;
	itfinder = std::find(currTripChainItem, tripChain.end(), del);
	if(itfinder!=currTripChainItem && itfinder!=tripChain.end())
	{
		tripChain.erase(itfinder);
		ret = true;
	}
	else if((dynamic_cast<SubTrip*>(del)))
	{
		std::vector<TripChainItem*>::iterator itfinder = currTripChainItem;
		for(++itfinder; itfinder!=tripChain.end(); ++itfinder)
		{
			std::vector<sim_mob::SubTrip>& subtrip = (dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW();
			SubTrip temp = *(dynamic_cast<SubTrip*>(del));
			std::vector<SubTrip>::iterator itfinder2 = std::find(subtrip.begin(), subtrip.end(), temp);
			if(itfinder2!=subtrip.end())
			{
				subtrip.erase(itfinder2);
				ret = true;
				break;
			}
		}
	}
	return ret;
}

bool sim_mob::Person::replaceATripChainItem(TripChainItem* rep, TripChainItem* newone)
{
	bool ret = false;
	std::vector<TripChainItem*>::iterator itfinder;
	itfinder = std::find(currTripChainItem, tripChain.end(), rep);
	if(itfinder!=currTripChainItem && itfinder!=tripChain.end())
	{
		(*itfinder) = newone;
		ret = true;
	}
	return ret;

}

bool sim_mob::Person::updateNextSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
	if(!trip) { return false; }
	if (currSubTrip == trip->getSubTrips().end()) { return false; } //just a routine check
	nextSubTrip = currSubTrip + 1;
	if(nextSubTrip == trip->getSubTrips().end()) { return false; }
	return true;
}

bool sim_mob::Person::updateNextTripChainItem()
{
	bool res = false;
	if(currTripChainItem == tripChain.end()) { return false; } //just a harmless basic check
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//check for next subtrip first
		res = updateNextSubTrip();
	}

	if(res)
	{
		nextTripChainItem = currTripChainItem;
		return res;
	}

	//no, it is not the subtrip we need to advance, it is the tripchain item
	nextTripChainItem = currTripChainItem + 1;
	if(nextTripChainItem == tripChain.end()) { return false; }

	//Also set the nextSubTrip to the beginning of trip , just in case
	if((*nextTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*nextTripChainItem);//easy reading
		if(!trip) { throw std::runtime_error("non sim_mob::Trip cannot have subtrips"); }
		nextSubTrip =  trip->getSubTrips().begin();
	}

	return true;
}

//advance to the next subtrip inside the current TripChainItem
bool sim_mob::Person::advanceCurrentSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
	if(!trip) {
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end()) /*just a routine check*/ {
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

	if (currSubTrip == trip->getSubTrips().end()) {
		return false;
	}
	return true;
}

bool sim_mob::Person::advanceCurrentTripChainItem()
{
	if(currTripChainItem == tripChain.end()) /*just a harmless basic check*/ {
		return false;
	}

	// current role (activity or sub-trip level role)[for now: only subtrip] is about to change, time to collect its movement metrics(even activity performer)
	if(currRole != nullptr)
	{
		TravelMetric currRoleMetrics = currRole->Movement()->finalizeTravelTimeMetric();
		currRole->Movement()->resetTravelTimeMetric();//sorry for manual reset, just a precaution for now
		serializeSubTripChainItemTravelTimeMetrics(currRoleMetrics,currTripChainItem,currSubTrip);
	}

	//first check if you just need to advance the subtrip
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP || (*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_FMODSIM )
	{
		//don't advance to next tripchainItem immediately, check the subtrip first
		bool res = advanceCurrentSubTrip();
		//	subtrip advanced successfully, no need to advance currTripChainItem
		if(res) {
			return res;
		}
	}

	//if you are here, TripchainItem has to be incremented
	//Trip is about the change, it is a good time to collect the Metrics
//	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP){
//		aggregateSubTripMetrics();
//	}

	//serializeTripChainItem(currTripChainItem);

	//	do the increment
	++currTripChainItem;


	if (currTripChainItem == tripChain.end())  {
		//but tripchain items are also over, get out !
		return false;
	}

	//so far, advancing the tripchainitem has been successful
	//Also set the currSubTrip to the beginning of trip , just in case
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP  || (*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_FMODSIM) {
		currSubTrip = resetCurrSubTrip();

	}
	return true;
}

void sim_mob::Person::buildSubscriptionList(vector<BufferedBase*>& subsList) {
	//First, add the x and y co-ordinates
	Agent::buildSubscriptionList(subsList);

	//Now, add our own properties.
	if (this->getRole()) {
		vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();
		for (vector<BufferedBase*>::iterator it = roleParams.begin(); it != roleParams.end(); ++it) {
			subsList.push_back(*it);
		}
	}
}

//Role changing should always be done through changeRole, because it also manages subscriptions.
//TODO: Currently, this is also done via the return value to worker. Probably best not to have
//      the same code in two places.
void sim_mob::Person::changeRole(sim_mob::Role* newRole) {
	if (currRole) {
		currRole->setParent(nullptr);
		if (this->currWorkerProvider) {
			this->currWorkerProvider->stopManaging(currRole->getSubscriptionParams());
			this->currWorkerProvider->stopManaging(currRole->getDriverRequestParams().asVector());
		}
	}
	prevRole = currRole;
	currRole = newRole;

	if (currRole) {
		currRole->setParent(this);
		if (this->currWorkerProvider) {
			this->currWorkerProvider->beginManaging(currRole->getSubscriptionParams());
			this->currWorkerProvider->beginManaging(currRole->getDriverRequestParams().asVector());
		}
	}
}

sim_mob::Role* sim_mob::Person::getRole() const {
	return currRole;
}

sim_mob::Role* sim_mob::Person::getPrevRole() const {
	return prevRole;
}

void sim_mob::Person::setNextRole(sim_mob::Role* newRole)
{
	nextRole = newRole;
}

sim_mob::Role* sim_mob::Person::getNextRole() const {
	return nextRole;
}

void sim_mob::Person::setPersonCharacteristics()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::map<int, PersonCharacteristics>& personCharacteristics = config.personCharacteristicsParams.personCharacteristics;

	int lowestAge = DEFAULT_LOWEST_AGE;
	int highestAge = DEFAULT_HIGHEST_AGE;
	// if no personCharacteristics item in the config file, introduce default lowestAge and highestAge
	if (!personCharacteristics.empty()) {
		lowestAge = config.personCharacteristicsParams.lowestAge;
		highestAge = config.personCharacteristicsParams.highestAge;
	}
	const int defaultLowerSecs = config.personCharacteristicsParams.DEFAULT_LOWER_SECS;
	const int defaultUpperSecs = config.personCharacteristicsParams.DEFAULT_UPPER_SECS;
	boost::mt19937 gen(static_cast<unsigned int>(getId()*getId()));
	boost::uniform_int<> ageRange(lowestAge, highestAge);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAge(gen, ageRange);
	age = (unsigned int)varAge();
	boost::uniform_int<> BoardingTime(defaultLowerSecs, defaultUpperSecs);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varBoardingTime(gen, BoardingTime);
	boardingTimeSecs = varBoardingTime();

	boost::uniform_int<> AlightingTime(defaultLowerSecs, defaultUpperSecs);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAlightingTime(gen, AlightingTime);
	alightingTimeSecs = varAlightingTime();

	for(std::map<int, PersonCharacteristics>::const_iterator iter=personCharacteristics.begin();iter != personCharacteristics.end(); ++iter) {
		if(age >= iter->second.lowerAge && age < iter->second.upperAge) {
			boost::uniform_int<> BoardingTime(iter->second.lowerSecs, iter->second.upperSecs);
			boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varBoardingTime(gen, BoardingTime);
			boardingTimeSecs = varBoardingTime();

			boost::uniform_int<> AlightingTime(iter->second.lowerSecs, iter->second.upperSecs);
			boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAlightingTime(gen, AlightingTime);
			alightingTimeSecs = varAlightingTime();
		}
	}
}

void sim_mob::Person::printTripChainItemTypes() const{
	std::stringstream ss;
	ss << "Person: " << id << "|TripChain: "<<std::endl;
	for(std::vector<TripChainItem*>::const_iterator tci=tripChain.begin(); tci!=tripChain.end(); ++tci)
	{
		const TripChainItem* tcItem = *tci;
		switch(tcItem->itemType)
		{
		case TripChainItem::IT_TRIP:
		{
			ss << "Person: " << id << "|TripChain: ";
			const Trip* trip = dynamic_cast<const Trip*>(tcItem);
			const std::vector<sim_mob::SubTrip>& subTrips = trip->getSubTrips();
			for(std::vector<SubTrip>::const_iterator sti=subTrips.begin(); sti!=subTrips.end(); ++sti)
			{
				ss << "|" << tcItem->getMode() << "-trip->";
			}
			break;
		}
		case TripChainItem::IT_ACTIVITY:
			ss << "|activity";
			break;
		case TripChainItem::IT_BUSTRIP:
			ss << "|bus-trip";
			break;
		case TripChainItem::IT_FMODSIM:
			ss << "|fmod-trip";
			break;
		case TripChainItem::IT_WAITBUSACTIVITY:
			ss << "|waitbus-activity";
			break;
		}
	}
	ss << std::endl;
	Print() << ss.str();
}
/********************************************************************
 * ************* Collection and presentation of metrics *************
 * ******************************************************************
 */

 namespace
 {
 	 sim_mob::BasicLogger & csv = sim_mob::Logger::log(ConfigManager::GetInstance().FullConfig().pathSet().subTripOP);
 	 sim_mob::OneTimeFlag titleSubPredayTT;
 }

 void sim_mob::Person::serializeSubTripChainItemTravelTimeMetrics(
		 const TravelMetric& subtripMetrics,
		 std::vector<TripChainItem*>::iterator currTripChainItem,
		 std::vector<SubTrip>::iterator currSubTrip) const
 {
	 if(!(subtripMetrics.finalized && subtripMetrics.started)) { return; } //sanity check
	 if(titleSubPredayTT.check())
	 {
		 csv << "person_id,trip_id,subtrip_id,origin,destination,mode,start_time,end_time,travel_time,total_distance,ptt_wt,pt_walk,cbd_entry_node,cbd_exit_node,cbd_entry_time,cbd_exit_time,cbd_travel_time,non_cbd_travel_time,cbd_distance,non_cbd_distance\n";
	 }

	 sim_mob::SubTrip &st = (*currSubTrip);//easy reading
	 // restricted area which is to be appended at the end of the csv line
	 std::stringstream restrictedRegion("");
	 if(st.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER || st.cbdTraverseType == sim_mob::TravelMetric::CBD_EXIT)
	 {
		 //sanity check
		 if(!(subtripMetrics.cbdOrigin.node_&&subtripMetrics.cbdDestination.node_))
		 {
			 restrictedRegion <<
					 subtripMetrics.origin.node_->getID() << "," <<
					 subtripMetrics.destination.node_->getID() <<
					 (st.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER ? " , Enter ": "  ,Exit") <<
					  " has null values " <<
					 (subtripMetrics.cbdOrigin.node_ != nullptr ? subtripMetrics.cbdOrigin.node_ : 0) << "," <<
					 (subtripMetrics.cbdDestination.node_ != nullptr ? subtripMetrics.cbdDestination.node_ : 0) << "\n";
		 }
		 else //valid scenario:
		 {
			 restrictedRegion <<
					 subtripMetrics.cbdOrigin.node_->getID() << "," <<											//	cbd_entry_node
					 subtripMetrics.cbdDestination.node_->getID() << "," <<										//	cbd_exit_node
					 subtripMetrics.cbdStartTime.getRepr_() << ","  <<											//	cbd_entry_time
					 subtripMetrics.cbdEndTime.getRepr_() << ","  <<											//	cbd_exit_time
					 subtripMetrics.cbdTravelTime << ","  <<													//	cbd_travel_time
					 (subtripMetrics.travelTime - subtripMetrics.cbdTravelTime) << "," <<						//	non_cbd_travel_time
					 subtripMetrics.cbdDistance << "," <<	 	 	 	 	 	 	 	 	 	 	 	 	 	//	cbd_distance
					 (subtripMetrics.distance - subtripMetrics.cbdDistance) ;		 	 	 	 	 	 	 	//	non_cbd_distance
		 }
	 }
	 else// if Agent never entered or exitted CBD
	 {
		 restrictedRegion<<
				 "0" << "," <<																				//	cbd_entry_node
				 "0" << "," <<																				//	cbd_exit_node
				 "00:00:00" << ","  <<																		//	cbd_entry_time
				 "00:00:00" << ","  <<																		//	cbd_exit_time
				 "0" << ","  <<																				//	cbd_travel_time
				 subtripMetrics.travelTime << "," <<														//	non_cbd_travel_time
				 "0" << "," <<	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 					//	cbd_distance
				 subtripMetrics.distance ;			 	 	 	 	 	 	 	 	 	 	 	 	 	 	//	non_cbd_distance
	 }

	 std::stringstream res("");
	 // actual writing
	 res <<
			 this->getId() << "," <<																		//	person_id
			 (static_cast<Trip*>(*currTripChainItem))->tripID  << "," <<									//	trip_id
			 st.tripID  << "," <<																			//	subtrip_id
			 st.fromLocation.node_->getID() << "," <<														//	origin
			 st.toLocation.node_->getID() << "," <<															//	destination
			 st.mode  << "," <<																				//	mode
			 subtripMetrics.startTime.getRepr_()  << "," <<													//	start_time
			 subtripMetrics.endTime.getRepr_()  << "," <<													//	end_time
			 //			 TravelMetric::getTimeDiffHours(subtripMetrics.endTime, subtripMetrics.startTime)  << ","		//	travel_time### commented
			 subtripMetrics.travelTime << "," <<															//	travel_time
			 subtripMetrics.distance << "," <<	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	//	total_distance
			 "0" << ","	<<	//placeholder for paublic transit's waiting time								//	ptt_wt
			 "0" << "," << //placeholder for paublic transit's walk time									//	pt_walk
			 restrictedRegion.str() << "\n";																/* MIXED CBD Information */

	 csv << res.str();
	 std::cout << res.str();
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

	sim_mob::Trip * trip = dynamic_cast<Trip*>(*currTripChainItem);
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
		trip->fromLocation.node_->getID() << "," <<
		(trip->fromLocationType == TripChainItem::LT_NODE ? "node" : "stop") << "," <<
		trip->toLocation.node_->getID()<< ","   <<
		(trip->toLocationType == TripChainItem::LT_NODE ? "node" : "stop") << "," ;

	//step-2 subtrip information part 1
	const sim_mob::SubTrip &st = *currSubTrip;
	std::stringstream subTripStrm_1("");
	subTripStrm_1 <<
		st.tripID <<  "," <<
		metric.cbdOrigin.node_->getID() <<  "," <<			//replace original subtrip
		"Node" <<  "," <<
		metric.cbdDestination.node_->getID() <<  "," <<
		"Node" <<  "," <<
		st.mode <<  "," <<
		(st.isPrimaryMode ? "TRUE" : "FALSE") <<  "," <<
		st.startTime.getRepr_()  <<  "," <<
		st.ptLineId;

	//step-3 activity, part 1
	std::stringstream activity_1("");
	activity_1 << ","
			<< "0" << ","		//activity_id
			<< "dummy" << ","	//activity_type
			<< "FALSE" << ","	//is_primary_activity
			<< "FALSE" << ","	//flexible_activity
			<< "TRUE" << ","	//mandatory_activity
			<< "0" << ","		//mandatory_activity
			<< "0" << ","		//activity_location
			<< "node" << "," 	//activity_loc_type
			<< "," 				//activity_start_time
			<< "" 				//activity_end_time
			 ;


	//step-4 INTEGRATION with sub trip:
	//order:
	//trip information part 1 :  tripStrm_1
	//subtrip 'custom info' : subTripStrm_1
	//activity part 1 : activity_1
	std::stringstream line("");
	line <<
			tripStrm_1.str() <<  "," <<
			subTripStrm_1.str() <<  "," <<
			activity_1.str() ;
	sim_mob::Logger::log("tripchain_info_for_short_term.csv") << line.str();
 }

 void sim_mob::Person::serializeCBD_Activity(const TravelMetric &metric)
 {
	//sanity checks

	sim_mob::Activity * activity = dynamic_cast<Activity*>(*currTripChainItem);
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
	if(subTripTravelMetrics.begin() == subTripTravelMetrics.end())
	{
		return;
	}
	TravelMetric newTripMetric;
	std::vector<TravelMetric>::iterator item(subTripTravelMetrics.begin());
	newTripMetric.startTime = item->startTime;//first item
	newTripMetric.origin = item->origin;
	for(;item !=subTripTravelMetrics.end(); ++item)
	{
		newTripMetric.travelTime += item->travelTime;
	}
	newTripMetric.endTime = subTripTravelMetrics.rbegin()->endTime;
	newTripMetric.destination = subTripTravelMetrics.rbegin()->destination;
	subTripTravelMetrics.clear();
	tripTravelMetrics.push_back(newTripMetric);
}

void sim_mob::Person::addSubtripTravelMetrics(TravelMetric &value){
	 subTripTravelMetrics.push_back(value);
 }
/**
 * Serializer for Trip level travel time
 */
 void sim_mob::Person::serializeTripTravelTimeMetrics()
 {
	 if(tripTravelMetrics.empty())
	 {
		 return;
	 }
	 sim_mob::BasicLogger & csv = sim_mob::Logger::log("trip_level_travel_time.csv");
	 BOOST_FOREACH(TravelMetric &item, tripTravelMetrics)
	 {
		 csv << this->getId() << "," <<
				 item.origin.node_->getID() << ","
				 << item.destination.node_->getID() << ","
				 << item.startTime.getRepr_() << ","
				 << item.endTime.getRepr_() << ","
				 << (item.endTime - item.startTime).getRepr_()
				 << "\n";
	 }
	 tripTravelMetrics.clear();
 }
