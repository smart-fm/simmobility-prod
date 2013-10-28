//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriver.hpp"
#include "BusDriverFacets.hpp"

#include <vector>
#include <iostream>
#include <cmath>
#include "DriverUpdateParams.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/BusController.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "logging/Log.hpp"

#include "geospatial/Point2D.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/AuraManager.hpp"
#include "util/PassengerDistribution.hpp"
#include "entities/roles/activityRole/WaitBusActivityRole.hpp"

using namespace sim_mob;
using std::vector;
using std::map;
using std::string;

namespace {
//const int BUS_STOP_WAIT_PASSENGER_TIME_SEC = 2;
}//End anonymous namespace

sim_mob::BusDriver::BusDriver(Person* parent, MutexStrategy mtxStrat, sim_mob::BusDriverBehavior* behavior, sim_mob::BusDriverMovement* movement, Role::type roleType_) :
	Driver(parent, mtxStrat, behavior, movement, roleType_), existed_Request_Mode(mtxStrat, 0), waiting_Time(mtxStrat, 0),
	lastVisited_Busline(mtxStrat, "0"), lastVisited_BusTrip_SequenceNo(mtxStrat, 0), lastVisited_BusStop(mtxStrat, nullptr), lastVisited_BusStopSequenceNum(mtxStrat, 0),
	real_DepartureTime(mtxStrat, 0), real_ArrivalTime(mtxStrat, 0), DwellTime_ijk(mtxStrat, 0), busstop_sequence_no(mtxStrat, 0),
	xpos_approachingbusstop(-1), ypos_approachingbusstop(-1)
{
	last_busStopRealTimes = new Shared<BusStop_RealTimes>(mtxStrat,BusStop_RealTimes());
	if(parent) {
		if(parent->getAgentSrc() == "BusController") {
			BusTrip* bustrip = dynamic_cast<BusTrip*>(*(parent->currTripChainItem));
			if(bustrip && bustrip->itemType==TripChainItem::IT_BUSTRIP) {
				std::vector<const BusStop*> busStops_temp = bustrip->getBusRouteInfo().getBusStops();
				std::cout << "busStops_temp.size() " << busStops_temp.size() << std::endl;
				for(int i = 0; i < busStops_temp.size(); i++) {
					Shared<BusStop_RealTimes>* pBusStopRealTimes = new Shared<BusStop_RealTimes>(mtxStrat,BusStop_RealTimes());
					busStopRealTimes_vec_bus.push_back(pBusStopRealTimes);
				}
			}
		}
	}
}

Role* sim_mob::BusDriver::clone(Person* parent) const {
	BusDriverBehavior* behavior = new BusDriverBehavior(parent);
	BusDriverMovement* movement = new BusDriverMovement(parent);
	BusDriver* busdriver = new BusDriver(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(busdriver);
	movement->setParentDriver(busdriver);
	behavior->setParentBusDriver(busdriver);
	movement->setParentBusDriver(busdriver);
	return busdriver;
}

double sim_mob::BusDriver::getPositionX() const {
	return vehicle ? vehicle->getX() : 0;
}

double sim_mob::BusDriver::getPositionY() const {
	return vehicle ? vehicle->getY() : 0;
}

vector<BufferedBase*> sim_mob::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	res = Driver::getSubscriptionParams();

	// BusDriver's features
	res.push_back(&(lastVisited_BusStop));
	res.push_back(&(real_DepartureTime));
	res.push_back(&(real_ArrivalTime));
	res.push_back(&(DwellTime_ijk));
	res.push_back(&(busstop_sequence_no));
	res.push_back(last_busStopRealTimes);

	for(int j = 0; j < busStopRealTimes_vec_bus.size(); j++) {
		res.push_back(busStopRealTimes_vec_bus[j]);
	}

	return res;
}

void sim_mob::BusDriver::setBusStopRealTimes(const int& busstop_sequence_no, const BusStop_RealTimes& busstop_realTimes) {
	// busStopRealTimes_vec_bus empty validation
	if(!busStopRealTimes_vec_bus.empty()) {
		// busstop_sequence_no range validation
		if(busstop_sequence_no >= 0 && busstop_sequence_no <= (busStopRealTimes_vec_bus.size() - 1)) {
			// if the range is reasonable, set the BusStopRealTime for this bus stop
			busStopRealTimes_vec_bus[busstop_sequence_no]->set(busstop_realTimes);
		}
	}
}

sim_mob::DriverRequestParams sim_mob::BusDriver::getDriverRequestParams()
{
//	Person* person = dynamic_cast<Person*>(parent);
	sim_mob::DriverRequestParams res;

	res.existedRequest_Mode = &existed_Request_Mode;
	res.lastVisited_Busline = &lastVisited_Busline;
	res.lastVisited_BusTrip_SequenceNo = &lastVisited_BusTrip_SequenceNo;
	res.busstop_sequence_no = &busstop_sequence_no;
	res.real_ArrivalTime = &real_ArrivalTime;
	res.DwellTime_ijk = &DwellTime_ijk;
	res.lastVisited_BusStop = &lastVisited_BusStop;
	res.last_busStopRealTimes = last_busStopRealTimes;
	res.waiting_Time = &waiting_Time;

	return res;
}
