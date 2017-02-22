//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "message/MessageHandler.hpp"

namespace sim_mob
{

enum SensorType
{
	SENSOR_TYPE_INVALID = 0,
	SENSOR_TYPE_TRAFFIC = 1,
	SENSOR_TYPE_VEHICLE = 2,
	SENSOR_TYPE_VRC = 3,
	SENSOR_TYPE_AREA = 4,
	SENSOR_TYPE_TRAFFIC_LINK_WIDE = 257,
	SENSOR_TYPE_VEHICLE_LINK_WIDE = 258,
	SENSOR_TYPE_VRC_LINK_WIDE = 259,
	SENSOR_TYPE_AREA_LINK_WIDE = 260
};

class TrafficSensor : public messaging::MessageHandler
{
private:
	/**Unique identifier for the sensor*/
	unsigned int sensorId;

	/**
	 * Defines the type of sensor. The sensor can be of the following types
	 * Traffic: 1 [measures vehicle counts]
	 * Vehicle: 2 [vehicle probe sensor e.g. gantry]
	 * VRC: 3 [vehicle to roadside communication]
	 * Area: 4  [detection zone spans an area, e.g. camera]
	 * 256 added to above value if the sensor is across the link (i.e. not at lane level);
	 */
	SensorType sensorType;

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
	 * 0x0040 Departure time0x0080 Origin
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
	double zoneLength;

	/**Id of the segment at which the sensor is located*/
	unsigned int segmentId;

	/**Position of the sensor in terms of distance from the start of the road segment*/
	double offsetDistance;

	/**The probability that the sensor is in working condition*/
	double workingProbability;

	/**Id of the lane if the sensor is lane level, 0 otherwise*/
	unsigned int laneId;

	/**The associated traffic light id, 0 if sensor is not associated with traffic lights*/
	unsigned int trafficLightId;

public:
	TrafficSensor();
	virtual ~TrafficSensor();

	unsigned int getSensorId() const;
	void setSensorId(unsigned int value);

	SensorType getSensorType() const;
	void setSensorType(const SensorType &value);

	unsigned int getTaskCode() const;
	void setTaskCode(unsigned int value);

	double getZoneLength() const;
	void setZoneLength(double value);

	unsigned int getSegmentId() const;
	void setSegmentId(unsigned int value);

	double getOffsetDistance() const;
	void setOffsetDistance(double value);

	double getWorkingProbability() const;
	void setWorkingProbability(double value);

	unsigned int getLaneId() const;
	void setLaneId(unsigned int value);

	unsigned int getTrafficLightId() const;
	void setTrafficLightId(unsigned int value);

	/**
	 * Handles all messages
	 * .
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);
};

}
