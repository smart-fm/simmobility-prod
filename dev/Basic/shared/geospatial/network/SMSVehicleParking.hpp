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
    std::string parkingId;

    /**The segment at which the parking is located*/
    unsigned int segmentId;

    /**The road segment at which the parking is located*/
    const RoadSegment *parkingSegment;
    unsigned int parkingType;

    unsigned int capacityPCU;

    unsigned int vehicleTypeId;
    unsigned int nodeAccess;
    unsigned int nodeEgress;
    double distanceServiceRoad;
    unsigned int capacityServiceRoad;
    double segmentOffset;
    double startTime;
    double endTime;

public:

    /**Stores the vehicle parking as a r-tree*/
    static GeneralR_TreeManager<SMSVehicleParking> smsParkingRTree;

    SMSVehicleParking();
    virtual ~SMSVehicleParking();

    const std::string getParkingId() const;
    void setParkingId(const std::string id);

    const unsigned int getSegmentId() const;
    void setSegmentId(const unsigned int id);

    const RoadSegment *getParkingSegment() const;
    void setParkingSegment(const RoadSegment *rdSegment);

    const Node *getAccessNode() const;
    const Node *getEgressNode() const;

    double getPosX() const;
    double getPosY() const;
    const double getStartTime() const;
    void setStartTime(const double startTime);

    const double getEndTime() const;
    void setEndTime(const double endTime);

    const unsigned int getParkingType() const
    {
        return parkingType;
    }
    void setParkingType(const int parkingType)
    {
        this->parkingType=parkingType;
    }

    const unsigned int getCapacityPCU() const
    {
        return capacityPCU;
    }
    void setCapacityPCU(const int capacityPCU)
    {
        this->capacityPCU=capacityPCU;
    }

    const unsigned int getVehicleType() const
    {
        return vehicleTypeId;
    }
    void setVehicleType(const int vehicleTypeId)
    {
        this->vehicleTypeId=vehicleTypeId;
    }
};

}