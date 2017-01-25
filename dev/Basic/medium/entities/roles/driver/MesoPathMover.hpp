//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <vector>
#include "entities/conflux/SegmentStats.hpp"
#include "entities/conflux/Conflux.hpp"

namespace sim_mob
{
namespace medium
{
class MesoReroute;
class MesoPathMover
{
protected:
	//Note: Be careful if you want to change the container type of Path.
	//The functions in this class assume that Path is a container with random
	//access iterators. They perform operations like it+n where n is an integer.
	//If random access iterators are not available for Path, this class will
	//not work as expected.
	typedef std::vector<const SegmentStats*> Path;
	Path path;
	Path::iterator currSegStatIt;

	//representation of position within segment stats
	double distToSegmentEnd;

	friend MesoReroute;

public:
	MesoPathMover() :
			distToSegmentEnd(0)
	{
	}

	double getPositionInSegment() const
	{
		return this->distToSegmentEnd;
	}

	void setPositionInSegment(double distanceToEnd)
	{
		this->distToSegmentEnd = distanceToEnd;
	}

	/**
	 * sets path and resets the current iterator
	 * @param path the path to be set
	 */
	void setPath(const std::vector<const SegmentStats*>& path);
	const std::vector<const SegmentStats*>  getPath() const;

	/**
	 * resets the path. used when the path changes enroute.
	 * This function changes the path and sets the currSegStatIt to the same
	 * SegmentStats* in the new path.
	 *
	 * @param newPath the new path to be set
	 */
	void resetPath(const std::vector<const SegmentStats*>& newPath);

	/**
	 * gets the SegmentStats* pointed by currSegStatIt in the path
	 * @return constant pointer to SegmentStats corresponding to currSegStatIt
	 * 		or nullptr if currSegStatIt points to the end of the path
	 */
	const SegmentStats* getCurrSegStats() const;

	void setSegmentStatIterator(const SegmentStats* currSegStats);

	/**
	 * gets the SegmentStats* corresponding to the element in path next to currSegStatIt
	 * @param inSameLink indicates whether the next SegmentStats* is requested
	 * 		in the same link or adjacent link.
	 * @return constant pointer to SegmentStats corresponding to currSegStatIt+1
	 * 		if currSegStatIt and currSegStatIt+1 are not end of path and
	 * 		inSameLink condition is satisfied; nullptr otherwise.
	 */
	const SegmentStats* getNextSegStats(bool inSameLink = true) const;

	/**
	 * gets the SegmentStats* corresponding to the element in path at currSegStatIt+2
	 * @return constant pointer to SegmentStats corresponding to currSegStatIt+1
	 * 		if currSegStatIt, currSegStatIt+1 and currSegStatIt+2 are not end of path;
	 * 		nullptr otherwise.
	 */
	const SegmentStats* getSecondSegStatsAhead() const;

	/**
	 * gets the SegmentStats* corresponding to the element in path before currSegStatIt
	 * @param inSameLink indicates whether the previous SegmentStats* is requested
	 * 		in the same link or adjacent link.
	 * @return constant pointer to SegmentStats corresponding to currSegStatIt-1
	 * 		if currSegStatIt is not the first element in path and
	 * 		inSameLink condition is satisfied; nullptr otherwise.
	 */
	const SegmentStats* getPrevSegStats(bool inSameLink = true) const;

	/**
	 * tells whether subsequent SegmentStats* in path is in same link or not
	 * @param inSameLink indicates whether the next SegmentStats* is requested
	 * 		in the same link or adjacent link
	 * @return true nextSegment is available; false otherwise;
	 */
	bool hasNextSegStats(bool inSameLink) const;

	bool isEndOfPath();

	/**
	 * increments the currSegStatIt
	 */
	void advanceInPath();

	/**
	 * checks if currSegStatIt has reached the end of path
	 * @return (currSegStatIt == path.end())
	 */
	bool isPathCompleted() const;

	/**
	 * decrements the distToSegmentEnd by fwdDisplacement
	 * @param fwdDisplacement distance by which the driver has moved forward in segment stat
	 */
	void moveFwdInSegStats(double fwdDisplacement);
	void addPathFromCurrentSegmentToEndNodeOfLink();
	void erasePathAfterCurrenrLink();
	void appendRoute(std::vector<WayPoint> &routeToTaxiStand);
	void appendSegmentStats(const std::vector<RoadSegment*>& roadSegments,Conflux *conflux);
	void eraseFullPath();

	/**
	 * prints the seg stats path. useful for debugging
	 */
	void printPath();

	/**
	 * gets the first segstats in the immediate downstream Link of nextSegStats
	 * @param segStats downstream seg stats for which downstream link is required
	 * @return first segstats in the immediate downstream Link of path
	 */
	const SegmentStats* getFirstSegStatsInNextLink(const SegmentStats* segStats) const;

	//debug
	///return string of path by aimsun section id
	static std::string getPathString(const MesoPathMover::Path &path, const Node *node = 0);
};
}
}
