/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusController.hpp"

#include <stdexcept>

#include "entities/Person.hpp"
#include "util/LangHelpers.hpp"

using std::vector;
using std::string;
using std::map;

using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;
vector<BusController*> BusController::all_busctrllers_;// Temporary saved all the buscontroller, eventually it will go to all agent stream


void sim_mob::BusController::RegisterNewBusController(unsigned int startTime, const MutexStrategy& mtxStrat)
{
	BusController* busctrller = new sim_mob::BusController(-1, mtxStrat);
	busctrller->setStartTime(startTime);
	all_busctrllers_.push_back(busctrller);
}

bool sim_mob::BusController::HasBusControllers()
{
	return !all_busctrllers_.empty();
}

void sim_mob::BusController::InitializeAllControllers(vector<Entity*>& agents_list, vector<PT_bus_dispatch_freq>& busdispatch_freq)
{
	//Check: Do we have exactly one BusController?
	if (all_busctrllers_.size()>1) {
		throw std::runtime_error("Currently, we only support zero or one Bus Controller");
	}

	//Initialize every item in the list.
	for (vector<BusController*>::iterator it=all_busctrllers_.begin(); it!=all_busctrllers_.end(); it++) {
		(*it)->setPTScheduleFromConfig(busdispatch_freq);
		(*it)->assignBusTripChainWithPerson(agents_list);
	}
}


void sim_mob::BusController::DispatchAllControllers(vector<Entity*>& agents_list)
{
	//Push every item on the list into the agents array as an active agent
	for (vector<BusController*>::iterator it=all_busctrllers_.begin(); it!=all_busctrllers_.end(); it++) {
		agents_list.push_back(*it);
	}
}


BusController* sim_mob::BusController::TEMP_Get_Bc_1()
{
	if (all_busctrllers_.size()!=1) {
		throw std::runtime_error("BusControllers array is empty.");
	}
	return all_busctrllers_.front();
}



void sim_mob::BusController::buildSubscriptionList(vector<BufferedBase*>& subsList)
{
	Agent::buildSubscriptionList(subsList);
}


UpdateStatus sim_mob::BusController::update(timeslice now)
{
	//NOTE: I am removing the AGENT_UPDATE_PROFILE/STRICT_AGENT_ERRORS check, since it was clearly copied from Person.cpp
	//      We will (after merging) be migrating the "frame_init"/"frame_tick" pattern out of Person and into Agent, with
	//      Entity remaining the "minimal" class. To make this easier, it will help if BusController::update() only contains
	//      what will later go into BusController::frame_tick.
	// ~Seth

//	unsigned int currTimeMS = now.frame*ConfigParams::GetInstance().baseGranMS;

	//Has update() been called early?
	//TODO: This should eventually go into its own helper function in the Agent class.
	//      Aim for the following API:
	//      if (updateBeforeStartTime() { return UpdateStatus::Continue; }
	//      ...and have the parent function take care of throwing the error. ~Seth
	if (now.ms < getStartTime()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			std::stringstream msg;
			msg << "Agent(" << getId() << ") specifies a start time of: "
					<< getStartTime() << " but it is currently: " << now.ms
					<< "; this indicates an error, and should be handled automatically.";
			throw std::runtime_error(msg.str().c_str());
		}
		return UpdateStatus::Continue;
	}

	//TEMPORARY: will exist at the Agent level later.
	if (firstFrameTick) {
		frame_init(now);
		firstFrameTick = false;
	}
	//END TEMPORARY

	//TEMPORARY: This will become our frame_tick method.
	dispatchFrameTick(now);
	//END TEMPORARY

	//TEMPORARY: This will become frame_output
	frame_tick_output(now);
	//END TEMPORARY

	//The variable UpdateStatus::Continue is a convenient way of returning just the simple status.
	return UpdateStatus::Continue;
}


void sim_mob::BusController::addBus(Bus* bus)
{
	managedBuses.push_back(bus);
}

void sim_mob::BusController::remBus(Bus* bus)
{
	vector<Bus*>::iterator it = std::find(managedBuses.begin(), managedBuses.end(), bus);
	if (it!=managedBuses.end()) {
		managedBuses.erase(it);
	}
}

void sim_mob::BusController::assignBusTripChainWithPerson(vector<Entity*>& active_agents)
{
	ConfigParams& config = ConfigParams::GetInstance();
	map<string, Busline*>& buslines = pt_schedule.get_busLines();
	if(0 == buslines.size()) {
		throw std::runtime_error("Error: No busline in the PT_Schedule, please check the setPTSchedule.");
	}

	vector<const TripChainItem*> currAgTripChain;
	for(map<string, Busline*>::const_iterator buslinesIt = buslines.begin();buslinesIt!=buslines.end();buslinesIt++) {
		Busline* busline = buslinesIt->second;
		const vector<BusTrip>& busTrip_vec = busline->queryBusTrips();
		std::cout << "busTrip_vec.size() for busline:" << busline->getBusLineID() << " " << busTrip_vec.size() << std::endl;
		for(int i = 0; i < busTrip_vec.size(); i++) {
			Person* currAg = new Person("DB_TripChain", config.mutexStategy, busTrip_vec[i].personID);
			currAg->setStartTime(busTrip_vec[i].startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime));
			currAgTripChain.clear();

			currAgTripChain.push_back(&busTrip_vec[i]);// one person for one busTrip, currently not considering Activity for BusDriver
			currAg->setTripChain(currAgTripChain);

			// scheduled for dispatch
			addOrStashBuses(currAg, active_agents);
		}
	}
}

void sim_mob::BusController::setPTScheduleFromConfig(vector<PT_bus_dispatch_freq>& busdispatch_freq)
{
	ConfigParams& config = ConfigParams::GetInstance();

	sim_mob::Busline* busline = nullptr;
	int step = 0;

	for (vector<sim_mob::PT_bus_dispatch_freq>::const_iterator curr=busdispatch_freq.begin(); curr!=busdispatch_freq.end(); curr++) {
		vector<sim_mob::PT_bus_dispatch_freq>::const_iterator next = curr+1;

		//If we're on a new BusLine, register it with the scheduler.
		if(!busline || (curr->route_id != busline->getBusLineID())) {
			busline = new sim_mob::Busline(curr->route_id,"no_control");
			pt_schedule.registerBusLine(curr->route_id, busline);
			step = 0; //NOTE: I'm fairly sure this needs to be reset here. ~Seth
		}

		// define frequency_busline for one busline
		busline->addFrequencyBusline(Frequency_Busline(curr->start_time,curr->end_time,curr->headway_sec));

		//Set nextTime to the next frequency bus line's start time or the current line's end time if this is the last line.
		sim_mob::DailyTime nextTime = nextTime = curr->end_time;
		if(next != busdispatch_freq.end()) {
			nextTime = next->start_time;
		}

		//We use a trick to "advance" the time by a given amount; just create a DailyTime with that advance value
		//  and add it during each time step.
		DailyTime advance(curr->headway_sec*50);
		for(DailyTime startTime = curr->start_time; startTime.isBefore(nextTime); startTime += advance) {
			//TODO: I am setting the Vehicle ID to -1 for now; it *definitely* shouldn't be the same as the Agent ID.
			BusTrip bustrip(-1, "BusTrip", 0, startTime, DailyTime("00:00:00"), step++, curr->route_id, -1, curr->route_id, nullptr, "node", nullptr, "node");// 555 for test
			vector<const RoadSegment*>& segments = config.getRoadSegments_Map()[curr->route_id];
			vector<const BusStop*>& stops = config.getBusStops_Map()[curr->route_id];
			if(bustrip.setBusRouteInfo(segments, stops)) {
				busline->addBusTrip(bustrip);
			}
		}
	}
}

void sim_mob::BusController::receiveBusInformation(const string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk) {
	std::cout<<"Report Aijk: --->"<<ATijk<<std::endl;
}

unsigned int sim_mob::BusController::decisionCalculation(const string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk, int lastVisited_BusStopSeqNum)
{
	CONTROL_TYPE controltype = pt_schedule.findBuslineControlType(busline_i);
	unsigned int departure_time = 0; // If we use Control, since the busstopSequence_j is in the middle, so should not be 0
	switch(controltype) {
	case SCHEDULE_BASED:
		departure_time = scheduledDecision(busline_i, trip_k, busstopSequence_j, ATijk);
		break;
	case HEADWAY_BASED:
		departure_time = headwayDecision(busline_i, trip_k, busstopSequence_j, ATijk);
		break;
	case EVENHEADWAY_BASED:
		departure_time = evenheadwayDecision(busline_i, trip_k, busstopSequence_j, ATijk, lastVisited_BusStopSeqNum);
		break;
	case HYBRID_BASED:
		departure_time = hybridDecision(busline_i, trip_k, busstopSequence_j, ATijk);
		break;
	default:
		// may add default scheduled departure time here
		std::cout<<"No Control Decision is used!!!"<<std::endl;
		break;
	}
	return departure_time;
}

unsigned int sim_mob::BusController::scheduledDecision(const string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times

	unsigned int DTijk = 0;
	unsigned int SETijk = 0;
	unsigned int ETijk = 0;
	unsigned int sij = 0;// slack size(should be zero)

	//Fwd_ATijk = ATijk;// assign value
	const vector<BusTrip>& BusTrips = busline->queryBusTrips();

	//StopInformation(Times)
	const vector<BusStop_ScheduledTimes>& busStopScheduledTime_tripK = BusTrips[trip_k].getBusStopScheduledTimes();
	//const vector<const BusStopInfo*>& busStopInfoFwd_tripK = busRouteInfoFwd_tripK->getBusStopsInfo();
	SETijk = busStopScheduledTime_tripK[busstopSequence_j].scheduled_DepartureTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
	ETijk = std::max(SETijk - sij, ATijk + DTijk);

	BusStop_RealTimes busStop_RealTimes(ConfigParams::GetInstance().simStartTime + DailyTime(ATijk), ConfigParams::GetInstance().simStartTime + DailyTime(ETijk));
	busline->resetBusTrip_StopRealTimes(trip_k, busstopSequence_j, busStop_RealTimes);// set this value for next step

	return ETijk;
}

unsigned int sim_mob::BusController::headwayDecision(const string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times

	unsigned int DTijk = 0;
	unsigned int ETijk = 0;
	unsigned int ATijk_1 = 0;
	unsigned int Hi = 0;
	double alpha = 0.6;// range from 0.6 to 0.8

	const vector<BusTrip>& BusTrips = busline->queryBusTrips();
	const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripK_1 = BusTrips[trip_k - 1].getBusStopRealTimes();

	ATijk_1 = busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
	Hi = BusTrips[trip_k].startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
			- BusTrips[trip_k - 1].startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
	ETijk = std::max((unsigned int)(ATijk_1 + alpha*Hi), ATijk + DTijk);

	BusStop_RealTimes busStop_RealTimes(ConfigParams::GetInstance().simStartTime + DailyTime(ATijk), ConfigParams::GetInstance().simStartTime + DailyTime(ETijk));
	busline->resetBusTrip_StopRealTimes(trip_k, busstopSequence_j, busStop_RealTimes);// set this value for next step

	return ETijk;
}

unsigned int sim_mob::BusController::evenheadwayDecision(const string& busline_i, int trip_k, int busstopSequence_j,  unsigned int ATijk, int lastVisited_BusStopSeqNum)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times

	unsigned int DTijk = 0;
	unsigned int ETijk = 0;
	unsigned int ATijk_1 = 0;
	unsigned int ATimk_plus1 = 0;
	unsigned int SRTmj = 0;

	const vector<BusTrip>& BusTrips = busline->queryBusTrips();
	const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripK_1 = BusTrips[trip_k - 1].getBusStopRealTimes();
	ATijk_1 = busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopRealTimes();
	ATimk_plus1 = busStopRealTime_tripKplus1[lastVisited_BusStopSeqNum]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	const vector<BusStop_ScheduledTimes>& busStopScheduledTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopScheduledTimes();
	SRTmj = busStopScheduledTime_tripKplus1[busstopSequence_j].scheduled_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
			- busStopScheduledTime_tripKplus1[lastVisited_BusStopSeqNum].scheduled_DepartureTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
	ETijk = std::max((unsigned int)(ATijk_1 + (double)(ATimk_plus1 + SRTmj - ATijk_1)/2.0), ATijk + DTijk); // need some changes for precision

	BusStop_RealTimes busStop_RealTimes(ConfigParams::GetInstance().simStartTime + DailyTime(ATijk), ConfigParams::GetInstance().simStartTime + DailyTime(ETijk));
	busline->resetBusTrip_StopRealTimes(trip_k, busstopSequence_j, busStop_RealTimes);// set this value for next step
	return ETijk;
}

unsigned int sim_mob::BusController::hybridDecision(const string& busline_i, int trip_k, int busstopSequence_j, unsigned int ATijk)
{
	unsigned int DTijk = 0;
	return DTijk;
}

unsigned int sim_mob::BusController::dwellTimeCalculation(const string& busline_i, int trip_k, int busstopSequence_j)
{
	double alpha1 = 0.0;
	double alpha2 = 0.0;
	double alpha3 = 0.0;
	double alpha4 = 0.0;

	double beta1 = 0.0;
	double beta2 = 0.0;
	double beta3 = 0.0;
	int Pfront = 1;

	double PTijk_front = 0.0;
	double PTijk_rear = 0.0;
	unsigned int DTijk = 0;
	return DTijk;
}

unsigned int sim_mob::BusController::sendBusInformation()
{

}

void sim_mob::BusController::addOrStashBuses(Agent* p, vector<Entity*>& active_agents)
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled() || p->getStartTime()==0) {
		//Only agents with a start time of zero should start immediately in the all_agents list.
		p->load(p->getConfigProperties());
		p->clearConfigProperties();
		active_agents.push_back(p);
	} else {
		//Start later.
		pending_buses.push(p);
	}
}


void sim_mob::BusController::dispatchFrameTick(timeslice now)
{
	//Note: The WorkGroup (see below) will delay an entity until its time tick has arrived, so there's nothing wrong
	//      with dispatching early. To reflect this, I've added +3 to the next time tick. Ideally, the BusController
	//      would stage the Bus as soon as it was 100% sure that this bus would run. (We can add functionality later for
	//      updating a pending request). In other words, let the WorkGroup do what it does best. ~Seth
	nextTimeTickToStage += tickStep;
	unsigned int nextTickMS = (nextTimeTickToStage+3)*ConfigParams::GetInstance().baseGranMS;

	//Stage any pending entities that will start during this time tick.
	while (!pending_buses.empty() && pending_buses.top()->getStartTime() <= nextTickMS) {
		///////////////////////////////////////////////////////////////////
		//Ask the current worker's parent WorkGroup to schedule this Entity.
		///////////////////////////////////////////////////////////////////
		//
		// TODO: The use of "getParent()" in Worker is extremely dangerous.
		//       It only works because there is exactly 1 BusController, and that
		//       Controller is the only thing which accesses the "parent" class.
		//       To fix this, we should probably return a list of entities to schedule
		//       as a result of update(), and let the WorkGroup skim this list
		//       when it knows it's safe to. ~Seth
		//
		///////////////////////////////////////////////////////////////////
		currWorker->getParent()->scheduleEntity(pending_buses.top());
		pending_buses.pop();
	}
}



void sim_mob::BusController::frame_init(timeslice now)
{
	frameNumberCheck = 0;
}

void sim_mob::BusController::frame_tick_output(timeslice now)
{
	//if no buscontroller in the loadorder, no output
	if (!getToBeInList()) {
		return;
	}

#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"BusController\""
			<<","<<now.frame
			<<","<<getId()
			<<",{"
			<<"\"managedBuses size\":\""<<static_cast<int>(managedBuses.size())
			<<"\",\"Bus_xPos\":\""<<static_cast<int>(posBus.x)
			<<"\",\"Bus_yPos\":\""<<static_cast<int>(posBus.y)
			<<"\"})"<<std::endl);
#endif
}




////////////////////////////////////////////////////////////////////////////////////////
// Trivial getters and setters
////////////////////////////////////////////////////////////////////////////////////////

sim_mob::Link* sim_mob::BusController::getCurrLink(){
	return currLink;
}
void sim_mob::BusController::setCurrLink(sim_mob::Link* link) {
	currLink = link;
}

