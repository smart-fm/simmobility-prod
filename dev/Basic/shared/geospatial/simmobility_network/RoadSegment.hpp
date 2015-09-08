//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "Lane.hpp"
#include "Link.hpp"
#include "PolyLine.hpp"
#include "Tag.hpp"


namespace simmobility_network
{

class Link;
class Lane;

class RoadSegment
{
private:

	//Unique identifier for the road segment
	unsigned int roadSegmentId;

	//Indicates the maximum number of vehicles that the road segment can accommodate
	unsigned int capacity;

	//The lanes that make up th road segment
	std::vector<Lane *> lanes;

	//Length of the road segment
	double length;

	//The id of the link to which the segment belongs
	unsigned int linkId;

	//Indicates the maximum speed the vehicles should adhere to when traveling on the road segment
	unsigned int maxSpeed;

	//The link to which this segment belongs
	Link* parentLink;

	//Represents the poly-line for the road segment
	PolyLine *polyLine;

	//The name of the road to which this segment belongs
	std::string roadName;

	//Indicates the position of the road segment within the link
	unsigned int sequenceNumber;

	//Holds additional information
	std::vector<Tag>* tags;

public:
	RoadSegment();
	virtual ~RoadSegment();

	//Returns the road segment id
	unsigned int getRoadSegmentId() const;

	//Setter for the road segment id
	void setRoadSegmentId(unsigned int roadSegmentId);

	//Returns the value of the road segment capacity
	unsigned int getCapacity() const;

	//Setter for the road segment capacity
	void setCapacity(unsigned int capacity);

	//Returns the lane at the given index
	const Lane* getLane(int index) const;

	//Returns the vector of lanes that make up the segment
	const std::vector<Lane*>& getLanes() const;

	//Returns the length of the road segment
	double getLength();

	//Returns the id of the link to which the road segment belongs
	unsigned int getLinkId() const;

	//Setter for the id of the link to which the road segment belongs
	void setLinkId(unsigned int linkId);

	//Returns the max permissible speed for the road segment
	unsigned int getMaxSpeed() const;

	//Setter for the max permissible speed for the road segment
	void setMaxSpeed(unsigned int maxSpeed);

	//Returns a pointer to the link which contains the road segment
	const Link* getParentLink() const;

	//Setter for the parentLink of the road segment
	void setParentLink(Link* parentLink);

	//Returns the poly-line for the road segment
	const PolyLine* getPolyLine() const;

	//Sets the poly-line for the road segment
	void setPolyLine(PolyLine *polyLine);

	//Returns the sequence number of the road segment within the link
	unsigned int getSequenceNumber() const;

	//Setter for the sequence number of the road segment
	void setSequenceNumber(unsigned int sequenceNumber);

	//Returns a vector of tags which holds the additional information
	const std::vector<Tag>* getTags() const;

	//Setter for the tags field which holds the additional information
	void setTags(std::vector<Tag> *tags);

	//Adds a lane to the vector of lanes that make up the segment
	void addLane(Lane *lane);

};
}

