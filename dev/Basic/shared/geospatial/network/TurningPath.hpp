//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include "PolyLine.hpp"
#include "TurningConflict.hpp"
#include "Lane.hpp"

namespace sim_mob
{
class Lane;

/**
 * A turning path connects one lane of a road segment in a link to another lane of a road segment in the next link.
 * This class defines the structure of a turning path.
 * \author Neeraj D
 * \author Harish L
 */
class TurningPath
{
private:

	/**Unique identifier for the turning path*/
	unsigned int turningPathId;

	/**Represents the lane where the turning path begins*/
	Lane* fromLane;

	/**Indicates the id of the lane where the turning path begins*/
	unsigned int fromLaneId;

	/**Defines the max speed that the vehicles should adhere to on the turning (m/s)*/
	double maxSpeed;

	/**Represents the poly-line for the turning path*/
	PolyLine* polyLine;

	/**Represents the lane where the turning path ends*/
	Lane* toLane;

	/**Indicates the id of the lane at which the turning path ends*/
	unsigned int toLaneId;

	/**The turning conflicts that lie on the turning.
	 * The key for this map is the other turning path
	 */
	std::map<TurningPath *, TurningConflict *> turningConflicts;

	/**Indicates the id of the turning group to which this turning path belongs*/
	unsigned int turningGroupId;

public:

	TurningPath();

	virtual ~TurningPath();

	unsigned int getTurningPathId() const;
	void setTurningPathId(unsigned int turningPathId);

	unsigned int getFromLaneId() const;
	void setFromLaneId(unsigned int fromLaneId);

	PolyLine* getPolyLine() const;
	void setPolyLine(PolyLine* polyLine);

	unsigned int getToLaneId() const;
	void setToLaneId(unsigned int toLaneId);

	unsigned int getTurningGroupId() const;
	void setTurningGroupId(unsigned int turningGroupId);

	/**
	 * Adds the turning conflict to the map of conflicts
	 * @param other - the conflicting turning path
	 * @param conflict - the turning conflict to be added
	 */
	void addTurningConflict(TurningPath *other, TurningConflict *conflict);

	/**
	 * Gets the length of the turning path poly-line. This is equal to the length of the turning path.
	 * @return length of the turning path
	 */
	double getLength() const;
};
}
