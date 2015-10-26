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

const std::vector<const SegmentStats*> & MesoPathMover::getPath() const{
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
	if (inSameLink && (nextSegStats->getRoadSegment()->getLink() != (*currSegStatIt)->getRoadSegment()->getLink()))
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
	if (inSameLink && (prevSegStats->getRoadSegment()->getLink() != (*currSegStatIt)->getRoadSegment()->getLink()))
	{
		return nullptr;
	}

	return prevSegStats;
}

const sim_mob::SegmentStats* sim_mob::medium::MesoPathMover::getFirstSegStatsInNextLink(const SegmentStats* segStats) const
{
	if(!segStats || currSegStatIt == path.end()) { return nullptr; }

	Path::iterator it = currSegStatIt;
	for(; it!=path.end(); it++) // locate segStats in downstream path
	{
		if((*it) == segStats) { break; }
	}
	if(it == path.end()) { return nullptr; }
	const sim_mob::Link* currLink = (*it)->getRoadSegment()->getLink(); //note segStats's link
	it++; //start looking from stats after segStats
	for(; it!=path.end(); it++)
	{
		if((*it)->getRoadSegment()->getLink() != currLink) { return (*it); } //return if different link is found
	}
	return nullptr;
}

const SegmentStats* MesoPathMover::getFirstSegStatsInNextLink(const SegmentStats* segStats) const
{
	if(!segStats || currSegStatIt == path.end()) { return nullptr; }

	Path::iterator it = currSegStatIt;
	for(; it!=path.end(); it++) // locate segStats in downstream path
	{
		if((*it) == segStats) { break; }
	}
	if(it == path.end()) { return nullptr; }
	const Link* currLink = (*it)->getRoadSegment()->getLink(); //note segStats's link
	it++; //start looking from stats after segStats
	for(; it!=path.end(); it++)
	{
		if((*it)->getRoadSegment()->getLink() != currLink) { return (*it); } //return if different link is found
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
		return ((*currSegStatIt)->getRoadSegment()->getLink() == (*nextStatIt)->getRoadSegment()->getLink());
	}
	else
	{
		return ((*currSegStatIt)->getRoadSegment()->getLink() != (*nextStatIt)->getRoadSegment()->getLink());
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
	for(Path::iterator i=path.begin(); i!=path.end(); i++)
	{
		pathStream << (*i)->getRoadSegment()->getSegmentAimsunId() << "-" << (*i)->getStatsNumberInSegment() << "|";
	}
	pathStream << std::endl;
	Print() << pathStream.str();
}
std::string MesoPathMover::printPath(const Path &path, const Node *node){
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
	if(out.str().size())
	{
		out << "\n";
	}
	return out.str();
}
