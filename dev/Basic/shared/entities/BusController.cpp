/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusController.hpp"

#include <stdexcept>

#include "entities/Person.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "util/LangHelpers.hpp"

using std::vector;
using std::string;
using std::map;

using namespace sim_mob;

typedef Entity::UpdateStatus UpdateStatus;
vector<BusController*> BusController::all_busctrllers_;// Temporary saved all the buscontroller, eventually it will go to all agent stream
static int default_headway = 170000;//85000; // 143000(best), 142000, 140000(headway*100), 138000, 181000(bad effect) ;60000(headway*50)
bool BusController::busBreak = false;
const char* BusController::buslineID = NULL;
int BusController::busstopindex = 1;

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

void sim_mob::BusController::InitializeAllControllers(vector<Entity*>& agents_list, const vector<PT_bus_dispatch_freq>& busdispatch_freq)
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

//	unsigned int currTimeMS = now.frame()*ConfigParams::GetInstance().baseGranMS;

	//Has update() been called early?
	//TODO: This should eventually go into its own helper function in the Agent class.
	//      Aim for the following API:
	//      if (updateBeforeStartTime() { return UpdateStatus::Continue; }
	//      ...and have the parent function take care of throwing the error. ~Seth
	if (now.ms() < getStartTime()) {
		//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
		if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			std::stringstream msg;
			msg << "Agent(" << getId() << ") specifies a start time of: "
					<< getStartTime() << " but it is currently: " << now.ms()
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
	const map<string, Busline*>& buslines = pt_schedule.get_busLines();
	if(0 == buslines.size()) {
		throw std::runtime_error("Error: No busline in the PT_Schedule, please check the setPTSchedule.");
	}

	for(map<string, Busline*>::const_iterator buslinesIt = buslines.begin();buslinesIt!=buslines.end();buslinesIt++) {
		Busline* busline = buslinesIt->second;
		const vector<BusTrip>& busTrip_vec = busline->queryBusTrips();
		for (vector<BusTrip>::const_iterator tripIt=busTrip_vec.begin(); tripIt!=busTrip_vec.end(); tripIt++) {
			if(tripIt->startTime.isAfterEqual(ConfigParams::GetInstance().simStartTime)) {// in case sometimes BusTrip startTime is smaller than simStartTime to skip some BusTrips
				Person* currAg = new Person("BusController", config.mutexStategy, tripIt->personID);
				currAg->setStartTime(tripIt->startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime));

				vector<TripChainItem*> currAgTripChain;
				currAgTripChain.push_back(const_cast<BusTrip*>(&(*tripIt)));// one person for one busTrip, currently not considering Activity for BusDriver
				currAg->setTripChain(currAgTripChain);
				currAg->initTripChain();

				// scheduled for dispatch
				addOrStashBuses(currAg, active_agents);
			}
		}
	}
}

void sim_mob::BusController::setPTScheduleFromConfig(const vector<PT_bus_dispatch_freq>& busdispatch_freq)
{
	ConfigParams& config = ConfigParams::GetInstance();

	sim_mob::Busline* busline = nullptr;
	int step = 0;

	for (vector<sim_mob::PT_bus_dispatch_freq>::const_iterator curr=busdispatch_freq.begin(); curr!=busdispatch_freq.end(); curr++) {
		vector<sim_mob::PT_bus_dispatch_freq>::const_iterator next = curr+1;

		//If we're on a new BusLine, register it with the scheduler.
		if(!busline || (curr->route_id != busline->getBusLineID())) {
			busline = new sim_mob::Busline(curr->route_id,config.busline_control_type);

			//From Santhosh's branch; re-enable if needed.
			//busline = new sim_mob::Busline(curr->route_id,"headway_based");

			pt_schedule.registerBusLine(curr->route_id, busline);
			pt_schedule.registerControlType(curr->route_id, busline->getControlType());
			step = 0; //NOTE: I'm fairly sure this needs to be reset here. ~Seth
		}

		// define frequency_busline for one busline
		busline->addFrequencyBusline(Frequency_Busline(curr->start_time,curr->end_time,curr->headway_sec));

		//Set nextTime to the next frequency bus line's start time or the current line's end time if this is the last line.
		sim_mob::DailyTime nextTime = curr->end_time;

		//We use a trick to "advance" the time by a given amount; just create a DailyTime with that advance value
		//  and add it during each time step.
		DailyTime advance(curr->headway_sec*200);
		for(DailyTime startTime = curr->start_time; startTime.isBeforeEqual(nextTime); startTime += advance) {
			//TODO: I am setting the Vehicle ID to -1 for now; it *definitely* shouldn't be the same as the Agent ID.
			BusTrip bustrip(-1, "BusTrip", 0, startTime, DailyTime("00:00:00"), step++, busline, -1, curr->route_id, nullptr, "node", nullptr, "node");

			//Try to find our data.
			map<string, vector<const RoadSegment*> >::const_iterator segmentsIt = config.getRoadSegments_Map().find(curr->route_id);
			map<string, vector<const BusStop*> >::const_iterator stopsIt = config.getBusStops_Map().find(curr->route_id);

			//Our algorithm expects empty vectors in some cases.
			//TODO: Clean this up! Logic for dealing with null cases should go here, not in the subroutine.
			vector<const RoadSegment*> segments = (segmentsIt==config.getRoadSegments_Map().end()) ? vector<const RoadSegment*>() : segmentsIt->second;
			vector<const BusStop*> stops = (stopsIt==config.getBusStops_Map().end()) ? vector<const BusStop*>() : stopsIt->second;

			if(bustrip.setBusRouteInfo(segments, stops)) {
				busline->addBusTrip(bustrip);
			}
		}
	}
}


void sim_mob::BusController::storeRealTimes_eachBusStop(const std::string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, const BusStop* lastVisited_BusStop, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return;
	}
	std::cout << "busline_i: " << busline_i << "trip_k: " << trip_k << std::endl;
	std::cout << "busStopRealTimes_vec_bus[0]->get() " << busStopRealTimes_vec_bus[0]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[1]->get() " << busStopRealTimes_vec_bus[1]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[2]->get() " << busStopRealTimes_vec_bus[2]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[3]->get() " << busStopRealTimes_vec_bus[3]->get().real_ArrivalTime.getRepr_() << std::endl;

	double ETijk = 0;
	double departure_time = 0;
	departure_time = ATijk + (DTijk * 1000.0);
	BusStop_RealTimes busStop_RealTimes(ConfigParams::GetInstance().simStartTime + DailyTime(ATijk), ConfigParams::GetInstance().simStartTime + DailyTime(departure_time));
	busStop_RealTimes.setReal_BusStop(lastVisited_BusStop);
	busStopRealTimes_vec_bus[busstopSequence_j]->set(busStop_RealTimes);

	// here need test, need add fake RealTimes first
	busline->resetBusTrip_StopRealTimes(trip_k, busstopSequence_j, busStopRealTimes_vec_bus[busstopSequence_j]);// set this value for next step
}

double sim_mob::BusController::decisionCalculation(const string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus, const BusStop* lastVisited_BusStop)
{
	CONTROL_TYPE controltype = pt_schedule.findBuslineControlType(busline_i);
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<BusTrip>& BusTrips = busline->queryBusTrips();
	const BusStop* lastvisited_busStop = lastVisited_BusStop;// stored it for no control side

	double departure_time = 0; // If we use Control, since the busstopSequence_j is in the middle, so should not be 0
	double waitTime_BusStop = 0;

	switch(controltype) {
	case SCHEDULE_BASED:
		departure_time = scheduledDecision(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, busStopRealTimes_vec_bus, lastVisited_BusStop);
		waitTime_BusStop = (departure_time - ATijk) * 0.001;
		break;
	case HEADWAY_BASED:
		departure_time = headwayDecision(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, busStopRealTimes_vec_bus, lastVisited_BusStop);
		waitTime_BusStop = (departure_time - ATijk) * 0.001;
		break;
	case EVENHEADWAY_BASED:
		departure_time = evenheadwayDecision(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, busStopRealTimes_vec_bus, lastVisited_BusStop);
		waitTime_BusStop = (departure_time - ATijk) * 0.001;
		break;
	case HYBRID_BASED:
		departure_time = hybridDecision(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, busStopRealTimes_vec_bus, lastVisited_BusStop);
		waitTime_BusStop = (departure_time - ATijk) * 0.001;
		break;
	default:
		// may add default scheduled departure time here
		storeRealTimes_eachBusStop(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, lastVisited_BusStop, busStopRealTimes_vec_bus);
		waitTime_BusStop = DTijk;
		break;
	}
	return waitTime_BusStop;
}

double sim_mob::BusController::scheduledDecision(const string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus, const BusStop* lastVisited_busStop)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times

	//unsigned int DTijk = 0;
	double SETijk = 0;
	double ETijk = 0;
	double sij = 0;// slack size(should be zero)

	//Fwd_ATijk = ATijk;// assign value
	const vector<BusTrip>& BusTrips = busline->queryBusTrips();

	//StopInformation(Times)
	const vector<BusStop_ScheduledTimes>& busStopScheduledTime_tripK = BusTrips[trip_k].getBusStopScheduledTimes();
	//const vector<const BusStopInfo*>& busStopInfoFwd_tripK = busRouteInfoFwd_tripK->getBusStopsInfo();
	SETijk = busStopScheduledTime_tripK[busstopSequence_j].scheduled_DepartureTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

	//DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
	ETijk = std::max(SETijk - sij, ATijk + (DTijk * 1000.0));

	storeRealTimes_eachBusStop(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, lastVisited_busStop, busStopRealTimes_vec_bus);

	return ETijk;
}

double sim_mob::BusController::headwayDecision(const string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus, const BusStop* lastVisited_busStop)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times

	//unsigned int DTijk = 0;
	double ETijk = 0;
	double ATijk_1 = 0;
	double Hi = 0;
	double alpha = 0.6;// 0.7(Hi = 100000) range from 0.6 to 0.8

	std::cout << "busStopRealTimes_vec_bus[0]->get() " << busStopRealTimes_vec_bus[0]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[1]->get() " << busStopRealTimes_vec_bus[1]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[2]->get() " << busStopRealTimes_vec_bus[2]->get().real_ArrivalTime.getRepr_() << std::endl;
	std::cout << "busStopRealTimes_vec_bus[3]->get() " << busStopRealTimes_vec_bus[3]->get().real_ArrivalTime.getRepr_() << std::endl;

	if (0 == trip_k) {
		// the first trip just use Dwell Time, no holding strategy
		ETijk = ATijk + (DTijk * 1000.0);

		// Test
		const vector<BusTrip>& BusTrips = busline->queryBusTrips();
		const vector<Shared<BusStop_RealTimes>* >& busStopRealTime_tripK = BusTrips[trip_k].getBusStopRealTimes();
		std::cout << "busStopRealTime_tripK_1  size(): " << busStopRealTime_tripK.size() << std::endl;
		std::cout << "busStop_RealTimes : real_ArrivalTime output " << busStopRealTime_tripK[busstopSequence_j]->get().real_ArrivalTime.getRepr_() << std::endl;
		std::cout << "busStop_RealTimes : real_DepartureTime output " << busStopRealTime_tripK[busstopSequence_j]->get().real_DepartureTime.getRepr_() << std::endl;
		// Test

	} else {
		const vector<BusTrip>& BusTrips = busline->queryBusTrips();
		const vector<Shared<BusStop_RealTimes>* >& busStopRealTime_tripK_1 = BusTrips[trip_k - 1].getBusStopRealTimes();

//		std::cout << "busStopRealTime_tripK_1  size(): " << busStopRealTime_tripK_1.size() << std::endl;
		std::cout << "real_ArrivalTime: " << busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.getRepr_() << std::endl;
		if(busStopRealTime_tripK_1[busstopSequence_j]->get().Real_busStop) { // data has already updated
			ATijk_1 = busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);// there are some cases that buses are bunched together so that k-1 has no values updated yet
	//		Hi = BusTrips[trip_k].startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
	//				- BusTrips[trip_k - 1].startTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
			Hi = 143000;// 143000(best), 142000, 140000(headway*100), 138000, 181000(bad effect) ;60000(headway*50)

			ETijk = std::max(ATijk_1 + alpha*Hi, ATijk + (DTijk * 1000.0)); // DTijk unit is sec, so change to ms by multiplying 1000
		} else {// data has not yet updated, sometimes happens especially buses are bunched together(trip_k bus can overtake tripk_1 bus)
			ETijk = ATijk + (DTijk * 1000.0); // immediately leaving
		}


	}

	storeRealTimes_eachBusStop(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, lastVisited_busStop, busStopRealTimes_vec_bus);

	return ETijk;
}

double sim_mob::BusController::evenheadwayDecision(const string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus, const BusStop* lastVisited_busStop)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
	if(!busline) {
		std::cout << "wrong busline assigned:" << std::endl;
		return -1;
	}
	const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times
	const vector<BusTrip>& BusTrips = busline->queryBusTrips();

	//unsigned int DTijk = 0;
	double ETijk = 0;
	double ATijk_1 = 0;
	double ATimk_plus1 = 0;
	double SRTmj = 0;

    bool lastTrip = ((BusTrips.size()-1) == trip_k);//check if last trip
    int lastVisitedStopNum = 0;// check whether last visited Stop num is valid or not
    if(!lastTrip)
    {
    	lastVisitedStopNum = BusTrips[trip_k+1].lastVisitedStop_SequenceNumber;
    }

	if (0 == trip_k) {
		// the first trip just use Dwell Time, no holding strategy
		ETijk = ATijk + (DTijk * 1000.0);

	}
	else if(lastTrip || lastVisitedStopNum == -1){
		// If last trip or if next trip k+1 is not dispatched yet then use single headway
		return headwayDecision(busline_i,trip_k,busstopSequence_j,ATijk,DTijk, busStopRealTimes_vec_bus, lastVisited_busStop);
	}
	else {
		lastVisitedStopNum = BusTrips[trip_k+1].lastVisitedStop_SequenceNumber;//last stop visited by bus trip k+1
		const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripK_1 = BusTrips[trip_k - 1].getBusStopRealTimes();
		ATijk_1 = busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

		const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopRealTimes();
		ATimk_plus1 = busStopRealTime_tripKplus1[lastVisitedStopNum]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

		const vector<BusStop_ScheduledTimes>& busStopScheduledTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopScheduledTimes();
		SRTmj = busStopScheduledTime_tripKplus1[busstopSequence_j].scheduled_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
				- busStopScheduledTime_tripKplus1[lastVisitedStopNum].scheduled_DepartureTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

		//DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
		ETijk = std::max(ATijk_1 + (ATimk_plus1 + SRTmj - ATijk_1)/2.0, ATijk + (DTijk * 1000.0)); // need some changes for precision

	}

	std::cout<<"YaoJinTest:  busstop "<<busstopSequence_j<<" trip "<<trip_k<<" arrival time "<<ATijk<<" Departure time "<<ETijk<<std::endl;
	storeRealTimes_eachBusStop(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, lastVisited_busStop, busStopRealTimes_vec_bus);

	return ETijk;
}

double sim_mob::BusController::hybridDecision(const string& busline_i, int trip_k, int busstopSequence_j, double ATijk, double DTijk, std::vector<Shared<BusStop_RealTimes>* >& busStopRealTimes_vec_bus, const BusStop* lastVisited_busStop)
{
	Busline* busline = pt_schedule.findBusline(busline_i);
		if(!busline) {
			std::cout << "wrong busline assigned:" << std::endl;
			return -1;
		}
		const vector<Frequency_Busline>& freq_busline = busline->query_Frequency_Busline();// query different headways for different times
		const vector<BusTrip>& BusTrips = busline->queryBusTrips();

		//unsigned int DTijk = 0;
		double ETijk = 0;
		double ATijk_1 = 0;
		double ATimk_plus1 = 0;
		double SRTmj = 0;
		double Hi = 0;

	    bool lastTrip = ((BusTrips.size()-1) == trip_k);//check if last trip
	    int lastVisitedStopNum = 0;// check whether last visited Stop num is valid or not
	    if(!lastTrip)
	    {
	    	lastVisitedStopNum = BusTrips[trip_k+1].lastVisitedStop_SequenceNumber;
	    }

		if (0 == trip_k) {
			// the first trip just use Dwell Time, no holding strategy
			ETijk = ATijk + (DTijk * 1000.0);

		}
		else if(lastTrip || lastVisitedStopNum == -1){
			// If last trip or if next trip k+1 is not dispatched yet then use single headway
			return headwayDecision(busline_i,trip_k,busstopSequence_j,ATijk,DTijk, busStopRealTimes_vec_bus, lastVisited_busStop);
		}
		else {
			lastVisitedStopNum = BusTrips[trip_k+1].lastVisitedStop_SequenceNumber;//last stop visited by bus trip k+1
			const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripK_1 = BusTrips[trip_k - 1].getBusStopRealTimes();
			ATijk_1 = busStopRealTime_tripK_1[busstopSequence_j]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

			const vector <Shared<BusStop_RealTimes>* >& busStopRealTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopRealTimes();
			ATimk_plus1 = busStopRealTime_tripKplus1[lastVisitedStopNum]->get().real_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);

			const vector<BusStop_ScheduledTimes>& busStopScheduledTime_tripKplus1 = BusTrips[trip_k + 1].getBusStopScheduledTimes();
			SRTmj = busStopScheduledTime_tripKplus1[busstopSequence_j].scheduled_ArrivalTime.offsetMS_From(ConfigParams::GetInstance().simStartTime)
					- busStopScheduledTime_tripKplus1[lastVisitedStopNum].scheduled_DepartureTime.offsetMS_From(ConfigParams::GetInstance().simStartTime);
			Hi = 143000;
			//DTijk = dwellTimeCalculation(busline_i, trip_k, busstopSequence_j);
			ETijk = std::max(std::min(ATijk_1 + (ATimk_plus1 + SRTmj - ATijk_1)/2.0, (ATijk_1 + Hi)),(double)(ATijk) + (DTijk * 1000.0)); // need some changes for precision
		}

		std::cout<<"YaoJinTest:  busstop "<<busstopSequence_j<<" trip "<<trip_k<<" arrival time "<<ATijk<<" Departure time "<<ETijk<<std::endl;
		storeRealTimes_eachBusStop(busline_i, trip_k, busstopSequence_j, ATijk, DTijk, lastVisited_busStop, busStopRealTimes_vec_bus);

		return ETijk;
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

	LogOut("(\"BusController\""
			<<","<<now.frame()
			<<","<<getId()
			<<",{"
			<<"\"managedBuses size\":\""<<static_cast<int>(managedBuses.size())
			<<"\",\"Bus_xPos\":\""<<static_cast<int>(posBus.x)
			<<"\",\"Bus_yPos\":\""<<static_cast<int>(posBus.y)
			<<"\"})"<<std::endl);
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

