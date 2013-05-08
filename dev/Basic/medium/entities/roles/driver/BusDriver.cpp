/*
 * BusDriver.cpp
 *
 *  Created on: May 6, 2013
 *      Author: zhang
 */

#include "BusDriver.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;
using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

sim_mob::medium::BusDriver::BusDriver(Agent* parent, MutexStrategy mtxStrat) : Driver(parent, mtxStrat) {
	// TODO Auto-generated constructor stub

}

sim_mob::medium::BusDriver::~BusDriver() {
	// TODO Auto-generated destructor stub
}


std::vector<BufferedBase*> sim_mob::medium::BusDriver::getSubscriptionParams() {
	vector<BufferedBase*> res;
	//res.push_back(&(currLane_));
	//res.push_back(&(currLaneOffset_));
	//res.push_back(&(currLaneLength_));
	return res;
}

sim_mob::UpdateParams& sim_mob::medium::BusDriver::make_frame_tick_params(timeslice now)
{
	params.reset(now, *this);
	return params;
}

void sim_mob::medium::BusDriver::frame_init(sim_mob::UpdateParams& p)
{
	Driver::frame_init(p);
}
void sim_mob::medium::BusDriver::frame_tick(sim_mob::UpdateParams& p)
{
	Driver::frame_tick(p);
}
void sim_mob::medium::BusDriver::frame_tick_output(const sim_mob::UpdateParams& p)
{
	Driver::frame_tick_output(p);
}

sim_mob::Vehicle* sim_mob::medium::BusDriver::initializePath(bool allocateVehicle)
{
	Vehicle* res = nullptr;

	//Only initialize if the next path has not been planned for yet.
	if(!parent->getNextPathPlanned()){
		//Save local copies of the parent's origin/destination nodes.
		origin.node = parent->originNode.node_;
		origin.point = origin.node->location;
		goal.node = parent->destNode.node_;
		goal.point = goal.node->location;

		//Retrieve the shortest path from origin to destination and save all RoadSegments in this path.
		vector<WayPoint> path;
		/*Person* parentP = dynamic_cast<Person*> (parent);
		if (!parentP || parentP->specialStr.empty()) {
			const StreetDirectory& stdir = StreetDirectory::instance();
			path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*origin.node), stdir.DrivingVertex(*goal.node));
		}*/

		vector<const RoadSegment*> pathRoadSeg;
		sim_mob::Person* person = dynamic_cast<sim_mob::Person*>(parent);
		if (person) {
			const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(person->currTripChainItem));
			if (!bustrip)
				std::cout << "bustrip is null\n";
			if (bustrip&& (*(person->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP) {
				pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
				std::cout << "BusTrip path size = " << pathRoadSeg.size() << std::endl;
				std::vector<const RoadSegment*>::iterator itor;
				for(itor=pathRoadSeg.begin(); itor!=pathRoadSeg.end(); itor++){
					path.push_back(WayPoint(*itor));
				}
			} else {
				if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_TRIP)
					std::cout << TripChainItem::IT_TRIP << " IT_TRIP\n";
				if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_ACTIVITY)
					std::cout << "IT_ACTIVITY\n";
				if ((*(person->currTripChainItem))->itemType== TripChainItem::IT_BUSTRIP)
			       std::cout << "IT_BUSTRIP\n";
				std::cout<< "BusTrip path not initialized coz it is not a bustrip, (*(person->currTripChainItem))->itemType = "<< (*(person->currTripChainItem))->itemType<< std::endl;
			}
		}

		//For now, empty paths aren't supported.
		if (path.empty()) {
			throw std::runtime_error("Can't initializePath(); path is empty.");
		}

		//TODO: Start in lane 0?
		int startlaneID = 0;

		// Bus should be at least 1200 to be displayed on Visualizer
		const double length = 400;
		const double width = 200;

		//A non-null vehicle means we are moving.
		if (allocateVehicle) {
			res = new Vehicle(path, startlaneID, length, width);
		}
	}

	//to indicate that the path to next activity is already planned
	parent->setNextPathPlanned(true);
	return res;

}


