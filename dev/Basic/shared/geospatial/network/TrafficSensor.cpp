#include "TrafficSensor.hpp"

using namespace sim_mob;
using namespace messaging;

TrafficSensor::TrafficSensor() : MessageHandler(0), sensorId(0), sensorType(SENSOR_TYPE_INVALID), taskCode(0), zoneLength(0), segmentId(0),
	offsetDistance(0), workingProbability(0), laneId(0), trafficLightId(0)
{
}

TrafficSensor::~TrafficSensor()
{
}

unsigned int TrafficSensor::getSensorId() const
{
	return sensorId;
}

void TrafficSensor::setSensorId(unsigned int value)
{
	sensorId = value;
}

SensorType TrafficSensor::getSensorType() const
{
	return sensorType;
}

void TrafficSensor::setSensorType(const SensorType &value)
{
	sensorType = value;
}

unsigned int TrafficSensor::getTaskCode() const
{
	return taskCode;
}

void TrafficSensor::setTaskCode(unsigned int value)
{
	taskCode = value;
}

double TrafficSensor::getZoneLength() const
{
	return zoneLength;
}

void TrafficSensor::setZoneLength(double value)
{
	zoneLength = value;
}

unsigned int TrafficSensor::getSegmentId() const
{
	return segmentId;
}

void TrafficSensor::setSegmentId(unsigned int value)
{
	segmentId = value;
}

double TrafficSensor::getOffsetDistance() const
{
	return offsetDistance;
}

void TrafficSensor::setOffsetDistance(double value)
{
	offsetDistance = value;
}

double TrafficSensor::getWorkingProbability() const
{
	return workingProbability;
}

void TrafficSensor::setWorkingProbability(double value)
{
	workingProbability = value;
}

unsigned int TrafficSensor::getLaneId() const
{
	return laneId;
}

void TrafficSensor::setLaneId(unsigned int value)
{
	laneId = value;
}

unsigned int TrafficSensor::getTrafficLightId() const
{
	return trafficLightId;
}

void TrafficSensor::setTrafficLightId(unsigned int value)
{
	trafficLightId = value;
}

void TrafficSensor::HandleMessage(Message::MessageType type, const Message &message)
{
}

