//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Node.hpp"
#include "RoadSegment.hpp"
#include "spatial_trees/GeneralR_TreeManager.hpp"

namespace sim_mob
{

class Node;
class RoadSegment;

/**
 * Represents the  parking Info
 */
class SMSVehicleParking
{
private:
	/**Identifier for the SMS vehicle parking*/
    unsigned int parkingId;

	/**The segment at which the parking is located*/
    unsigned int segmentId;

	/**The road segment at which the parking is located*/
	const RoadSegment *parkingSegment;

public:

	/**Stores the vehicle parking as a r-tree*/
	static GeneralR_TreeManager<SMSVehicleParking> smsParkingRTree;

	SMSVehicleParking();
    virtual ~SMSVehicleParking();

    const unsigned int getParkingId() const;
    void setParkingId(const unsigned int id);

    const unsigned int getSegmentId() const;
    void setSegmentId(const unsigned int id);

	const RoadSegment *getParkingSegment() const;
	void setParkingSegment(const RoadSegment *rdSegment);

	const Node *getAccessNode() const;
	const Node *getEgressNode() const;

	double getPosX() const;
	double getPosY() const;
};

}