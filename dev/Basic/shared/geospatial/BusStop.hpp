/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>

#include "RoadItem.hpp"
#include "Route.hpp"
#include "Lane.hpp"
#include "util/DynamicVector.hpp"

namespace aimsun
{
//Forward declaration
class Loader;
} //End

namespace geo
{
class BusStop_t_pimpl;
}

namespace sim_mob
{

//Forward declarations
class Lane;
class BusRoute;
class Busline;

   //
/**
 * Representation of a Bus Stop.
 * \author Skyler Seto
 * \author Seth N. Hetu
 */
class BusStop : public sim_mob::RoadItem {
	friend class ::geo::BusStop_t_pimpl;
public:
	///BusStops must be constructed with their stopPt, which must be the same
	///  as the lane zero offset in their RoadSegment.
	explicit BusStop() : RoadItem() {lane_location = 0;
	busCapacityAsLength = 0;
	parentSegment_ = 0;
//	xPos = yPos = 0;
	}

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

	//The position bus shall stop in segment from start node
	//NOTE: This is now correctly stored in the RoadSegment's obstacle list.
	//const double stopPoint;
	//what if some one decides to make them public later?-vahid
	const inline bool hasShelter()const  { return has_shelter; }
	const inline bool isTerminal() const { return is_terminal; }
	const inline bool isBay() const { return is_bay; }
	const inline Lane* getLaneLocation() const { return lane_location; }
	const inline unsigned int getBusCapacityAsLength() const { return busCapacityAsLength; }
	const std::string getBusstopno_() const { return busstopno_; }


public:
    sim_mob::RoadSegment* getRoadSegment() const {
        return parentSegment_;
    }
    void setParentSegment(RoadSegment *rs) { parentSegment_ = rs;} //virtual from RoadItem

    //Estimate the stop point of this BusStop on a given road segment
    static double EstimateStopPoint(double xPos, double yPos, const sim_mob::RoadSegment* rs);


public:
    std::vector<Busline*> BusLines;///to store bus line info at each bus stop for passengers
	sim_mob::RoadSegment* parentSegment_;
	std::string busstopno_;
		double xPos;
		double yPos;

		std::vector<Point2D> position_;
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
