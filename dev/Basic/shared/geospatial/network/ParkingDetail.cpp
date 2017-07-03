//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "geospatial/network/ParkingDetail.hpp"
#include "geospatial/network/Node.hpp"

using namespace sim_mob;

ParkingDetail::ParkingDetail() : parking_id(0), accessNodeID(0), egressNodeID(0), accessNode(nullptr),
                                 egressNode(nullptr)
{
}

ParkingDetail::~ParkingDetail()
{
}

const int ParkingDetail::getParkingID() const
{
	return this->parking_id;
}

void ParkingDetail::setParkingID(int pkID)
{
	this->parking_id = pkID;
}


const int ParkingDetail::getAccessNodeID() const
{
	return accessNodeID;
}

void ParkingDetail::setAccessNodeID(int NodeID)
{
	accessNodeID = NodeID;
}

const int ParkingDetail::getEgressNodeID() const
{
	return egressNodeID;
}

void ParkingDetail::setEgressNodeID(int NodeID)
{
	egressNodeID = NodeID;
}

const Node *ParkingDetail::getAccessNode() const
{
	return this->accessNode;
}

void ParkingDetail::setAccessNode(const Node *node)
{
	this->accessNode = node;
}

const Node *ParkingDetail::getEgressNode() const
{
	return this->egressNode;
}

void ParkingDetail::setEgressNode(const Node *node)
{
	this->egressNode = node;
}

const int ParkingDetail::getSegmentID() const
{
	return segment_id;
}

void ParkingDetail::setSegmentID(int segmentID)
{
	segment_id = segmentID;
}

