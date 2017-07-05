//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Node.hpp"

namespace sim_mob
{

class Node;

/**
 * Represents the  parking Info
 */
class ParkingDetail
{
private:
    int parking_id;
    int segment_id;
    int accessNodeID;
    int egressNodeID;

    /**The access Node*/
    const Node *accessNode;

    /**The egress Node*/
    const Node *egressNode;

public:
    ParkingDetail();

    virtual ~ParkingDetail();

    const int getParkingID() const;

    void setParkingID(const int pkID);

    const int getSegmentID() const;

    void setSegmentID(const int segmentID);

    const int getAccessNodeID() const;

    void setAccessNodeID(const int NodeID);

    const int getEgressNodeID() const;

    void setEgressNodeID(const int NodeID);

    const Node *getAccessNode() const;

    void setAccessNode(const Node *node);

    const Node *getEgressNode() const;

    void setEgressNode(const Node *node);

};

}