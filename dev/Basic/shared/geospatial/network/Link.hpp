//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "PolyLine.hpp"
#include "RoadSegment.hpp"
#include "Node.hpp"

namespace sim_mob
{

/**Defines the various categories of links supported by SimMobility*/
enum LinkCategory
{
	/**Unknown/Default category*/
	LINK_CATEGORY_UNKNOWN = 0,
	
	/**Category A*/
	LINK_CATEGORY_A = 1,

	/**Category B*/
	LINK_CATEGORY_B = 2,

	/**Category C*/
	LINK_CATEGORY_C = 3,

	/**Category D*/
	LINK_CATEGORY_D = 4,

	/**Category E*/
	LINK_CATEGORY_E = 5,	

	/**Slip-road*/
	LINK_CATEGORY_SLIP_ROAD = 6,
	
	/**Round-about*/
	LINK_CATEGORY_ROUND_ABOUT = 7
};

/**Defines the various types of links supported by SimMobility*/
enum LinkType
{
	/**Default road segment*/
	LINK_TYPE_DEFAULT = 0,

	/**Expressway*/
	LINK_TYPE_EXPRESSWAY = 1,

	/**Urban road*/
	LINK_TYPE_URBAN = 2,

	/**Ramp*/
	LINK_TYPE_RAMP = 3,

	/**Roundabout*/
	LINK_TYPE_ROUNDABOUT = 4,

	/**Access*/
	LINK_TYPE_ACCESS = 5
};

class RoadSegment;
class Node;

/**
 * Defines the structure of a Link
 * \author Neeraj D
 * \author Harish L
 */
class Link
{
private:

	/**Unique identifier for the link*/
	unsigned int linkId;

	/**Pointer to the node from which this link begins*/
	Node *fromNode;

	/**Indicates the node from which this link begins*/
	unsigned int fromNodeId;

	/**The length of the link in meters*/
	double length;

	/**Indicates the link category*/
	LinkCategory linkCategory;

	/**Indicates the link type*/
	LinkType linkType;

	/**The name of the road this link represents*/
	std::string roadName;

	/**The road segments making up the link*/
	std::vector<RoadSegment *> roadSegments;

	/**Pointer to the node at which this link ends*/
	Node *toNode;

	/**Indicates the node at which this link ends*/
	unsigned int toNodeId;

public:

	Link();

	virtual ~Link();

	unsigned int getLinkId() const;
	void setLinkId(unsigned int linkId);

	Node* getFromNode() const;
	void setFromNode(Node *fromNode);

	unsigned int getFromNodeId() const;
	void setFromNodeId(unsigned int fromNodeId);	

	LinkCategory getLinkCategory() const;
	void setLinkCategory(LinkCategory linkCategory);

	LinkType getLinkType() const;
	void setLinkType(LinkType linkType);

	std::string getRoadName() const;
	void setRoadName(std::string roadName);

	const std::vector<RoadSegment *>& getRoadSegments() const;
	const RoadSegment* getRoadSegment(int index) const;

	int getRoadSegmentIndex(const RoadSegment * seg)const;

	Node* getToNode() const;
	void setToNode(Node *toNode);

	unsigned int getToNodeId() const;
	void setToNodeId(unsigned int toNodeId);

	double getLength() const;

	/**
	 * Adds a road segment to the vector of road segments that make up the link
	 * @param roadSegment - the road segment to be added to the link
	 */
	void addRoadSegment(RoadSegment *roadSegment);

	/**
	 * Calculates the length of the link by summing up the lengths of the road segments within
     */
	void calculateLength();
};
}
