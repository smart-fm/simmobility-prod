//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "MesoPathMover.hpp"

#include <algorithm>
#include <sstream>
#include "geospatial/network/RoadSegment.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

void MesoPathMover::setPath(const std::vector<const SegmentStats*>& segStatPath)
{
	if (segStatPath.empty())
	{
		throw std::runtime_error("cannot assign an empty path to MesoPathMover");
	}
	path = segStatPath;
	currSegStatIt = path.begin();
}

const std::vector<const SegmentStats*>  MesoPathMover::getPath() const
{
	return path;
}
void MesoPathMover::resetPath(const std::vector<const SegmentStats*>& segStatPath)
{
	if (segStatPath.empty())
	{
		throw std::runtime_error("cannot assign an empty path to MesoPathMover");
	}
	if (!path.empty() && (currSegStatIt != path.end()))
	{
		const SegmentStats* currSegStat = *currSegStatIt;
		path.clear();
		path = segStatPath;
		currSegStatIt = std::find(path.begin(), path.end(), currSegStat);
		if (currSegStatIt == path.end())
		{
			throw std::runtime_error("MesoPathMover::resetPath() - new path does not contain current segment");
		}
	}
	else
	{
		path = segStatPath;
		currSegStatIt = path.begin();
	}
}

const SegmentStats* MesoPathMover::getCurrSegStats() const
{
	if (currSegStatIt == path.end())
	{
		return nullptr;
	}
	return (*currSegStatIt);
}

void MesoPathMover::setSegmentStatIterator(const SegmentStats* currSegStats)
{
	currSegStatIt = std::find(path.begin(),path.end(),currSegStats);
	const SegmentStats * sg = *(currSegStatIt);
	int degug = 1;
}

const SegmentStats* MesoPathMover::getNextSegStats(bool inSameLink) const
{
	if (currSegStatIt == path.end())
	{
		return nullptr;
	}
	Path::iterator nextStatIt = currSegStatIt + 1;
	if (nextStatIt == path.end())
	{
		return nullptr;
	}
	const SegmentStats* nextSegStats = (*nextStatIt);
	if (inSameLink && (nextSegStats->getRoadSegment()->getParentLink() != (*currSegStatIt)->getRoadSegment()->getParentLink()))
	{
		return nullptr;
	}
	return nextSegStats;
}

const SegmentStats* MesoPathMover::getSecondSegStatsAhead() const
{
	if (currSegStatIt == path.end())
	{
		return nullptr;
	}
	Path::iterator statIt = currSegStatIt + 1;
	if (statIt == path.end())
	{
		return nullptr;
	}
	statIt++; // currSegStatIt + 2
	if (statIt == path.end())
	{
		return nullptr;
	}
	return (*statIt);
}

const SegmentStats* MesoPathMover::getPrevSegStats(bool inSameLink) const
{
	if (currSegStatIt == path.begin())
	{
		return nullptr;
	}
	Path::iterator prevStatIt = currSegStatIt - 1;
	const SegmentStats* prevSegStats = (*prevStatIt);
	if (inSameLink && (prevSegStats->getRoadSegment()->getParentLink() != (*currSegStatIt)->getRoadSegment()->getParentLink()))
	{
		return nullptr;
	}

	return prevSegStats;
}

const SegmentStats* MesoPathMover::getFirstSegStatsInNextLink(const SegmentStats* segStats) const
{
	if (!segStats || currSegStatIt == path.end())
	{
		return nullptr;
	}

	Path::iterator it = currSegStatIt;
	for (; it != path.end(); it++) // locate segStats in downstream path
	{
		if ((*it) == segStats)
		{
			break;
		}
	}
	if (it == path.end())
	{
		return nullptr;
	}
	const Link* currLink = (*it)->getRoadSegment()->getParentLink(); //note segStats's link
	const SegmentStats *sStatFound = *(it);
	const RoadSegment *rdFound = sStatFound->getRoadSegment();
	it++; //start looking from stats after segStats
	int count =0;
	const SegmentStats *prevSegStat = (*it);
	for (; it != path.end(); it++)
	{
		const SegmentStats *segStat = (*it);
		const RoadSegment *rd = segStat->getRoadSegment();
		const Link *link = rd->getParentLink();
		if ((*it)->getRoadSegment()->getParentLink() != currLink)
		{
			return (*it);
		} //return if different link is found
		count++;
	}
	return nullptr;
}

bool MesoPathMover::hasNextSegStats(bool inSameLink) const
{

	if (currSegStatIt == path.end() || (currSegStatIt + 1) == path.end())
	{
		return false;
	}
	Path::iterator nextStatIt = currSegStatIt + 1;
	if (inSameLink)
	{
		return ((*currSegStatIt)->getRoadSegment()->getParentLink() == (*nextStatIt)->getRoadSegment()->getParentLink());
	}
	else
	{
		return ((*currSegStatIt)->getRoadSegment()->getParentLink() != (*nextStatIt)->getRoadSegment()->getParentLink());
	}
}

bool MesoPathMover::isEndOfPath()
{
	if (currSegStatIt == path.end()-1)
	{
		return true;
	}
}

void MesoPathMover::advanceInPath()
{
	if (currSegStatIt == path.end())
	{
		throw std::runtime_error("Error: Attempt to advance in path which is already complete.");
	}

	//Move
	currSegStatIt++;
}

bool MesoPathMover::isPathCompleted() const
{
	return (currSegStatIt == path.end());
}

void MesoPathMover::moveFwdInSegStats(double fwdDisplacement)
{
	if (currSegStatIt == path.end())
	{
		throw std::runtime_error("Error: Attempt to advance in path which is already complete.");
	}
	distToSegmentEnd -= fwdDisplacement;

	//fwdDisplacement should ideally be less than distToSegmentEnd.
	//Just to make sure it is so...
	distToSegmentEnd = std::max(distToSegmentEnd, 0.0);
}

void MesoPathMover::printPath()
{
	std::stringstream pathStream;
	pathStream << "SegmentStats path: ";
	for (Path::iterator i = path.begin(); i != path.end(); i++)
	{
		pathStream << (*i)->getRoadSegment()->getRoadSegmentId() << "-" << (*i)->getStatsNumberInSegment() << "|";
	}
	pathStream << std::endl;
	Print() << pathStream.str();
}

std::string MesoPathMover::getPathString(const Path &path, const Node *node)
{
	std::ostringstream out("");
	if (node)
	{
		out << node->getNodeId() << ": ";
	}
	for (Path::const_iterator it = path.begin(); it != path.end(); it++)
	{
		out << (*it)->getRoadSegment()->getRoadSegmentId() << "-" << (*it)->getStatsNumberInSegment() << "|";
	}
	if (out.str().size())
	{
		out << "\n";
	}
	return out.str();
}
