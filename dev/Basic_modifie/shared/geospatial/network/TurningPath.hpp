//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "PolyLine.hpp"
#include "Lane.hpp"
#include "TurningConflict.hpp"
#include "TurningGroup.hpp"

namespace sim_mob
{

class Lane;
class TurningGroup;

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
	Lane *fromLane;

	/**Indicates the id of the lane where the turning path begins*/
	unsigned int fromLaneId;

	/**Defines the max speed that the vehicles should adhere to on the turning (m/s)*/
	double maxSpeed;

	/**Represents the poly-line for the turning path*/
	PolyLine *polyLine;

	/**Represents the lane where the turning path ends*/
	Lane *toLane;

	/**Indicates the id of the lane at which the turning path ends*/
	unsigned int toLaneId;

	/**
	 * The turning conflicts that lie on the turning.
	 * The key for this map is the other turning path
	 */
	std::map<const TurningPath *, TurningConflict *> turningConflicts;

	/**The turning conflicts that lie on the turning in a sorted order (nearest conflict first)*/
	std::vector<TurningConflict *> conflicts;

	/**Indicates the id of the turning group to which this turning path belongs*/
	unsigned int turningGroupId;

	/**The turning group to which this turning path belongs*/
	TurningGroup *turningGroup;

public:
	TurningPath();
	virtual ~TurningPath();

	unsigned int getTurningPathId() const;
	void setTurningPathId(unsigned int turningPathId);

    const Lane* getFromLane() const;
	void setFromLane(Lane *fromLane);

	unsigned int getFromLaneId() const;
	void setFromLaneId(unsigned int fromLaneId);
    
    double getMaxSpeed() const;
	void setMaxSpeed(double maxSpeedKmph);

	PolyLine* getPolyLine() const;
	void setPolyLine(PolyLine *polyLine);

	const Lane* getToLane() const;
    void setToLane(Lane *toLane);

	unsigned int getToLaneId() const;
	void setToLaneId(unsigned int toLaneId);

	const std::map<const TurningPath *, TurningConflict *>& getTurningConflicts() const;
	const std::vector<TurningConflict *>& getConflictsOnPath() const;

	unsigned int getTurningGroupId() const;
	void setTurningGroupId(unsigned int turningGroupId);

	void setTurningGroup(TurningGroup *turningGroup);
	const TurningGroup* getTurningGroup() const;

	/**
	 * Gets the length of the turning path poly-line. This is equal to the length of the turning path.
	 *
	 * @return length of the turning path
	 */
	double getLength() const;
	
	/**
	 * Gets the width of the turning path. This is equal to the average of the widths of the from and 
	 * to lanes
     *
	 * @return width of the turning path
     */
    double getWidth() const;

	/**
	 * Adds the turning conflict to the map of conflicts
	 *
	 * @param other - the conflicting turning path
	 * @param conflict - the turning conflict to be added
	 */
	void addTurningConflict(const TurningPath *other, TurningConflict *conflict);

	/**
	 * This method looks up and returns the turning conflict between this turning path and the given turning path
     *
	 * @param turningPath - the turning path with which this turning has a conflict
     *
	 * @return the turning conflict if found, else NULL
     */
	const TurningConflict* getTurningConflict(const TurningPath *turningPath) const;
};
}
