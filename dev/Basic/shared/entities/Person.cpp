/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"

#include <algorithm>
#include <sstream>

//For debugging
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "util/OutputUtil.hpp"

#include "geospatial/Node.hpp"
#include "entities/misc/TripChain.hpp"
#include "workers/Worker.hpp"
#include "geospatial/aimsun/Loader.hpp"

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
Trip* MakePseudoTrip(const Person& ag, const std::string& mode)
{
	//Make sure we have something to work with
	if (!(ag.originNode .node_&& ag.destNode.node_)) {
		std::stringstream msg;
		msg <<"Can't make a pseudo-trip for an Agent with no origin and destination nodes: " <<ag.originNode.node_ <<" , " <<ag.destNode.node_;
		throw std::runtime_error(msg.str().c_str());
	}

	//Make the trip itself
	Trip* res = new Trip();
	res->personID = ag.getId();
	res->itemType = TripChainItem::getItemType("Trip");
	res->sequenceNumber = 1;
	res->startTime = DailyTime(ag.getStartTime());  //TODO: This may not be 100% correct
	res->endTime = res->startTime; //No estimated end time.
	res->tripID = "";
	res->fromLocation = WayPoint(ag.originNode);
	res->fromLocationType = TripChainItem::getLocationType("node");
	res->toLocation = WayPoint(ag.destNode);
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
	prevRole(nullptr), currRole(nullptr), agentSrc(src), currTripChainSequenceNumber(0), curr_params(nullptr),remainingTimeThisTick(0.0),
	requestedNextSegment(nullptr), canMoveToNextSegment(false), databaseID(databaseID), debugMsgs(std::stringstream::out)
{
	tripchainInitialized = false;
	laneID = -1;
}

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, std::vector<sim_mob::TripChainItem*>  tcs)
	: Agent(mtxStrat), remainingTimeThisTick(0.0), requestedNextSegment(nullptr), canMoveToNextSegment(false), databaseID(tcs.front()->personID), debugMsgs(std::stringstream::out)
{
	prevRole = 0;
	currRole = 0;
	laneID = -1;
	agentSrc = src;
	tripChain = tcs;
	tripchainInitialized = false;
#ifndef SIMMOB_USE_CONFLUXES
	simplyModifyTripChain(tcs);
#endif
	initTripChain();
}

void sim_mob::Person::initTripChain(){
	currTripChainItem = tripChain.begin();
	setStartTime((*currTripChainItem)->startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime));
	unsigned int start = getStartTime();
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<sim_mob::Trip*>(*currTripChainItem))->getSubTripsRW()).begin();
		//consider putting this in IT_TRIP clause
		if(!updateOD(*currTripChainItem)){ //Offer some protection
				throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
		}
	}

	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_BUSTRIP) {
		std::cout << "Person " << this << "  is going to ride a bus\n";
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


	std::map<std::string, std::string>::const_iterator lanepointer = configProps.find("lane");
	if(lanepointer != configProps.end())
	{
		try {
		    int x = boost::lexical_cast<int>( lanepointer->second );
		    laneID = x;
		} catch( boost::bad_lexical_cast const& ) {
		    std::cout << "Error: input string was not valid" << std::endl;
		}
	}


	//Consistency check: are they requesting a pseudo-trip chain when they actually have one?
	map<string, string>::const_iterator origIt = configProps.find("originPos");
	map<string, string>::const_iterator destIt = configProps.find("destPos");
	if (origIt!=configProps.end() && destIt!=configProps.end()) {
		//Double-check some potential error states.
		if (!tripChain.empty()) {
			throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
		}
		if (this->originNode .node_|| this->destNode.node_) {
			throw std::runtime_error("Manual position specified for Agent with existing Trip Chain.");
		}

		//Otherwise, make a trip chain for this Person.
		this->originNode = WayPoint( ConfigParams::GetInstance().getNetwork().locateNode(parse_point(origIt->second), true) );
		this->destNode = WayPoint( ConfigParams::GetInstance().getNetwork().locateNode(parse_point(destIt->second), true) );

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
		this->initTripChain();
	}

	//One more check: If they have a special string, save it now
	it = configProps.find("special");
	if (it != configProps.end()) {
		this->specialStr = it->second;
	}
}


bool sim_mob::Person::frame_init(timeslice now)
{
	//Agents may be created with a null Role and a valid trip chain
	if (!currRole) {
		//TODO: This UpdateStatus has a "prevParams" and "currParams" that should
		//      (one would expect) be dealt with. Where does this happen?
		UpdateStatus res =	checkTripChain(now.ms());

		//Reset the start time (to the current time tick) so our dispatcher doesn't complain.
		setStartTime(now.ms());

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
	curr_params = &currRole->make_frame_tick_params(now);

	//Now that the Role has been fully constructed, initialize it.
	if((*currTripChainItem)) {
		currRole->Behavior()->frame_init(*curr_params);
	}

	return true;
}


Entity::UpdateStatus sim_mob::Person::frame_tick(timeslice now)
{
	//TODO: Here is where it gets risky.
	if (!curr_params) {
		curr_params = &currRole->make_frame_tick_params(now);
	}

	Entity::UpdateStatus retVal(UpdateStatus::RS_CONTINUE);

	if (!isToBeRemoved()) {
		currRole->Behavior()->frame_tick(*curr_params);
	}

	//If we're "done", try checking to see if we have any more items in our Trip Chain.
	// This is not strictly the right way to do things (we shouldn't use "isToBeRemoved()"
	// in this manner), but it's the easiest solution that uses the current API.
	//TODO: This section should technically go after frame_output(), but doing that
	//      (by overriding Person::update() and calling Agent::update() then inserting this code)
	//      will bring us outside the bounds of our try {} catch {} statement. We might move this
	//      statement into the worker class, but I don't want to change too many things
	//      about Agent/Person at once. ~Seth
	if (isToBeRemoved()) {
		retVal = checkTripChain(now.ms());

		//Reset the start time (to the NEXT time tick) so our dispatcher doesn't complain.
		setStartTime(now.ms()+ConfigParams::GetInstance().baseGranMS);

		//IT_ACTIVITY as of now is just a matter of waiting for a period of time(between its start and end time)
		//since start time of the activity is usually later than what is configured initially,
		//we have to make adjustments so that it waits for exact amount of time
		if(currTripChainItem != tripChain.end()) {
			if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
				sim_mob::ActivityPerformer *ap = dynamic_cast<sim_mob::ActivityPerformer*>(currRole);
				ap->setActivityStartTime(sim_mob::DailyTime((*currTripChainItem)->startTime.getValue() + now.ms() + ConfigParams::GetInstance().baseGranMS));
				ap->setActivityEndTime(sim_mob::DailyTime(now.ms() + ConfigParams::GetInstance().baseGranMS + (*currTripChainItem)->endTime.getValue()));
				ap->initializeRemainingTime();
			}
		}
	}

	return retVal;
}



void sim_mob::Person::frame_output(timeslice now)
{
	//Save the output
	if (!isToBeRemoved()) {
		currRole->Behavior()->frame_tick_output(*curr_params);
	}

	//TODO: Still risky.
	curr_params = nullptr; //WARNING: Do *not* delete curr_params; it is only used to point to the result of get_params.
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
	if(targetRole->getRoleName() ==  currRole.getRoleName()) {
		return false;
	}
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
	//std::cout << "Checking if the change is required from currRole[" << currRole << "]: "<< currRole->getRoleName() << std::endl;
	string roleName = RoleFactory::GetSubTripMode(*currSubTrip);
	//std::cout << "Person::changeRoleRequired_Trip->roleName = " << roleName << std::endl;
	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	const sim_mob::Role* targetRole = rf.getPrototype(roleName);
	//std::cout << " and targetRole->getRoleName() will be " << targetRole->getRoleName() << " vs curr:" << currRole->getRoleName()<< std::endl;
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

		const sim_mob::TripChainItem* tci = *(this->currTripChainItem);
		const sim_mob::SubTrip* str = (tci->itemType == sim_mob::TripChainItem::IT_TRIP ? &(*currSubTrip) : 0);

		sim_mob::Role* newRole = rf.createRole(tci, str, this);
		changeRole(newRole);
}

UpdateStatus sim_mob::Person::checkTripChain(uint32_t currTimeMS) {
	//some normal checks
	if(tripChain.size() < 1) {
		return UpdateStatus::Done;
	}

	//advance the trip,subtrip or activity....
	if(!first_update_tick) {
		if(!(advanceCurrentTripChainItem())) {
			return UpdateStatus::Done;
		}
	}

	first_update_tick = false;

	//must be set to false whenever tripchainitem changes. And it has to happen before a probable creation of(or changing to) a new role
	setNextPathPlanned(false);

	//Create a new Role based on the trip chain type
	updatePersonRole();

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

	//Set our start time to the NEXT time tick so that frame_init is called
	//  on the first pass through.
	//TODO: Somewhere here the current Role can specify to "put me back on pending", since pending_entities
	//      now takes Agent* objects. (Use "currTimeMS" for this)

	//setStartTime(nextValidTimeMS); done out side this function
	//call_frame_init = true;//what a hack! -vahid

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return UpdateStatus(UpdateStatus::RS_CONTINUE, prevParams, currParams);

}

//sets the current subtrip to the first subtrip of the provided trip(provided trip is usually the current tripChianItem)
std::vector<sim_mob::SubTrip>::iterator sim_mob::Person::resetCurrSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
		if(!trip) throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
	return trip->getSubTripsRW().begin();
}

void sim_mob::Person::simplyModifyTripChain(std::vector<TripChainItem*>& tripChain)
{
	std::vector<TripChainItem*>::iterator tripChainItem;
	for(tripChainItem = tripChain.begin(); tripChainItem != tripChain.end(); tripChainItem++ )
	{
		if((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP )
		{
			std::vector<SubTrip>::iterator subChainItem1, subChainItem2;
			std::vector<sim_mob::SubTrip>& subtrip = (dynamic_cast<sim_mob::Trip*>(*tripChainItem))->getSubTripsRW();
			for(subChainItem1 = subtrip.begin();subChainItem1!=subtrip.end(); subChainItem1++)
			{
				std::cout << "first item  " << subChainItem1->fromLocation.getID() << " " <<subChainItem1->toLocation.getID() <<" mode " <<subChainItem1->mode << std::endl;
			}
			subChainItem2 = subChainItem1 = subtrip.begin();
			subChainItem2++;
			vector<SubTrip> newsubchain;
			newsubchain.push_back(*subChainItem1);
			while(subChainItem1!=subtrip.end() && subChainItem2!=subtrip.end() )
			{
				std::cout << "first item  " << subChainItem1->fromLocation.getID() << " " <<subChainItem1->toLocation.getID() << std::endl;
				std::cout << "second item  " << subChainItem2->fromLocation.getID() << " " <<subChainItem2->toLocation.getID() << std::endl;

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
					if(finder.getSourceBusStop())
					{
						source = WayPoint(finder.getDestinationBusStop());
						destination = subChainItem2->fromLocation;
					}
				}
				if(source.type_!=WayPoint::INVALID && destination.type_!=WayPoint::INVALID )
				{
					sim_mob::SubTrip subTrip;
					subTrip.personID = -1;
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

					//subtrip.insert(subChainItem2, subTrip);

					//if(destination.type_==WayPoint::BUS_STOP )
					//	subChainItem2++;
					//else if(source.type_==WayPoint::BUS_STOP)
					//	subChainItem2->fromLocation = source;

					if(destination.type_==WayPoint::BUS_STOP )
						subChainItem1->toLocation = destination;
					else if(source.type_==WayPoint::BUS_STOP)
						subChainItem2->fromLocation = source;

					newsubchain.push_back(subTrip);
				}

				newsubchain.push_back(*subChainItem2);
				subChainItem1 = subChainItem2;
				subChainItem2++;

				if(subChainItem1==subtrip.end() || subChainItem2==subtrip.end())
					break;
			}

			if(newsubchain.size()>2)
			{
				std::vector<SubTrip>::iterator subChainItem;
				/*subtrip.clear();
				for(subChainItem = newsubchain.begin();subChainItem!=newsubchain.end(); subChainItem++)
				{
					subtrip.push_back(*subChainItem);
				}*/

				for(subChainItem = subtrip.begin();subChainItem!=subtrip.end(); subChainItem++)
				{
					std::cout << "first item  " << subChainItem->fromLocation.getID() << " " <<subChainItem->toLocation.getID() <<" mode " <<subChainItem->mode << std::endl;
				}
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
			std::vector<SubTrip>::iterator itfinder2 = currSubTrip++;
			itfinder2 = std::find(currSubTrip, subtrip.end(), *before2);
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
		for(itfinder++; itfinder!=tripChain.end(); itfinder++)
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

//advance to the next subtrip inside the current TripChainItem
bool sim_mob::Person::advanceCurrentSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *>(*currTripChainItem);
	if(!trip) return false;
	if (currSubTrip == trip->getSubTripsRW().end())//just a routine check
		return false;

	currSubTrip++;

	if (currSubTrip == trip->getSubTripsRW().end())
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
	std::cout << "Advancing the tripchain for person " << (*currTripChainItem)->personID << std::endl;
	//first check if you just need to advance the subtrip
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//dont advance to next tripchainItem immidiately, check the subtrip first
		res = advanceCurrentSubTrip();
		if(res)
		std::cout << "Advancing the subtripchain for person mode: " << currSubTrip->mode << " from Node  "<<currSubTrip->fromLocation.getID()<< std::endl;

	}

	if(res) return res;

	//no, it is not the subtrip we need to advance, it is the tripchain item
	currTripChainItem ++;
	if(currTripChainItem != tripChain.end())
	if((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_ACTIVITY) {
	//	std::cout << "processing Activity";
	}
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
			this->currWorker->stopManaging(currRole->getDriverRequestParams().asVector());
		}
	}

	currRole = newRole;

	if (currRole) {
		currRole->setParent(this);
		if (this->currWorker) {
			this->currWorker->beginManaging(currRole->getSubscriptionParams());
			this->currWorker->beginManaging(currRole->getDriverRequestParams().asVector());
		}
	}
}

sim_mob::Role* sim_mob::Person::getRole() const {
	return currRole;
}
