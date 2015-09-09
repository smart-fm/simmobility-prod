//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "stddef.h"

namespace sim_mob
{

class TurningPath;

/**
 * A turning conflict is the point of overlap between two turning paths. This class defines
 * the structure of a turning conflict
 * \author Neeraj D
 * \author Harish L
 */
class TurningConflict
{
private:

	/**Unique identifier for the conflict*/
	unsigned int conflictId;

	/**
	 * Threshold value for accepting/rejecting the gap (and deciding whether to continue/slow down) between
	 * conflicting vehicles (in seconds)
	 */
	double criticalGap;

	/**Distance of conflict point from start of the first turning*/
	double firstConflictDistance;

	/**The first turning path in the conflict
	 * Note:: First/second doesn't have any significance
	 */
	TurningPath *firstTurning;

	/**Id of the first conflicting Turning path*/
	unsigned int firstTurningId;

	/**Indicates which turning has a higher priority.
	 * 0 - equal
	 * 1 - first_turning has higher priority
	 * 2 - second_turning has higher priority
	 */
	unsigned int priority;

	/**Distance of conflict point from the start of the second turning*/
	double secondConflictDistance;

	/**The second turning section in the conflict
	 * Note:: First/second doesn't have any significance
	 */
	TurningPath *secondTurning;

	/**Id of the second conflicting Turning path*/
	unsigned int secondTurningId;

public:

	TurningConflict();

	virtual ~TurningConflict();

	unsigned int getConflictId() const;
	void setConflictId(unsigned int conflictId);

	double getCriticalGap() const;
	void setCriticalGap(double criticalGap);

	double getFirstConflictDistance() const;
	void setFirstConflictDistance(double firstConflictDistance);

	TurningPath* getFirstTurning() const;
	void setFirstTurning(TurningPath* firstTurning);

	unsigned int getFirstTurningId() const;
	void setFirstTurningId(unsigned int firstTurningId);

	unsigned int getPriority() const;
	void setPriority(unsigned int priority);

	double getSecondConflictDistance() const;
	void setSecondConflictDistance(double secondConflictDistance);

	TurningPath* getSecondTurning() const;
	void setSecondTurning(TurningPath* secondTurning);

	unsigned int getSecondTurningId() const;
	void setSecondTurningId(unsigned int secondTurningId);
};
}
