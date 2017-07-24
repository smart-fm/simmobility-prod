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
    unsigned int parking_id;
    unsigned int segment_id;
    unsigned int accessNodeID;
    unsigned int egressNodeID;
/*
    /**The access Node
    const Node *accessNode;

    /**The egress Node
    const Node *egressNode;
*/
public:
    ParkingDetail();

    virtual ~ParkingDetail();

    const unsigned int getParkingID() const;

    void setParkingID(const unsigned int pkID);

    const unsigned int getSegmentID() const;

    void setSegmentID(const unsigned int segmentID);

    const unsigned int getAccessNodeID() const;

    void setAccessNodeID(const unsigned int NodeID);

    const unsigned int getEgressNodeID() const;

    void setEgressNodeID(const unsigned int NodeID);

    const Node *getAccessNode() const;

//    void setAccessNode(const Node *node);

    const Node *getEgressNode() const;

//    void setEgressNode(const Node *node);

};

}