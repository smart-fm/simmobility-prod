/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"

#include <algorithm>

//For debugging
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "util/OutputUtil.hpp"

#include "geospatial/Node.hpp"
#include "entities/misc/TripChain.hpp"
#include "workers/Worker.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/misc/TripChain.hpp"
#endif

using std::map;
using std::string;
using std::vector;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


namespace {
Trip* MakePseudoTrip(const Person& ag, const std::string& mode)
{
	//Make sure we have something to work with
	if (!(ag.originNode && ag.destNode)) {
		throw std::runtime_error("Can't make a pseudo-trip for an Agent with no origin and destination.");
	}

	//Make the trip itself
	Trip* res = new Trip();
	res->personID = ag.getId();
	res->itemType = TripChainItem::getItemType("Trip");
	res->sequenceNumber = 1;
	res->startTime = DailyTime(ag.getStartTime());  //TODO: This may not be 100% correct
	res->endTime = res->startTime; //No estimated end time.
	res->tripID = 0;
	res->fromLocation = ag.originNode;
	res->fromLocationType = TripChainItem::getLocationType("node");
	res->toLocation = ag.destNode;
	res->toLocationType = res->fromLocationType;

	//SubTrip generatedSubTrip(-1, "Trip", 1, DailyTime(candidate.start), DailyTime(),
	//candidate.origin, "node", candidate.dest, "node", "Car", true, "");

	//Make and assign a single sub-trip
	sim_mob::SubTrip subTrip;
	subTrip.personID = -1;
	subTrip.itemType = TripChainItem::getItemType("Trip");
	subTrip.sequenceNumber = 1;
	subTrip.startTime = res->startTime;
	subTrip.endTime = res->startTime;
	subTrip.fromLocation = res->fromLocation;
	subTrip.fromLocationType = res->fromLocationType;
	subTrip.toLocation = res->toLocation;
	subTrip.toLocationType = res->toLocationType;
	subTrip.tripID = 0;
	subTrip.mode = mode;
	subTrip.isPrimaryMode = true;
	subTrip.ptLineId = "";

	//Add it to the Trip; return this value.
	res->addSubTrip(subTrip);
	return res;
}

}  //End unnamed namespace


sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, unsigned int id) :
	Agent(mtxStrat, id), prevRole(nullptr), currRole(nullptr), agentSrc(src), currTripChainItem(nullptr), currSubTrip(nullptr), firstFrameTick(true)

{
	//throw 1;
}

sim_mob::Person::~Person() {
	safe_delete_item(currRole);
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

	//Consistency check: are they requesting a pseudo-trip chain when they actually have one?
	map<string, string>::const_iterator origIt = configProps.find("originPos");
	map<string, string>::const_iterator destIt = configProps.find("destPos");
	if (origIt!=configProps.end() && destIt!=configProps.end()) {
		if (!tripChain.empty()) {
			throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
		}

		if (this->originNode || this->destNode) {
			throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
		}

		//Otherwise, make a trip chain for this Person.

		std::cout << "sim_mob::Person::load=>input[" << origIt->second << " , " << destIt->second << "]\n";
		this->originNode = ConfigParams::GetInstance().getNetwork().locateNode(parse_point(origIt->second), true);
		this->destNode = ConfigParams::GetInstance().getNetwork().locateNode(parse_point(destIt->second), true);
		std::cout << "Resulting nodes[" << this->originNode << " , " << this->destNode << "]\n";

		//Make sure they have a mode specified for this trip
		it = configProps.find("#mode");
		if (it==configProps.end()) {
			throw std::runtime_error("Cannot load person: no mode");
		}
		std::string mode = it->second;

		Trip* singleTrip = MakePseudoTrip(*this, mode);

		std::vector<const TripChainItem*> trip_chain;
		trip_chain.push_back(singleTrip);

		//////
		//////TODO: Some of this should be performed in a centralized place; e.g., "Agent::setTripChain"
		//////
		////////TODO: This needs to go in a centralized place.
		this->originNode = singleTrip->fromLocation;
		this->destNode = singleTrip->toLocation;
		this->setNextPathPlanned(false);
		this->setTripChain(trip_chain);
	}

	//One more check: If they have a special string, save it now
	it = configProps.find("special");
	if (it != configProps.end()) {
		this->specialStr = it->second;
	}

	///
	///TODO: At some point, we need to check if "origin->dest" paths are valid.
	///      This should be an option that can be turned on in the config file, and it
	///      allows us to remove badly-specified agents before they generate an error
	///      in frame_init.
	///
	 /*const bool checkBadPaths = true;
	 //Optional: Only add this Agent if a path exists for it from start to finish.
	  StreetDirectory& sd = StreetDirectory::instance();
	if (foundOrigPos && foundDestPos && checkBadPaths) {
		bool skip = false;
		if (agentType=="pedestrian") {
			//For now, pedestrians can't have invalid routes.
			//skip = true;
			//vector<WayPoint> path = sd.shortestWalkingPath(agent->originNode->location, agent->destNode->location);
			//for (vector<WayPoint>::iterator it=path.begin(); it!=path.end(); it++) {
			//	if (it->type_ == WayPoint::SIDE_WALK) {
			//		skip = false;
			//		break;
			//	}
			//}
		} else if (agentType=="driver" || agentType=="bus") {
			skip = true;
			vector<WayPoint> path = sd.shortestDrivingPath(*candidate.origin, *candidate.dest);
			for (vector<WayPoint>::iterator it=path.begin(); it!=path.end(); it++) {
				if (it->type_ == WayPoint::ROAD_SEGMENT) {
					skip = false;
					break;
				}
			}
		}

		//Is this Agent invalid?
		if (skip) {
			std::cout <<"Skipping agent; can't find route from: " <<(candidate.origin?candidate.origin->originalDB_ID.getLogItem():"<null>") <<" to: " <<(candidate.dest?candidate.dest->originalDB_ID.getLogItem():"<null>");
			if (candidate.origin && candidate.dest) {
				std::cout <<"   {" <<candidate.origin->location <<"=>" <<candidate.dest->location <<"}";
			}
			std::cout <<std::endl;

			config.numAgentsSkipped++;
			safe_delete_item(candidate.rawAgent);
			continue;
		}*/
}



void sim_mob::Person::getNextSubTripInTrip(){
	if(!currTripChainItem || currTripChainItem->itemType == sim_mob::TripChainItem::IT_ACTIVITY){
		currSubTrip = nullptr;
	}
	else if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_TRIP){
		const Trip* currTrip = dynamic_cast<const Trip*>(this->currTripChainItem);
		const vector<SubTrip>& currSubTripsList = currTrip->getSubTrips();
		if(!currSubTrip) {
			//Return the first sub trip if the current sub trip which is passed in is null
			currSubTrip = &currSubTripsList.front();
		} else {
			// else return the next sub trip if available; return nullptr otherwise.
			vector<SubTrip>::const_iterator subTripIterator = std::find(currSubTripsList.begin(), currSubTripsList.end(), (*currSubTrip));
			currSubTrip = nullptr;
			if (subTripIterator!=currSubTripsList.end()) {
				//Set it equal to the next item, assuming we are not at the end of the list.
				if (++subTripIterator != currSubTripsList.end()) {
					currSubTrip = &(*subTripIterator);
				}
			}
		}
	} else if (this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_BUSTRIP) {
		std::cout << "BusTrip happens " << std::endl;
	} else{
		throw std::runtime_error("Invalid trip chain item type!");
	}
}

void sim_mob::Person::findNextItemInTripChain() {
	if(!this->currTripChainItem) {
		//set the first item if the current item is null
		this->currTripChainItem = this->tripChain.front();
	} else {
		// else set the next item if available; return nullptr otherwise.
		std::vector<const TripChainItem*>::const_iterator itemIterator = std::find(tripChain.begin(), tripChain.end(), currTripChainItem);
		currTripChainItem = nullptr;
		if (itemIterator!=tripChain.end()) {
			//Set it equal to the next item, assuming we are not at the end of the list.
			if (++itemIterator != tripChain.end()) {
				currTripChainItem = *itemIterator;
			}
		}
	}

	this->getNextSubTripInTrip(); //if currTripChainItem is Trip, set currSubTrip as well
}


void sim_mob::Person::update_time(timeslice now, UpdateStatus& retVal)
{
	//Agents may be created with a null Role and a valid trip chain
	if (firstFrameTick && !currRole) {
		if(this->getId() == 555) {
			std::cout << "555 is available " << std::endl;
		}
		checkAndReactToTripChain(now);
	}

	//Failsafe
	if (!currRole) {
		throw std::runtime_error("Person has no Role.");
	}

	//Get an UpdateParams instance.
	UpdateParams& params = currRole->make_frame_tick_params(now);
	//std::cout<<"Person ID:"<<this->getId()<<"---->"<<"Person position:"<<"("<<this->xPos<<","<<this->yPos<<")"<<std::endl;

	//Has update() been called early?
	if (now.ms()<getStartTime()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			std::stringstream msg;
			msg << "Agent(" << getId() << ") specifies a start time of: " << getStartTime()
					<< " but it is currently: " << now.ms()
					<< "; this indicates an error, and should be handled automatically.";
			throw std::runtime_error(msg.str().c_str());
		}
		retVal = UpdateStatus::Continue;
		return;
	}

	//Has update() been called too late?
	if (isToBeRemoved()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("Agent is already done, but hasn't been removed.");
		}
		retVal = UpdateStatus::Continue;
		return;
	}

	//Is this the first frame tick for this Agent?
	if (firstFrameTick) {
		//Helper check; not needed once we trust our Workers.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			if (abs(now.ms()-getStartTime())>=ConfigParams::GetInstance().baseGranMS) {
				std::stringstream msg;
				msg << "Agent was not started within one timespan of its requested start time.";
				msg << "\nStart was: " << getStartTime() << ",  Curr time is: " << now.ms() << "\n";
				msg << "Agent ID: " << getId() << "\n";
				throw std::runtime_error(msg.str().c_str());
			}
		}

		//Now that the Role has been fully constructed, initialize it.
		currRole->frame_init(params);

		//Done
		firstFrameTick = false;
	}

	//Now perform the main update tick
	if (!isToBeRemoved()) {
		//added to get the detailed plan before next activity
		currRole->frame_tick(params);
		//if mid-term
		//currRole->frame_tick_med(params);
	}

	//Finally, save the output
	if (!isToBeRemoved()) {
		currRole->frame_tick_output(params);
		//if mid-term
		//currRole->frame_tick_med(params);
	}

	//If we're "done", try checking to see if we have any more items in our Trip Chain.
	// This is not strictly the right way to do things (we shouldn't use "isToBeRemoved()"
	// in this manner), but it's the easiest solution that uses the current API.
	if (isToBeRemoved()) {
		retVal = checkAndReactToTripChain(now.ms(), now.ms()+ConfigParams::GetInstance().baseGranMS);
	}

	//Output if removal requested.
	if (Debug::WorkGroupSemantics && isToBeRemoved()) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "Person requested removal: " <<"(Role Hidden)" << "\n";
#endif
	}
}


UpdateStatus sim_mob::Person::update(timeslice now) {
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
		profile.logAgentUpdateBegin(*this, frameNumber);
#endif

	//First, we need to retrieve an UpdateParams subclass appropriate for this Agent.
	//unsigned int currTimeMS = now.frame() * ConfigParams::GetInstance().baseGranMS;

	//Update within an optional try/catch block.
	UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

#ifndef SIMMOB_STRICT_AGENT_ERRORS
	try {
#endif

		//Update functionality
		update_time(now, retVal);

//Respond to errors only if STRICT is off; otherwise, throw it (so we can catch it in the debugger).
#ifndef SIMMOB_STRICT_AGENT_ERRORS
	} catch (std::exception& ex) {
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
		profile.logAgentException(*this, frameNumber, ex);
#endif

		//Add a line to the output file.
#ifndef SIMMOB_DISABLE_OUTPUT
		std::stringstream msg;
		msg <<"Error updating Agent[" <<getId() <<"], will be removed from the simulation.";
		msg <<"\nFrom node: " <<(originNode?originNode->originalDB_ID.getLogItem():"<Unknown>");
		msg <<"\nTo node: " <<(destNode?destNode->originalDB_ID.getLogItem():"<Unknown>");
		msg <<"\n" <<ex.what();
		LogOut(msg.str() <<std::endl);
#endif
		setToBeRemoved();
	}
#endif

	//Return true unless we are scheduled for removal.
	//NOTE: Make sure you set this flag AFTER performing your final output.
	if (isToBeRemoved()) {
		retVal.status = UpdateStatus::RS_DONE;
	}

#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	profile.logAgentUpdateEnd(*this, frameNumber);
#endif

	return retVal;
}


UpdateStatus sim_mob::Person::checkAndReactToTripChain(unsigned int currTimeMS, unsigned int nextValidTimeMS) {
	this->getNextSubTripInTrip();

	if(!this->currSubTrip){
		this->findNextItemInTripChain();
	}

	if (!this->currTripChainItem) {
		return UpdateStatus::Done;
	}

	//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
	// it's better to avoid possible errors (e.g., if the equality operator is defined)
	// by saving it until the next time tick.
	safe_delete_item(prevRole);
	prevRole = currRole;

	//Create a new Role based on the trip chain type
	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	changeRole(rf.createRole(this->currTripChainItem, this));

	//Update our origin/dest pair.
	//TODO: We need to make TripChainItems a little friendlier in terms of
	//      getting the origin/dest (which can be the same or null for Activity types)
	if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_TRIP){
		originNode = this->currSubTrip->fromLocation;
		destNode = this->currSubTrip->toLocation;
	} else if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_BUSTRIP) {
		std::cout << "BusTrip happens " << std::endl;
	} else {
		originNode = dynamic_cast<const Activity&>(*currTripChainItem).location;
		destNode = originNode;
	}

	//Activities require some additional work
	//TODO: A more centralized way of doing this is necessary; we can't be casting directly
	//      to an ActivityPerformer.
	if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_ACTIVITY){
		ActivityPerformer* actPerf = dynamic_cast<ActivityPerformer*>(getRole());
		const Activity& act = dynamic_cast<const Activity&>(*currTripChainItem);
		actPerf->setActivityStartTime(act.startTime);
		actPerf->setActivityEndTime(act.endTime);
		actPerf->setLocation(act.location);
	}

	//Create a return type based on the differences in these Roles
	vector<BufferedBase*> prevParams;
	vector<BufferedBase*> currParams;
	if (prevRole) {
		prevParams = prevRole->getSubscriptionParams();
	}
	if (currRole) {
		currParams = currRole->getSubscriptionParams();
	}
	UpdateStatus res(UpdateStatus::RS_CONTINUE, prevParams, currParams);

	//Set our start time to the NEXT time tick so that frame_init is called
	//  on the first pass through.
	//TODO: Somewhere here the current Role can specify to "put me back on pending", since pending_entities
	//      now takes Agent* objects. (Use "currTimeMS" for this)
	setStartTime(nextValidTimeMS);
	firstFrameTick = true;

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return res;
}



void sim_mob::Person::buildSubscriptionList(vector<BufferedBase*>& subsList) {
	//First, add the x and y co-ordinates
	Agent::buildSubscriptionList(subsList);

	//Now, add our own properties.
	if (this->getRole()) {
		vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();
		for (vector<BufferedBase*>::iterator it = roleParams.begin(); it != roleParams.end(); it++) {
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
		if (this->currWorker) {
			this->currWorker->stopManaging(currRole->getSubscriptionParams());
		}
	}

	currRole = newRole;

	if (currRole) {
		currRole->setParent(this);
		if (this->currWorker) {
			this->currWorker->beginManaging(currRole->getSubscriptionParams());
		}
	}
}

sim_mob::Role* sim_mob::Person::getRole() const {
	return currRole;
}

/*sim_mob::Link* sim_mob::Person::getCurrLink(){
	return currLink;
}
void sim_mob::Person::setCurrLink(sim_mob::Link* link){
	currLink = link;
}*/

#ifndef SIMMOB_DISABLE_MPI
/*
 * package Entity, Agent, Person and Role
 */
//void sim_mob::Person::pack(PackageUtils& packageUtil) {
//	//package Entity
//	//std::cout << "Person package Called" << this->getId() << std::endl;
//	sim_mob::Agent::pack(packageUtil);
//
//	//package person
//	packageUtil.packBasicData(specialStr);
//	sim_mob::TripChain::pack(packageUtil, currTripChain);
//	packageUtil.packBasicData(firstFrameTick);
//}
//
//void sim_mob::Person::unpack(UnPackageUtils& unpackageUtil) {
//
//	sim_mob::Agent::unpack(unpackageUtil);
//	//std::cout << "Person unpackage Called" << this->getId() << std::endl;
//
//	specialStr = unpackageUtil.unpackBasicData<std::string> ();
//	currTripChain = sim_mob::TripChain::unpack(unpackageUtil);
//	firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
//}
//
//void sim_mob::Person::packProxy(PackageUtils& packageUtil) {
//	//package Entity
//	sim_mob::Agent::pack(packageUtil);
//
//	//package person
//	packageUtil.packBasicData(specialStr);
//	packageUtil.packBasicData(firstFrameTick);
//}
//
//void sim_mob::Person::unpackProxy(UnPackageUtils& unpackageUtil) {
//	sim_mob::Agent::unpack(unpackageUtil);
//
//	specialStr = unpackageUtil.unpackBasicData<std::string> ();
//	firstFrameTick = unpackageUtil.unpackBasicData<bool> ();
//}

#endif
