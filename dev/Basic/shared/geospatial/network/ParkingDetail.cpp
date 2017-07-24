//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "geospatial/network/ParkingDetail.hpp"
#include "geospatial/network/Node.hpp"
#include "RoadNetwork.hpp"

using namespace sim_mob;

ParkingDetail::ParkingDetail() : parking_id(0), segment_id(0),accessNodeID(0), egressNodeID(0) //, accessNode(nullptr),egressNode(nullptr)
{
}

ParkingDetail::~ParkingDetail()
{
}

const unsigned int ParkingDetail::getParkingID() const
{
	return this->parking_id;
}

void ParkingDetail::setParkingID(unsigned int pkID)
{
	this->parking_id = pkID;
}


const unsigned int ParkingDetail::getAccessNodeID() const
{
	return accessNodeID;
}

void ParkingDetail::setAccessNodeID(unsigned int NodeID)
{
	accessNodeID = NodeID;
}

const unsigned int ParkingDetail::getEgressNodeID() const
{
	return egressNodeID;
}

void ParkingDetail::setEgressNodeID(unsigned int NodeID)
{
	egressNodeID = NodeID;
}

const unsigned int ParkingDetail::getSegmentID() const
{
	return segment_id;
}

void ParkingDetail::setSegmentID(unsigned int segmentID)
{
	segment_id = segmentID;
}
const Node *ParkingDetail::getAccessNode() const
{
	const sim_mob::RoadNetwork *roadNetwork = sim_mob::RoadNetwork::getInstance();
	return roadNetwork->getMapOfIdvsNodes().at(this->getAccessNodeID());
}

/*void ParkingDetail::setAccessNode(const Node *node)
{
	this->accessNode = node;
}*/

const Node *ParkingDetail::getEgressNode() const
{
	const sim_mob::RoadNetwork *roadNetwork = sim_mob::RoadNetwork::getInstance();
	return roadNetwork->getMapOfIdvsNodes().at(egressNodeID);
}

/*void ParkingDetail::setEgressNode(const Node *node)
{
	this->egressNode = node;
}*/



