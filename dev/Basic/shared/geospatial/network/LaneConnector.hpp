//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Lane.hpp"

namespace sim_mob
{

class Lane;

/**
 * A lane connector connects one lane of a road segment to a lane of the next road segment in the same link.
 * This class define the structure of a lane
 * \author Neeraj D
 * \author Harish L
 */
class LaneConnector
{
private:

	/**Unique identifier for the lane connection*/
	unsigned int laneConnectionId;

	/**The lane from which the lane connection originates*/
	Lane *fromLane;

	/**Indicates the id of the lane from which the lane connection originates*/
	unsigned int fromLaneId;

	/**Indicates the id of the road segment from which the lane connection originates*/
	unsigned int fromRoadSegmentId;

	/**The lane at which the lane connection terminates*/
	Lane *toLane;

	/**Indicates the id of the lane at which the lane connection terminates*/
	unsigned int toLaneId;

	/**Indicates the id of the road segment at which the lane connection terminates*/
	unsigned int toRoadSegmentId;

public:

	LaneConnector();

	virtual ~LaneConnector();

	unsigned int getLaneConnectionId() const;
	void setLaneConnectionId(unsigned int laneConnectionId);
    
	void setFromLane(Lane *fromLane);
    const Lane* getFromLane() const;

	unsigned int getFromLaneId() const;
	void setFromLaneId(unsigned int fromLaneId);

	unsigned int getFromRoadSegmentId() const;
	void setFromRoadSegmentId(unsigned int fromRoadSectionId);
    
	void setToLane(Lane *toLane);
    const Lane* getToLane() const;

	unsigned int getToLaneId() const;
	void setToLaneId(unsigned int toLaneId);

	unsigned int getToRoadSegmentId() const;
	void setToRoadSegmentId(unsigned int toRoadSectionId);
};
}
