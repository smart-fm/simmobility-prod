//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "MesoPathMover.hpp"

#include <algorithm>
#include "geospatial/RoadSegment.hpp"
#include "util/Profiler.hpp"

void sim_mob::medium::MesoPathMover::setPath(const std::vector<const sim_mob::SegmentStats*>& segStatPath)
{
	if (segStatPath.empty())
	{
		throw std::runtime_error("cannot assign an empty path to MesoPathMover");
	}
	path = segStatPath;
	currSegStatIt = path.begin();
}

const std::vector<const sim_mob::SegmentStats*> & sim_mob::medium::MesoPathMover::getPath() const{
	return path;
}
void sim_mob::medium::MesoPathMover::resetPath(const std::vector<const sim_mob::SegmentStats*>& segStatPath)
{
	if (segStatPath.empty())
	{
		throw std::runtime_error("cannot assign an empty path to MesoPathMover");
	}
	if (!path.empty() && (currSegStatIt != path.end()))
	{
		const sim_mob::SegmentStats* currSegStat = *currSegStatIt;
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

const sim_mob::SegmentStats* sim_mob::medium::MesoPathMover::getCurrSegStats() const
{
	if (currSegStatIt == path.end())
	{
		return nullptr;
	}
	return (*currSegStatIt);
}

const sim_mob::SegmentStats* sim_mob::medium::MesoPathMover::getNextSegStats(bool inSameLink) const
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
	const sim_mob::SegmentStats* nextSegStats = (*nextStatIt);
	if (inSameLink && (nextSegStats->getRoadSegment()->getLink() != (*currSegStatIt)->getRoadSegment()->getLink()))
	{
		return nullptr;
	}
	return nextSegStats;
}

const sim_mob::SegmentStats* sim_mob::medium::MesoPathMover::getSecondSegStatsAhead() const
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

const sim_mob::SegmentStats* sim_mob::medium::MesoPathMover::getPrevSegStats(bool inSameLink) const
{
	if (currSegStatIt == path.begin())
	{
		return nullptr;
	}
	Path::iterator prevStatIt = currSegStatIt - 1;
	const sim_mob::SegmentStats* prevSegStats = (*prevStatIt);
	if (inSameLink && (prevSegStats->getRoadSegment()->getLink() != (*currSegStatIt)->getRoadSegment()->getLink()))
	{
		return nullptr;
	}

	return prevSegStats;
}

bool sim_mob::medium::MesoPathMover::hasNextSegStats(bool inSameLink) const
{
	if (currSegStatIt == path.end() || (currSegStatIt + 1) == path.end())
	{
		return false;
	}
	Path::iterator nextStatIt = currSegStatIt + 1;
	if (inSameLink)
	{
		return ((*currSegStatIt)->getRoadSegment()->getLink() == (*nextStatIt)->getRoadSegment()->getLink());
	}
	else
	{
		return ((*currSegStatIt)->getRoadSegment()->getLink() != (*nextStatIt)->getRoadSegment()->getLink());
	}
}

void sim_mob::medium::MesoPathMover::advanceInPath()
{
	if (currSegStatIt == path.end())
	{
		throw std::runtime_error("Error: Attempt to advance in path which is already complete.");
	}

	//Move
	currSegStatIt++;
}

bool sim_mob::medium::MesoPathMover::isPathCompleted() const
{
	return (currSegStatIt == path.end());
}

void sim_mob::medium::MesoPathMover::moveFwdInSegStats(double fwdDisplacement)
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


void sim_mob::medium::MesoPathMover::printPath(const Path &path, const Node *node){
	std::ostringstream out("");
	unsigned int id = 0;
	if(node){
		out << node->getID() << ": " ;
	}
	for(Path::const_iterator it = path.begin(); it != path.end(); it++){
		if(id != (*it)->getRoadSegment()->getSegmentAimsunId()){
			id = (*it)->getRoadSegment()->getSegmentAimsunId();
			out << id << "," ;
		}
	}
	sim_mob::Logger::log["path_set"] << out.str() << std::endl;
}
