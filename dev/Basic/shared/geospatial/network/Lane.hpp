//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

#include "PolyLine.hpp"
#include "LaneConnector.hpp"
#include "RoadSegment.hpp"

namespace sim_mob
{

#define PEDESTRIAN_LANE 0b1000000
#define BICYCLE_LANE 0b100000
#define CAR_LANE 0b10000
#define VAN_LANE 0b1000
#define TRUCK_LANE 0b100
#define BUS_LANE 0b10
#define TAXI_LANE 0b1

/**Defines the rules for the bus lanes*/
enum BusLaneRules
{
	/**Both cars and buses can use the lane during the entire day*/
	BUS_LANE_RULES_CAR_AND_BUS = 0,

	/**Buses only from Mon-Fri: 0730-0930 and 1700-2000*/
	BUS_LANE_RULES_NORMAL_BUS_LANE = 1,

	/**Buses only from Mon-Sat: 0730-2000*/
	BUS_LANE_RULES_FULL_DAY_BUS_LANE = 2
};

class LaneConnector;
class RoadSegment;

/**
 * This class define the structure of a lane
 * \author Neeraj D
 * \author Harish L
 */
class Lane
{
private:

	/**Unique identifier for the lane*/
	unsigned int laneId;

	/**Identifies the rule applicable for the bus lane*/
	BusLaneRules busLaneRules;

	/**Defines if a vehicle can park on the lane*/
	bool canVehiclePark;

	/**Defines if a vehicle can stop on the lane*/
	bool canVehicleStop;

	/**Defines whether the lane has a road shoulder*/
	bool hasRoadShoulder;

	/**Defines whether a high occupancy vehicle is allowed on the lane*/
	bool isHOV_Allowed;

	/**The outgoing lane connectors*/
	std::vector<LaneConnector *> laneConnectors;

	/**Indicates the index of the lane*/
	unsigned int laneIndex;

	/**The road segment to which the lane belongs*/
	RoadSegment *parentSegment;

	/**Represents the poly-line of the lane*/
	PolyLine *polyLine;

	/**The id of the road segment to which the lane belongs*/
	unsigned int roadSegmentId;

	/**Defines the types of vehicles that can use the lane 
	 * 7 bits are used to identify the modes as follows:
	 * (MSB) Pedestrian Bicycle Car Van Truck Bus Taxi (LSB)
	 * Note: Number is stored as decimal in the database
	 */
	unsigned int vehicleMode;

	/**The width of the lane*/
	double width;

public:

	Lane();

	virtual ~Lane();

	unsigned int getLaneId() const;
	void setLaneId(unsigned int laneId);

	BusLaneRules getBusLaneRules() const;
	void setBusLaneRules(BusLaneRules busLaneRules);

	bool isParkingAllowed() const;
	void setCanVehiclePark(bool canVehiclePark);

	bool isStoppingAllowed() const;
	void setCanVehicleStop(bool canVehicleStop);

	bool doesLaneHaveRoadShoulder() const;
	void setHasRoadShoulder(bool hasRoadShoulder);

	bool isHighOccupancyVehicleAllowed() const;
	void setHighOccupancyVehicleAllowed(bool HighOccupancyVehicleAllowed);

	const std::vector<LaneConnector*>& getLaneConnectors() const;

	unsigned int getLaneIndex() const;

	const RoadSegment* getParentSegment() const;
	void setParentSegment(RoadSegment* parentSegment);

	PolyLine* getPolyLine() const;
	void setPolyLine(PolyLine* polyLine);

	unsigned int getRoadSegmentId() const;
	void setRoadSegmentId(unsigned int roadSegmentId);

	double getWidth() const;
	void setWidth(double width);

	/**
	 * Checks if the lane is a pedestrian lane
	 * @return true if the lane is a pedestrian lane; false otherwise
	 */
	bool isPedestrianLane() const;

	/**
	 * Checks if the lane is a bicycle lane
	 * @return true if the lane is a bicycle lane; false otherwise
	 */
	bool isBicycleLane() const;

	/**
	 * Adds lane connector to the vector of outgoing lane connectors
	 * @param laneConnector - The lane connector object to be added to the lane
	 */
	void addLaneConnector(LaneConnector *laneConnector);
};
}
