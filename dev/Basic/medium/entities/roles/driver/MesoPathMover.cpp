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

void MesoPathMover::buildSegStatsPath(const std::vector<WayPoint> &pathWayPts,
                                      std::vector<const SegmentStats *>& pathSegStats)
{
	for(auto it = pathWayPts.begin(); it != pathWayPts.end(); ++it)
	{
		if((*it).type == WayPoint::LINK)
		{
			const Link *link = (*it).link;

			//Get the conflux responsible for the link
			const Node *toNode = link->getToNode();
			Conflux *nodeConflux = Conflux::getConfluxFromNode(toNode);

			//Convert all segments in the links to segment stats
			auto rdSegments = link->getRoadSegments();

			for(auto itSeg = rdSegments.begin(); itSeg != rdSegments.end(); ++itSeg)
			{
				auto segStats = nodeConflux->findSegStats(*itSeg);
				pathSegStats.insert(pathSegStats.end(), segStats.begin(), segStats.end());
			}
		}
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

#ifndef NDEBUG
	if(currSegStatIt == path.end())
	{
		throw std::runtime_error("Current segment not found in new path");
	}
#endif
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

void MesoPathMover::erasePathAfterCurrenrLink()
{
	const SegmentStats* currSegStat = getCurrSegStats();
	const Link *currSegmentParentLink = currSegStat->getRoadSegment()->getParentLink();
	std::vector<const SegmentStats*>::iterator itr = std::find(path.begin(),path.end(),currSegStat);
	if(itr != path.end())
	{
		itr++;
		while(itr != path.end() && (*itr)->getRoadSegment()->getParentLink() == currSegmentParentLink)
		{
			itr++;
		}

		if(itr != path.end())
		{
			path.erase(itr,path.end());
		}
	}
}

void MesoPathMover::appendRoute(std::vector<WayPoint> &routeToTaxiStand)
{
	std::vector<WayPoint>::iterator itr = routeToTaxiStand.begin();
	while(itr != routeToTaxiStand.end())
	{
		const Link *link = (*itr).link;
		const std::vector<RoadSegment*>& roadSegments = link->getRoadSegments();
		std::vector<RoadSegment*>::const_iterator segItr = roadSegments.begin();
		const Node *linkToNode = link->getToNode();
		Conflux *conflux = Conflux::getConfluxFromNode(linkToNode);
		while(segItr != roadSegments.end())
		{
			const std::vector<SegmentStats*>&  segStats = conflux->findSegStats((*segItr));
			path.insert(path.end(),segStats.begin(),segStats.end());
			segItr++;
		}
		itr++;
	}
}

void MesoPathMover::addPathFromCurrentSegmentToEndNodeOfLink()
{
	const SegmentStats *currStats = getCurrSegStats();
	const RoadSegment *roadSegment = currStats->getRoadSegment();
	std::vector<const SegmentStats*>::iterator itr = std::find(path.begin(),path.end(),currStats);
	path.erase(itr+1,path.end());
	const Link *parentLink = currStats->getRoadSegment()->getParentLink();
	const Node *toNode = parentLink->getToNode();
	Conflux * conflux = Conflux::getConfluxFromNode(toNode);
	const std::vector<RoadSegment*>& roadSegments =  parentLink->getRoadSegments();
	std::vector<RoadSegment*>::const_iterator segItr = std::find(roadSegments.begin(),roadSegments.end(),roadSegment);
	++segItr;
	while(segItr != roadSegments.end())
	{
		const std::vector<SegmentStats*> &segStats = conflux->findSegStats(*segItr);
		path.insert(path.end(),segStats.begin(),segStats.end());
		segItr++;
	}
}
void MesoPathMover::appendSegmentStats(const std::vector<RoadSegment*>& roadSegments,Conflux *conflux)
{
	std::vector<RoadSegment*>::const_iterator itr = roadSegments.begin();
	while(itr != roadSegments.end())
	{
		const std::vector<SegmentStats*>& segStats = conflux->findSegStats(*itr);
		path.insert(path.end(),segStats.begin(),segStats.end());
		itr++;
	}
}

void MesoPathMover::eraseFullPath()
{
	path.erase(path.begin(),path.end());
}

const SegmentStats* MesoPathMover::getFirstSegStatsInNextLink(const SegmentStats* segStats) const
{
	if (!segStats || currSegStatIt == path.end())
	{
		return nullptr;
	}

	Path::iterator it = currSegStatIt;
	int x =0 ;
	for (; it != path.end(); it++) // locate segStats in downstream path
	{
		if ((*it) == segStats)
		{
			break;
		}
		x++;
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
	bool retVal = false;

	if (currSegStatIt == path.end()-1)
	{
		retVal = true;
	}

	return retVal;
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

std::string MesoPathMover::printPath() const
{
	std::stringstream pathStream;
	pathStream << "SegmentStats path: ";
	for (Path::const_iterator i = path.begin(); i != path.end(); i++)
	{
		pathStream << (*i)->getRoadSegment()->getRoadSegmentId() << "-" << (*i)->getStatsNumberInSegment() << "|";
	}
	pathStream << std::endl;
	Print() << pathStream.str();
	return pathStream.str();
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

void MesoPathMover::initDriverPathTracking()
{
       driverPathTracker.setStartSegmentIterator(currSegStatIt);
       driverPathTracker.startDistToEndSegment = distToSegmentEnd;


//#ifndef NDEBUG
//		consistencyChecks();
//#endif


//#ifndef NDEBUG
//       //aa: just to see if he pointer we set is wrong
//       std::cout<<(*currSegStatIt)->getLength();
//#endif
}

void MesoPathMover::finalizeDriverPathTracking()
{
       driverPathTracker.setEndSegmentIterator(currSegStatIt);
       driverPathTracker.endDistToEndSegment = distToSegmentEnd;

//#ifndef NDEBUG
//		consistencyChecks();
//#endif



}

//double MesoPathMover::getDistanceCovered() const
//{
//       double distance = 0.0;
//
//       if (driverPathTracker.getStartSegmentIterator() == driverPathTracker.getEndSegmentIterator())
//       {
//               distance = std::max(driverPathTracker.startDistToEndSegment - driverPathTracker.endDistToEndSegment,0.0);
//       }
//       else
//       {
//               for (Path::iterator iter = driverPathTracker.getStartSegmentIterator(); (iter != path.end() && iter != driverPathTracker.getEndSegmentIterator()+1 ); ++iter)
//               {
//                       if (iter == driverPathTracker.getStartSegmentIterator())
//                       {
//                               distance += driverPathTracker.startDistToEndSegment;
//                       }
//                       else if (iter == driverPathTracker.getEndSegmentIterator())
//                       {
//                               distance += std::max(std::abs( (*iter)->getLength()) - driverPathTracker.endDistToEndSegment,0.0);
//                       }
//                       else
//                       {
//                    	   if (*(iter) == NULL )
//                    	   {
//                    		   std::stringstream ss;
//                    		   ss << "*iter is null"; throw std::runtime_error(ss.str() );
//                    	   }
//                    	   else
//                    	   {
//                               distance += std::abs((*iter)->getLength());
//                    	   }
//                       }
//               }
//       }
//#ifndef NDEBUG
//		consistencyChecks();
//#endif
//
//
//
//       return distance;
//}


double MesoPathMover::getDistanceCovered() const
{
	double distance = 0.0;

	if (driverPathTracker.getStartSegmentIterator() == driverPathTracker.getEndSegmentIterator())
	{
		distance = std::max(driverPathTracker.startDistToEndSegment - driverPathTracker.endDistToEndSegment,0.0);
	}
	else
	{
		for (Path::iterator iter = driverPathTracker.getStartSegmentIterator(); iter != driverPathTracker.getEndSegmentIterator(); ++iter)
		{
			if((iter+1 != path.end())|| iter!=path.end())
			{
				distance += 0.0;
				break;
			}
			else
			{

				if (iter == driverPathTracker.getStartSegmentIterator())
				{
					distance += driverPathTracker.startDistToEndSegment;
				}
				else if (iter == driverPathTracker.getEndSegmentIterator())
				{
					distance += std::max(std::abs( (*iter)->getLength()) - driverPathTracker.endDistToEndSegment,0.0);
				}
				else
				{
					distance += std::abs((*iter)->getLength());
				}
			}

		}

		distance += driverPathTracker.endDistToEndSegment;
	}
	return distance;
}
