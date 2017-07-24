//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "geospatial/network/SMSVehicleParking.hpp"
#include "geospatial/network/Node.hpp"
#include "RoadNetwork.hpp"

using namespace sim_mob;

SMSVehicleParking::SMSVehicleParking() : parkingId(0), segmentId(0)
{
}

SMSVehicleParking::~SMSVehicleParking()
{
}

const unsigned int SMSVehicleParking::getParkingId() const
{
	return this->parkingId;
}

void SMSVehicleParking::setParkingId(unsigned int id)
{
	this->parkingId = id;
}

const unsigned int SMSVehicleParking::getSegmentId() const
{
	return segmentId;
}

void SMSVehicleParking::setSegmentId(unsigned int id)
{
	segmentId = id;
}

const RoadSegment *SMSVehicleParking::getParkingSegment() const
{
	return parkingSegment;
}

void SMSVehicleParking::setParkingSegment(const RoadSegment *rdSegment)
{
	parkingSegment = rdSegment;
}

const Node *SMSVehicleParking::getAccessNode() const
{
	return parkingSegment->getParentLink()->getFromNode();
}

const Node *SMSVehicleParking::getEgressNode() const
{
	return parkingSegment->getParentLink()->getToNode();
}

