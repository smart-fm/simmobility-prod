/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <bitset>
#include <sstream>

#include "Point2D.hpp"
#include "RoadSegment.hpp"

//using namespace geo;
/*namespace geo {
class lane_t_pimpl;
class Lanes_pimpl;
class segment_t_pimpl;
}*/

namespace sim_mob
{


//Which lane "side", left, right, or both?
struct LaneSide {
	bool left;
	bool right;
	bool both() const { return left && right; }
	bool leftOnly() const { return left && !right; }
	bool rightOnly() const { return right && !left; }
};

//Which lane should we change to? Includes "none".
enum LANE_CHANGE_SIDE {
	LCS_LEFT = -1,
	LCS_SAME = 0,
	LCS_RIGHT = 1
};



#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

namespace aimsun
{
//Forward declaration
class Loader;
} //End aimsun namespace



/**
 * A lane, including its movement rules and its parent segment.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 *
 * When the StreetDirectory creates a Lane, it needs to set up the rules for that lane.  Since
 * the bitset constructor will set the bits to false, the StreetDirectory would only need to
 * set up the rules which are to be true.
 *   \code
 *   // A road with 3 lanes for vehicle traffic.
 *   // Any vehicle on the middle lane can filter to the left lane only
 *   // if it is a bus or the driver is not law-abiding.
 *   // Vehicles on the right lane may turn right even though the signal
 *   // is red.
 *
 *   Lane* leftLane = new Lane;
 *   leftLane.is_vehicle_lane(true);
 *   leftLane.can_go_straight(true);
 *   leftLane.can_turn_left(true);
 *   leftLane.can_change_lane_right(true);
 *   leftLane.is_whole_day_bus_lane(true);
 *
 *   Lane* centerLane = new Lane;
 *   centerLane.is_vehicle_lane(true);
 *   centerLane.can_go_straight(true);
 *   centerLane.can_change_lane_left(true);
 *   centerLane.can_change_lane_right(true);
 *
 *   Lane* rightLane = new Lane;
 *   rightLane.is_vehicle_lane(true);
 *   rightLane.can_turn_right(true);
 *   rightLane.can_change_lane_left(true);
 *   rightLane.can_turn_on_red_signal(true);
 *   \endcode
 *
 * In reality, the above example would entail additional code.  The StreetDirectory would need
 * to query the GIS database to confirm, for example, that the 3rd lane is indeed a vehicle lane,
 * that vehicles must turn right and cannot go straight, that vehicles can turn even though on a
 * red signal, that drivers are not allowed to stop or park their vehicles, etc.
 *
 * Such initialization code would increase the simulator's startup time.  For this reason, the
 * simulator should have two initialization procedures; a command-line option flag would determine
 * which procedure is called.  The first procedure is based on the above discussion; at the end
 * the procedure saves the lane's rules as a bit-pattern to the database:
 *   \code
 *   save_to_database(lane.ID(), lane.to_string());
 *   \endcode
 *
 * The faster alternative initialization procedure initializes the lane's rules based on the
 * bit pattern that was saved in the database:
 *   \code
 *   const std::string bit_pattern = read_database(lane_ID);
 *   Lane* lane = new Lane;
 *   lane.set(bit_pattern);
 *   \endcode
 * or simply:
 *   \code
 *   const std::string bit_pattern = read_database(lane_ID);
 *   Lane* lane = new Lane(bit_pattern);
 *   \endcode
 */
class Lane {
private:
    /**
     * Lane movement rules.
     */
    enum LANE_MOVEMENT_RULES {
    // Let the compiler assigns value to each enum item.

	//Who can drive here.
	IS_BICYCLE_LANE,
	IS_PEDESTRIAN_LANE,
	IS_VEHICLE_LANE,
	IS_HIGH_OCCUPANCY_VEHICLE_LANE,

	//Will likely be moved to Intersection or Link
	IS_U_TURN_ALLOWED,

	//Lane changing rules, traffic light turning
    CAN_GO_STRAIGHT,
    CAN_TURN_LEFT,
    CAN_TURN_RIGHT,
	CAN_CHANGE_LANE_LEFT,
	CAN_CHANGE_LANE_RIGHT,
	CAN_TURN_ON_RED_SIGNAL,

	//Parking rules
	CAN_STOP_HERE,
	CAN_FREELY_PARK_HERE,

	//General driving restrictions
	IS_STANDARD_BUS_LANE,
	IS_WHOLE_DAY_BUS_LANE,
	IS_ROAD_SHOULDER,

    // Leave this as the last item, it is used as the template parameter of the
    // rules_ data member.
    MAX_LANE_MOVEMENT_RULES
    };


public:
    /** Return true if vehicles can go straight on this lane.  */
	bool can_go_straight() const { return rules_[CAN_GO_STRAIGHT]; }

    /** Return true if vehicles can turn left from this lane.  */
	bool can_turn_left() const { return rules_[CAN_TURN_LEFT]; }

    /** Return true if vehicles can turn right from this lane.  */
	bool can_turn_right() const { return rules_[CAN_TURN_RIGHT]; }

    /** Return true if vehicles can turn from this lane even when the signal is red.  */
	bool can_turn_on_red_signal() const { return rules_[CAN_TURN_ON_RED_SIGNAL]; }

    /**
     * Return true if vehicles can move to the adjacent lane on the left.
     *
     * Vehicles are unable to move to the adjacent left lane for several reasons:
     *   - there is no lane on the left.
     *   - the adjacent left lane is a raised pavement road divider.
     *   - the adjacent left lane is a road divider with railings.
     *   - the adjacent left lane is a sidewalk.
     *
     * Even if vehicles are allowed to move to the adjacent left lane, the lane could be
     * a designated bus lane or road shoulder.
     */
	bool can_change_lane_left() const { return rules_[CAN_CHANGE_LANE_LEFT]; }

    /**
     * Return true if vehicles can move to the adjacent lane on the right.
     *
     * Vehicles are unable to move to the adjacent right lane for several reasons:
     *   - there is no lane on the right.
     *   - the adjacent right lane is a raised pavement road divider.
     *   - the adjacent right lane is a road divider with railings.
     *   - the adjacent right lane is a sidewalk.
     *
     * Even if vehicles are allowed to move to the adjacent right lane, the lane could be
     * a designated bus lane or road shoulder.
     */
	bool can_change_lane_right() const { return rules_[CAN_CHANGE_LANE_RIGHT]; }

	/** Return true if this lane is a designated road shoulder.  */
	bool is_road_shoulder() const { return rules_[IS_ROAD_SHOULDER]; }

    /** Return true if this lane is reserved for cyclists.  */
    bool is_bicycle_lane() const { return rules_[IS_BICYCLE_LANE]; }

    /** Return true if this lane is reserved for pedestrians.  Duh, it's a sidewalk.  */
	bool is_pedestrian_lane() const { return rules_[IS_PEDESTRIAN_LANE]; }

    /** Return true if this lane is reserved for vehicle traffic.  */
	bool is_vehicle_lane() const { return rules_[IS_VEHICLE_LANE]; }

	/** Return true if this lane is reserved for buses during bus lane operation hours.  */
	bool is_standard_bus_lane() const { return rules_[IS_STANDARD_BUS_LANE]; }

    /** Return true if this lane is reserved for buses for the whole day.  */
	bool is_whole_day_bus_lane() const { return rules_[IS_WHOLE_DAY_BUS_LANE]; }

    /**
     * Return true if this lane is reserved for high-occupancy vehicles.
     *
     * A bus lane (standard or whole-day) is reserved for buses.  But a high-occupancy-vehicle
     * lane can be used by both buses and car-pools.
     */
	bool is_high_occupancy_vehicle_lane() const { return rules_[IS_HIGH_OCCUPANCY_VEHICLE_LANE]; }

    /**
     * Return true if agents can park their vehicles on this lane.
     *
     * The agent may have to pay to park on this lane.
     */
	bool can_freely_park_here() const { return rules_[CAN_FREELY_PARK_HERE]; }

    /**
     * Return true if agents can stop their vehicles on this lane temporarily.
     *
     * The agent may stop their vehicles for passengers to board or alight the vehicle
     * or for loading and unloading purposes.
     */
	bool can_stop_here() const { return rules_[CAN_STOP_HERE]; }

	/** Return true if vehicles can make a U-turn on this lane.  */
	bool is_u_turn_allowed() const { return rules_[IS_U_TURN_ALLOWED]; }

public:
    /** Return the RoadSegment this Lane is in. */
    sim_mob::RoadSegment* getRoadSegment() const {
        return parentSegment_;
    }

    ///Retrieve this lane's width. If 0, then this width is an even division of the
    /// road segment's total width.
    unsigned int getWidth() const {
    	if (width_==0) {
    		return parentSegment_->width / parentSegment_->getLanes().size();
    	}
    	return width_;
    }

    ///Return the laneID.
    ///NOTE: This will probably end up being the "tranlated" lane ID, instead of the "raw" lane ID.
    unsigned int getLaneID() const {
    	return laneID_;
    }
    std::string getLaneID_str() const {
    	return laneID_str;
    }

    /** Return the polyline of the Lane, which traces the middle of the lane.  */
    const std::vector<sim_mob::Point2D>& getPolyline() const;

    void insertNewPolylinePoint(Point2D p, bool isPre);

public:

    ///////////
    // TODO: These functions are used by the geo* classes, but we need to clean them up and only expose some of them. ~Seth
    ///////////

    //Lane() : parentSegment_(nullptr) {};//is needed by the xml reader

    /** Create a Lane using the \c bit_pattern to initialize the lane's rules.  */
    explicit Lane(sim_mob::RoadSegment* segment=nullptr, unsigned int laneID=0, const std::string& bit_pattern="") : parentSegment_(segment), rules_(bit_pattern), width_(0), laneID_(laneID) {
    	setLaneID(laneID);
    }

    void setParentSegment(sim_mob::RoadSegment* segment) {
    	parentSegment_ = segment;
    	setLaneID(laneID_);
    }

    void setLaneID(unsigned int laneID) {
    	this->laneID_ = laneID;

    	//Build a laneID string
    	std::ostringstream Id ;
    	if (parentSegment_) {
    		Id << parentSegment_->getSegmentID();
    	}
    	Id <<  laneID;
    	laneID_str = Id.str();
    }

    void setLaneWidth(unsigned int width) {
    	this->width_ = width;
    }

    void setLanePolyline(const std::vector<Point2D>& polyline) {
    	this->polyline_ = polyline;
    }


public:

#ifndef SIMMOB_DISABLE_MPI
	///The identification of Lane is packed using PackageUtils;
	static void pack(sim_mob::PackageUtils& package, const Lane* one_lane);

	///UnPackageUtils use the identification of Lane to find the Lane Object
	static Lane* unpack(UnPackageUtils& unpackage);

#endif

private:
    void makePolylineFromParentSegment();

    friend class StreetDirectory;
    friend class sim_mob::aimsun::Loader;
    
    /** Set the lane's rules using the \c bit_pattern.  */
    void set(const std::string& bit_pattern) {
        std::istringstream stream(bit_pattern);
        stream >> rules_;
   }

   /** Return the lane's rules as a string containing a bit pattern of '0' and '1'.  */
    std::string to_string() const { return rules_.to_string(); }


public:
    //NOTE: I don't see any reason to make these private. ~Seth

    /** If \c value is true, vehicles can go straight on this lane.  */
    void can_go_straight(bool value) { rules_.set(CAN_GO_STRAIGHT, value); }

    /** If \c value is true, vehicles can turn left from this lane.  */
    void can_turn_left(bool value) { rules_.set(CAN_TURN_LEFT, value); }

   /** If \c value is true, vehicles can turn right from this lane.  */
   void can_turn_right(bool value) { rules_.set(CAN_TURN_RIGHT, value); }

    /** If \c value is true, vehicles can turn from this lane even when the signal is red.  */
	void can_turn_on_red_signal(bool value) { rules_.set(CAN_TURN_ON_RED_SIGNAL, value); }

    /** If \c value is true, vehicles can move to the adjacent lane on the left.  */
	void can_change_lane_left(bool value) { rules_.set(CAN_CHANGE_LANE_LEFT, value); }

    /** If \c value is true, vehicles can move to the adjacent lane on the right.  */
	void can_change_lane_right(bool value) { rules_.set(CAN_CHANGE_LANE_RIGHT, value); }

    /** If \c value is true, this lane is a designated road shoulder.  */
	void is_road_shoulder(bool value) { rules_.set(IS_ROAD_SHOULDER, value); }

    /** If \c value is true, this lane is reserved for cyclists.  */
	void is_bicycle_lane(bool value) { rules_.set(IS_BICYCLE_LANE, value); }

    /** If \c value is true, this lane is reserved for pedestrians.  */
	void is_pedestrian_lane(bool value) { rules_.set(IS_PEDESTRIAN_LANE, value); }

    /** If \c value is true, this lane is reserved for vehicle traffic.  */
	void is_vehicle_lane(bool value) { rules_.set(IS_VEHICLE_LANE, value); }

    /** If \c value is true, this lane is reserved for buses during bus lane operation hours.  */
	void is_standard_bus_lane(bool value) { rules_.set(IS_STANDARD_BUS_LANE, value); }

    /** If \c value is true, this lane is reserved for buses for the whole day.  */
	void is_whole_day_bus_lane(bool value) { rules_.set(IS_WHOLE_DAY_BUS_LANE, value); }

    /** If \c value is true, this lane is reserved for high-occupancy vehicles.  */
	void is_high_occupancy_vehicle_lane(bool value) { rules_.set(IS_HIGH_OCCUPANCY_VEHICLE_LANE, value); }

    /** If \c value is true, agents can park their vehicles on this lane.  */
	void can_freely_park_here(bool value) { rules_.set(CAN_FREELY_PARK_HERE, value); }

    /** If \c value is true, agents can stop their vehicles on this lane temporarily.  */
	void can_stop_here(bool value) { rules_.set(CAN_STOP_HERE, value); }

    /** If \c value is true, vehicles can make a U-turn on this lane.  */
	void is_u_turn_allowed(bool value) { rules_.set(IS_U_TURN_ALLOWED, value); }

private:
	sim_mob::RoadSegment* parentSegment_;
	std::bitset<MAX_LANE_MOVEMENT_RULES> rules_;
	unsigned int width_;
	unsigned int laneID_;
	//I don't want to disturb the usage of laneID_ by other modules so i made the following variable for xml write/read purposes--vahid
	std::string laneID_str;

        // polyline_ is mutable so that getPolyline() can be a const method.
	mutable std::vector<Point2D> polyline_;

	friend class RoadSegment;



};





}
