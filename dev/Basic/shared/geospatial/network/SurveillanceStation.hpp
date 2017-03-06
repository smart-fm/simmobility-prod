//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>

namespace sim_mob
{

class Lane;
class RoadSegment;
class TrafficSensor;

class SurveillanceStation
{
private:
	/**Unique identifier for the surveillance station*/
	unsigned int stationId;

	/**
	 * Defines the type of sensor. The sensor can be of the following types
	 * Traffic: 1 [measures vehicle counts]
	 * Vehicle: 2 [vehicle probe sensor e.g. gantry]
	 * VRC: 3 [vehicle to roadside communication]
	 * Area: 4  [detection zone spans an area, e.g. camera]
	 * 256 added to above value if the sensor is across the link (i.e. not at lane level);
	 */
	unsigned int stationType;

	/**
	 * It represents  the  mode  of  operation  of  the  sensor  and  the  data  it  collects.  The  corresponding  hexadecimal  value  is  the
	 * sum  (in  base  16)  of  the  different options described below:
	 * Traffic sensor:
	 * 0x0001 Flow
	 * 0x0002 Speed
	 * 0x0004 Occupancy
	 *
	 * Vehicle/VRC:
	 * 0x0010 ID
	 * 0x0020 Type
	 * 0x0040 Departure time
	 * 0x0080 Origin
	 * 0x0100 Destination
	 * 0x0200 Path
	 * 0x0400 Height
	 * 0x0800 Current speed
	 *
	 * Area:
	 * 0x0010 ID
	 * 0x0020 Type
	 * 0x0400 Height
	 * 0x0800 Current speed
	 * 0x1000 Lane ID
	 * 0x2000 Position in lane
	 *
	 * For example, a task code of 0x01c1 corresponds to
	 * 0x01  +  0x0040    +   0x0080   +    0x0100      =      0x01C1
	 * {flow, departure time, origin, and destination};
	 */
	unsigned int taskCode;

	/**Length of the sensor detection zone*/
	unsigned int zoneLength;

	/**Position of the sensor in terms of distance from the start of the road segment*/
	unsigned int offsetDistance;

	/**Id of the segment at which the sensor is located*/
	unsigned int segmentId;

	/**Road segment containing the surveillance station*/
	const RoadSegment *segment;

	/**The associated traffic light id, 0 if sensor is not associated with traffic lights*/
	unsigned int trafficLightId;

	/**The traffic sensors within the surveillance station*/
	std::vector<TrafficSensor *> trafficSensors;

public:
	SurveillanceStation(unsigned int id, unsigned int type, unsigned int code, double zone, double offset,
						unsigned int segId, unsigned int trafficLight);
	~SurveillanceStation();

	unsigned int getSurveillanceStnId() const;
	void setSurveillanceStnId(unsigned int value);

	unsigned int isLinkWide() const;
	void setType(const unsigned int value);

	unsigned int getTaskCode() const;
	void setTaskCode(unsigned int value);

	unsigned int getZoneLength() const;
	void setZoneLength(unsigned int value);

	unsigned int getOffsetDistance() const;
	void setOffsetDistance(unsigned int value);

	unsigned int getSegmentId() const;
	void setSegmentId(unsigned int value);

	const RoadSegment *getSegment() const;
	void setSegment(const RoadSegment *value);

	unsigned int getTrafficLightId() const;
	void setTrafficLightId(unsigned int value);

	TrafficSensor * getTrafficSensor(int index);
	const std::vector<TrafficSensor *>& getTrafficSensors() const;
};

class TrafficSensor
{
private:
	/**Id of the traffic sensor*/
	unsigned int id;

	/**index of the sensor within the container (surveillance station)*/
	unsigned int index;

	/**The lane where the sensor is located*/
	const Lane *lane;

	/**The surveillance station to which the sensor belongs*/
	SurveillanceStation *surveillanceStn;

	/**Number of vehicles counted*/
	unsigned int count;

	/**The time occupied*/
	double occupancy;

	/**The sum of the speeds of the vehicles counted*/
	double speed;

	/**Time previous vehicle left the detector*/
	double prevTimeDetectorOff;

	/**
	 * Calculate the speed at the moment that the vehicle's back bumper leaves the detector, i.e. speed sensed by the sensor
	 * @param vehPosition vehicle position
	 * @param vehLength vehicle length
	 * @param vehSpeed speed of the vehicle
	 * @param acceleration acceleration of the vehicle
	 * @return the speed the vehicle passes the detector.
	 */
	double calculateSensorSpeed(double vehPosition, double vehLength, double vehSpeed, double acceleration);

	/**
	 * Calculate the time required to travel a distance, given the vehicle's initial speed and
	 * a constant acc/deceleration
	 * @param distCovered distance convered
	 * @param vehSpeed initial speed of the vehicle
	 * @param acceleration acceleration
	 * @return time required to cover distance
	 */
	double calcTimeRequiredToCoverDistance(double distCovered, double vehSpeed, double acceleration);

public:
	TrafficSensor(SurveillanceStation *station);
	TrafficSensor(SurveillanceStation *station, const Lane *lane, unsigned int index);
	~TrafficSensor();

	unsigned int getId() const;

	unsigned int getIndex() const;
	void setIndex(unsigned int value);

	const Lane *getLane() const;
	void setLane(const Lane *value);

	SurveillanceStation *getSurveillanceStn() const;
	void setSurveillanceStn(SurveillanceStation *value);

	unsigned int getCount() const;
	double getOccupancy() const;
	double getSpeed() const;

	void resetReadings();

	/**
	 * Calculates the sensor data which are accumulated if the entire or a part of the vehicle was/is in the detection zone.
	 * @param vehPosition vehicle position
	 * @param vehLength vehicle length
	 * @param vehSpeed speed of the vehicle
	 * @param acceleration acceleration of the vehicle
	 * @param currTime the current time
	 */
	void calculateActivatingData(double vehPosition, double vehLength, double vehSpeed, double acceleration, unsigned int currTime);

	/**
	 * Calculates the sensor data which are accumulated back bumper of the vehicle crosses the down-edge of the
	 * of detection zone
	 * @param vehPosition vehicle position
	 * @param vehLength vehicle length
	 * @param vehSpeed speed of the vehicle
	 * @param acceleration acceleration of the vehicle
	 */
	void calculatePassingData(double vehPosition, double vehLength, double vehSpeed, double acceleration);	
};

}
