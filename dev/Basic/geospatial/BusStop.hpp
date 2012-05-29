/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

#include "RoadItem.hpp"
#include "Route.hpp"
#include "Lane.hpp"

namespace aimsun
{
//Forward declaration
class Loader;
} //End

namespace sim_mob
{

//Forward declarations
class Lane;
class BusRoute;

   //
/**
 * Representation of a Bus Stop.
 * \author Skyler Seto
 * \author Seth N. Hetu
 */
class BusStop : public sim_mob::RoadItem {
	
public:
	BusStop() : RoadItem() {}

	/* int getBusStopID() {
		   int busstopno = atoi(busstopno_);
	    	return busstopno;
	    }*/
public:
	///Which RoadItem and lane is this bus stop located at?
	Lane* lane_location;

	///Is this a bus bay, or does it take up space on the lane?
	///Bus bays are always to the dominant position away from the lane.
	///So, if drivingSide = OnLeft, then the bay extends to the left in its own lane.
	bool is_bay;

	///Is this a bus terminal? Currently, the only effect this has is to avoid
	///   requiring a bus to wait for the bus in front of it to leave.
	bool is_terminal;

	///How many meters of "bus" can park in the bus lane/bay to pick up pedestrians.
	///  Used to more easily represent double-length or mini-buses.
	unsigned int busCapacityAsLength;

	///Is the pedestrian waiting area sheltered? Currently does not affect anything.
	bool has_shelter;


private:
	///Get the bus lines available at this stop. Used for route planning.
	///NOTE: Placeholder method; will obviously not be returning void.
	std::vector <BusRoute> getBusLines() { throw std::runtime_error("Not implemented");  }

	///Get a list of bus arrival times. Pedestrians can consult this (assuming the bus stop is VMS-enabled).
	///NOTE: Placeholder method; will obviously not be returning void.
	void getBusArrivalVMS() {  }


	//Temporary items required for compiling
private:
	const std::vector<sim_mob :: Lane*>& getLanes() const { return lanes;}
	int bus_stop_lane(const RoadSegment& segment);
	sim_mob::Point2D getNearestPolyline(const sim_mob::Point2D &position);
	float getSumDistance();
public:
	sim_mob::RoadSegment* parentSegment_;
	std::string busstopno_;
		double xPos;
		double yPos;
		double x1d;
		double y1d;
		double x2d;
		double y2d;
		double x3d;
		double y3d;
		double x4d;
		double y4d;


#ifndef SIMMOB_DISABLE_MPI
	///The identification of Crossing is packed using PackageUtils;
	static void pack(PackageUtils& package, BusStop* one_bs);

	///UnPackageUtils use the identification of Crossing to find the Crossing Object
	static const BusStop* unpack(UnPackageUtils& unpackage);
#endif
	std::vector<sim_mob :: Lane*> lanes;
	friend class RoadSegment;

};



}
