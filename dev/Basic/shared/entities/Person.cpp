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


sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, unsigned int id) : Agent(mtxStrat, id),
	prevRole(nullptr), currRole(nullptr), agentSrc(src), currTripChainSequenceNumber(0), /*currTripChainItem(nullptr),
	currSubTrip(nullptr),*/ call_frame_init(true)

{
	tripchainInitialized = false;
	//throw 1;
}

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, std::vector<sim_mob::TripChainItem*>  tcs):Agent(mtxStrat, tcs.front()->personID)
{
	prevRole = 0;
	currRole = 0;
	agentSrc = src;
	call_frame_init = true;
	tripChain = tcs;
//	currTripChainItem(nullptr);
//	currSubTrip(nullptr);
	tripchainInitialized = false;
	initTripChain();

}

void sim_mob::Person::initTripChain(){
	currTripChainItem = tripChain.begin();
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTrips()).begin();
		//consider putting this in IT_TRIP clause
		if(!updateOD(*currTripChainItem)){ //Offer some protection
				throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
		}
	}

	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_BUSTRIP)
	{
		std::cout << "Pesron " << this << "  is going to ride a bus\n";
	}
	setNextPathPlanned(false);
	first_update_tick = true;
	tripchainInitialized = true;
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

		//std::cout << "sim_mob::Person::load=>input[" << origIt->second << " , " << destIt->second << "]\n";
		this->originNode = ConfigParams::GetInstance().getNetwork().locateNode(parse_point(origIt->second), true);
		this->destNode = ConfigParams::GetInstance().getNetwork().locateNode(parse_point(destIt->second), true);
		//std::cout << "Resulting nodes[" << this->originNode << " , " << this->destNode << "]\n";

		//Make sure they have a mode specified for this trip
		it = configProps.find("#mode");
		if (it==configProps.end()) {
			throw std::runtime_error("Cannot load person: no mode");
		}
		std::string mode = it->second;

		Trip* singleTrip = MakePseudoTrip(*this, mode);

		std::vector<TripChainItem*> trip_chain;
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
			//vector<WayPoint> path = sd.SearchShortestWalkingPath(agent->originNode->location, agent->destNode->location);
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



//SubTrip* sim_mob::Person::getNextSubTripInTrip(){
//	if((!(*currTripChainItem)) || ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY)){
//		return *currSubTrip;
//	}
//
//	if((*(this->currTripChainItem))->itemType != sim_mob::TripChainItem::IT_TRIP){
////		throw std::runtime_error("Invalid trip chain item type!");
//		return nullptr;//extra
//	}
//
//	Trip* currTrip = dynamic_cast<Trip*>(*(this->currTripChainItem));
//	vector<SubTrip>& currSubTripsList = currTrip->getSubTripsRW();
//	if (!(*currSubTrip)) {
//		//Return the first sub trip if the current sub trip which is passed in is null
//		*currSubTrip = &currSubTripsList.front();
//		return *currSubTrip;
//	}
//		// else return the next sub trip if available; return nullptr otherwise.
//	vector<SubTrip>::iterator subTripIterator = std::find(currSubTripsList.begin(), currSubTripsList.end(), (**currSubTrip));
//	*currSubTrip = nullptr;
//	if (subTripIterator != currSubTripsList.end()) {
//		//Set it equal to the next item, assuming we are not at the end of the list.
//		if (++subTripIterator != currSubTripsList.end()) {
//			*currSubTrip = &(*subTripIterator);
//		}
//	}
//}
//
//TripChainItem* sim_mob::Person::findNextItemInTripChain() {
//	if((!*(this->currTripChainItem)) /*|| (std::find(tripChain.begin(), tripChain.end(), *(this->currTripChainItem)) == this->tripChain.end())*/) {
//		if(tripChain.begin() == tripChain.end())
//		std::cout << "tripChain empty and taking currTripChainItem from front\n";
//		else
//		{
//			std::cout << "tripChain.size() = " << tripChain.size() << std::endl;
//		}
//		*(this->currTripChainItem) = this->tripChain.front();
//	}
//	else {
//		// else set the next item if available; return nullptr otherwise.
//		std::vector<TripChainItem*>::iterator itemIterator = std::find(tripChain.begin(), tripChain.end(), (*currTripChainItem));
//		(*currTripChainItem) = nullptr;
//		if (itemIterator!=tripChain.end()) {
//			//Set it equal to the next item, assuming we are not at the end of the list.
//			if (++itemIterator != tripChain.end()) {
//				*(this->currTripChainItem) = *itemIterator;
//			}
//		}
//	}
//
//	//comment and call explicitly/separately to get be more clear
////	this->getNextSubTripInTrip(); //if currTripChainItem is Trip, set currSubTrip as well
//	return *(this->currTripChainItem);
//}


void sim_mob::Person::update_time(timeslice now, UpdateStatus& retVal)
{
	
	//Agents may be created with a null Role and a valid trip chain
	if (call_frame_init && !currRole) {

		UpdateStatus res =	checkTripChain(now.ms());
		
		//Reset the start time (to the current time tick) so our dispatcher doesn't complain.
		setStartTime(now.ms());

	}
	 
	//Failsafe
	if (!currRole) {
		std::ostringstream txt ;
		txt << "Person " << this->getId() <<  " has no Role.";
		throw std::runtime_error(txt.str());
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
	if (call_frame_init) {
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
		std::cout << "Calling update_time.frame_init at frame " << now.frame() << std::endl;
		currRole->frame_init(params);

		//Done
		call_frame_init = false;
	}
	 
	//Now perform the main update tick
	if (!isToBeRemoved()) {
		//added to get the detailed plan before next activity
		currRole->frame_tick(params);
	}
	 
	//Finally, save the output
	if (!isToBeRemoved()) {
		currRole->frame_tick_output(params);
	}
	 
	//If we're "done", try checking to see if we have any more items in our Trip Chain.
	// This is not strictly the right way to do things (we shouldn't use "isToBeRemoved()"
	// in this manner), but it's the easiest solution that uses the current API.
	if (isToBeRemoved()) {
		retVal = checkTripChain(now.ms());

		//Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
		setStartTime(now.ms()+ConfigParams::GetInstance().baseGranMS);
		//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
		//since start time of the activity is usually later than what is configured initially,
		//we have to make adjustments so that it waits for exact amount of time
		if(currTripChainItem != tripChain.end())
		if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
		{
			sim_mob::ActivityPerformer *ap = dynamic_cast<sim_mob::ActivityPerformer *>(currRole);
			std::cout << "intial Activity start time = " << (*currTripChainItem)->startTime.toString() << " to " << (*currTripChainItem)->endTime.toString() << std::endl;
			std::cout << "intial Activity performer start time = " << ap->getActivityStartTime().toString() << " to " << ap->getActivityEndTime().toString() << std::endl;
			ap->setActivityStartTime(sim_mob::DailyTime((*currTripChainItem)->startTime.getValue() + now.ms() + ConfigParams::GetInstance().baseGranMS));
			ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigParams::GetInstance().baseGranMS + (*currTripChainItem)->endTime.getValue()));
			std::cout << "later Activity performer start time = " << ap->getActivityStartTime().toString() << " to " << ap->getActivityEndTime().toString() << std::endl;
			ap->initializeRemainingTime();
			std::cout << "Activity remaining time initialized to " << ap->remainingTimeToComplete << std::endl;
//			getchar();
		}
	}
	 
	//Output if removal requested.
	if (Debug::WorkGroupSemantics && isToBeRemoved()) {
		LogOut("Person requested removal: " <<"(Role Hidden)" <<std::endl);
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
//todo uncomment try-catch
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
		if (ConfigParams::GetInstance().OutputEnabled()) {
			std::stringstream msg;
			msg <<"Error updating Agent[" <<getId() <<"], will be removed from the simulation.";
			msg <<"\n  From node: " <<(originNode?originNode->originalDB_ID.getLogItem():"<Unknown>");
			msg <<"\n  To node: " <<(destNode?destNode->originalDB_ID.getLogItem():"<Unknown>");
			msg <<"\n  " <<ex.what();
			LogOut(msg.str() <<std::endl);
		}
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

bool sim_mob::Person::updateOD(sim_mob::TripChainItem * tc, const sim_mob::SubTrip *subtrip)
{
	return tc->setPersonOD(this, subtrip);
}
bool sim_mob::Person::changeRoleRequired(sim_mob::Role & currRole,sim_mob::SubTrip &currSubTrip) const
{
	string roleName = RoleFactory::GetSubTripMode(currSubTrip);
	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	const sim_mob::Role* targetRole = rf.getPrototype(roleName);
	if(targetRole->getRoleName() ==  currRole.getRoleName()) return false;
	//the current role type and target(next) role type are not same. so we need to change the role!
	return true;
}

bool sim_mob::Person::changeRoleRequired(sim_mob::TripChainItem &tripChinItem) const
{
	if(tripChinItem.itemType == sim_mob::TripChainItem::IT_TRIP)
		return changeRoleRequired_Trip();
		return changeRoleRequired_Activity();
}
bool sim_mob::Person::changeRoleRequired_Trip(/*sim_mob::Trip &trip*/) const
{
	std::cout << "Checking if the change is required from currRole[" << currRole << "]: "<< currRole->getRoleName() << std::endl;
	string roleName = RoleFactory::GetSubTripMode(*currSubTrip);
	std::cout << "Person::changeRoleRequired_Trip->roleName = " << roleName << std::endl;
	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	const sim_mob::Role* targetRole = rf.getPrototype(roleName);
	std::cout << " and targetRole->getRoleName() will be " << targetRole->getRoleName() << " vs curr:" << currRole->getRoleName()<< std::endl;
	if(targetRole->getRoleName() ==  currRole->getRoleName())
		return false;
	//the current role type and target(next) role type are not same. so we need to change the role!
	return true;
}

bool sim_mob::Person::changeRoleRequired_Activity(/*sim_mob::Activity &activity*/) const
{
	return true;
}

bool sim_mob::Person::updatePersonRole()
{
	if(!((!currRole) ||(changeRoleRequired(*(*(this->currTripChainItem)))))) return false;
		//Prepare to delete the previous Role. We _could_ delete it now somewhat safely, but
		// it's better to avoid possible errors (e.g., if the equality operator is defined)
		// by saving it until the next time tick.
		safe_delete_item(prevRole);
		const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
		prevRole = currRole;
		const sim_mob::SubTrip *temp = ((*(this->currTripChainItem))->itemType == sim_mob::TripChainItem::IT_TRIP ? &(*currSubTrip) : 0);
		sim_mob::Role* newRole = rf.createRole(*(this->currTripChainItem),temp, this);
		changeRole(newRole);
		std::cout << "role changed to " << rf.GetTripChainItemMode((*currTripChainItem),temp) << std::endl;
//		getchar();
}

UpdateStatus sim_mob::Person::checkTripChain(uint32_t currTimeMS) {
	std::cout << "checking the tripchain for person " << this ;
	std::cout << "[currTripChainItem: " << (*currTripChainItem) ;

	//some normal checks
	if(tripChain.size() < 1)
		return UpdateStatus::Done;
	//advance the trip,subtrip or activity....
	if(!first_update_tick)
		if(!(advanceCurrentTripChainItem()))
			return UpdateStatus::Done;

	int i = std::distance(currTripChainItem,tripChain.begin());
	std::cout << "\ndistance = " << i+1 << "/" << tripChain.size() << std::endl;
	first_update_tick = false;
	//must be set to false whenever tripchainitem changes. And it has to happen before a probable creation of(or changing to) a new role
	setNextPathPlanned(false);
	//Create a new Role based on the trip chain type
	if(currRole)
		std::cout << "calling updatePersonRole to Check if we can change the role from " << currRole->getRoleName();
	updatePersonRole();
	//currentTipchainItem or current subtrip are changed
	//so OD will be changed too,
//	therefore we need to call frame_init regardless of change in the role
	call_frame_init = true;

	//Update our origin/dest pair.
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)//put if to avoid & evade bustrips, can be removed when everything is ok
		updateOD(*currTripChainItem, &(*currSubTrip));

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
	//setStartTime(nextValidTimeMS); done out side this function
//	call_frame_init = true;//what a hack! -vahid

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return res;
}

//sets the current subtrip to the first subtrip of the provided trip(provided trip is usually the current tripChianItem)
std::vector<sim_mob::SubTrip>::const_iterator sim_mob::Person::resetCurrSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
		if(!trip) throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
	return trip->getSubTrips().begin();
}

////advance to the next subtrip inside the current TripChainItem assuming that the current TripChainItem is a Trip
////the function also resets the currSubtrip if necessary
//bool sim_mob::Person::advanceCurrentTripChainItem_Trip()
//{
//	if(currTripChainItem == tripChain.end()) return false;
//
//	currTripChainItem ++;
//
//	if(currTripChainItem == tripChain.end()) return false;
//
//	//everything seems ok, so set the currSubTrip also
//	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
//		currSubTrip = resetCurrSubTrip(*(*currTripChainItem));
//
//	return true;
//}

//advance to the next subtrip inside the current TripChainItem
bool sim_mob::Person::advanceCurrentSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
	if(!trip) return false;
	if (currSubTrip == trip->getSubTrips().end())//just a routine check
		return false;

	currSubTrip++;

	if (currSubTrip == trip->getSubTrips().end())
		return false;

	return true;
}

////advance to the next subtrip inside the current TripChainItem assuming that the current TripChainItem is an activity
//bool sim_mob::Person::advanceCurrentTripChainItem_Activity()
//{
//
//}
bool sim_mob::Person::advanceCurrentTripChainItem()
{

	bool res = false;
	if(currTripChainItem == tripChain.end()) return false; //just a harmless basic check
//	std::cout << "Advancing the tripchain for person " << (*currTripChainItem)->personID << std::endl;
	//first check if you just need to advance the subtrip
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//dont advance to next tripchainItem immidiately, check the subtrip first
		res = advanceCurrentSubTrip();
	}

	if(res) return res;

	//no, it is not the subtrip we need to advance, it is the tripchain item
	currTripChainItem ++;
	if(currTripChainItem != tripChain.end())
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY)
		std::cout << "processing Activity";
	//but tripchainitems are also over, get out !
	if(currTripChainItem == tripChain.end()) return false;

	//so far, advancing the tripchainitem has been successful

	//Also set the currSubTrip to the beginning of trip , just in case
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
		currSubTrip = resetCurrSubTrip();



	return true;
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
