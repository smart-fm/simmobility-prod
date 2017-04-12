#pragma once

#include "Message.hpp"

namespace sim_mob
{

enum VehicleControllerMessage
{
	MSG_VEHICLE_REQUEST = 7000000,
	MSG_VEHICLE_REQUEST_RESPONSE,
	MSG_VEHICLE_ASSIGNMENT,
	MSG_VEHICLE_ASSIGNMENT_RESPONSE
};

/**
 * Message to request a vehicle
 */
class VehicleRequestMessage: public messaging::Message
{
public:
	VehicleRequestMessage(timeslice ct, const std::string& p, const unsigned int sn, const unsigned int dn) :
			currTick(ct), personId(p), startNodeId(sn), destinationNodeId(dn)
	{
	}

	virtual ~VehicleRequestMessage()
	{
	}

	const timeslice currTick;
	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};

/**
 * Message to assign a vehicle to a person
 */
class VehicleAssignmentMessage: public messaging::Message
{
public:
	VehicleAssignmentMessage(timeslice ct, const std::string& person, const unsigned int sn, const unsigned int dn) :
			currTick(ct), personId(person), startNodeId(sn), destinationNodeId(dn)
	{
	}
	virtual ~VehicleAssignmentMessage()
	{
	}

	const timeslice currTick;
	const std::string personId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};

/**
 * Message to respond to a vehicle assignment
 */
class VehicleAssignmentResponseMessage: public messaging::Message
{
public:
	VehicleAssignmentResponseMessage(timeslice ct, const bool s, const std::string& p, const std::string& t, const unsigned int sn, const unsigned int dn) :
			currTick(ct), success(s), personId(p), taxiDriverId(t), startNodeId(sn), destinationNodeId(dn)
	{
	}

	virtual ~VehicleAssignmentResponseMessage()
	{
	}

	const timeslice currTick;
	const bool success;
	const std::string personId;
	const std::string taxiDriverId;
	const unsigned int startNodeId;
	const unsigned int destinationNodeId;
};

}





