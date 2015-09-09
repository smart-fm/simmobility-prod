//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include "Lane.hpp"
#include "Link.hpp"
#include "PolyLine.hpp"


namespace sim_mob
{

class Link;
class Lane;

/**
 * This class define the structure of a road segment
 * \author Neeraj D
 * \author Harish L
 */
class RoadSegment
{
private:

	/**Unique identifier for the road segment*/
	unsigned int roadSegmentId;

	/**Indicates the maximum number of vehicles that the road segment can accommodate*/
	unsigned int capacity;

	/**The lanes that make up th road segment*/
	std::vector<Lane *> lanes;

	/**Length of the road segment*/
	double length;

	/**The id of the link to which the segment belongs*/
	unsigned int linkId;

	/**Indicates the maximum speed the vehicles should adhere to when travelling on the road segment*/
	unsigned int maxSpeed;

	/**The link to which this segment belongs*/
	Link* parentLink;

	/**Represents the poly-line for the road segment*/
	PolyLine *polyLine;

	/**The name of the road to which this segment belongs*/
	std::string roadName;

	/**Indicates the position of the road segment within the link*/
	unsigned int sequenceNumber;

public:
	RoadSegment();

	virtual ~RoadSegment();

	unsigned int getRoadSegmentId() const;
	void setRoadSegmentId(unsigned int roadSegmentId);

	unsigned int getCapacity() const;
	void setCapacity(unsigned int capacity);

	const std::vector<Lane*>& getLanes() const;
	const Lane* getLane(int index) const;

	unsigned int getLinkId() const;
	void setLinkId(unsigned int linkId);

	unsigned int getMaxSpeed() const;
	void setMaxSpeed(unsigned int maxSpeed);

	const Link* getParentLink() const;
	void setParentLink(Link* parentLink);

	const PolyLine* getPolyLine() const;
	void setPolyLine(PolyLine *polyLine);

	unsigned int getSequenceNumber() const;
	void setSequenceNumber(unsigned int sequenceNumber);

	/**
	 * Adds a lane to the vector of lanes that make up the segment
	 * @param lane - lane to be added
	 */
	void addLane(Lane *lane);

	/**
	 * Gets the length of the road segment poly-line. This is equal to the length of the segment
	 * @return length of the road segment
	 */
	double getLength();

};
}

