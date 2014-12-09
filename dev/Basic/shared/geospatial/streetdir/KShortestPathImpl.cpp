/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "KShortestPathImpl.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/PathSetManager.hpp"

using namespace sim_mob;
boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::instance(new K_ShortestPathImpl());
sim_mob::K_ShortestPathImpl::K_ShortestPathImpl()
{
	init();
}
sim_mob::K_ShortestPathImpl::~K_ShortestPathImpl() {
	// TODO Auto-generated destructor stub
}
boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::getInstance()
{
	return instance;
}
void sim_mob::K_ShortestPathImpl::init()
{
	k=3;
	stdir = &StreetDirectory::instance();
}

int sim_mob::K_ShortestPathImpl::getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res)
{
	std::vector< std::vector<sim_mob::WayPoint> > &pathFound = res;//just renaming the variable
	std::vector<const RoadSegment*> bl;//black list
	std::vector<sim_mob::WayPoint> p = stdir->SearchShortestDrivingPath(
			stdir->DrivingVertex(*from),
			stdir->DrivingVertex(*to),
			bl);
	if(p.empty())
	{
		return 0;
	}
	pathFound.push_back(p);
	storeSegments(p);
	std::vector<sim_mob::WayPoint> rootPath;
//	int kk=0;
	std::map<std::string, std::vector<sim_mob::WayPoint> > pathIdMap; // store path ,key=segid_segid_...
	std::list< sim_mob::PathLength > sortList;
	while(true)
	{
		std::vector<sim_mob::WayPoint> pathPrevious = pathFound.back();
		std::vector< std::vector<sim_mob::WayPoint> > pathWayPoints;
		// blacklist is initiated for every iteration so that previously blocked links are restored.
		std::vector<const RoadSegment*> blacklist;
		// set path list pathWayPoints = pathFound.
		for(int i=0;i<pathFound.size();i++)
		{
			std::vector<sim_mob::WayPoint> path_inA = pathFound[i];
			pathWayPoints.push_back(path_inA);
		}
		for(int i=0;i<pathPrevious.size();i++)
		{
			if (pathPrevious[i].type_ == WayPoint::ROAD_SEGMENT) {
				const sim_mob::RoadSegment *nextRootPathLink = pathPrevious[i].roadSegment_;
				const sim_mob::Node *spur_node = nextRootPathLink->getStart();
				// block link pathWayPoints^j[i].
				std::vector< std::vector<sim_mob::WayPoint> > pathSegments;
				for(int j=0;j<pathWayPoints.size();j++)
				{
					std::vector<sim_mob::WayPoint> path_inC = pathWayPoints[j];
					if (i > path_inC.size() || path_inC.empty() )
					{
						continue;
					}
					if (path_inC[i].type_ == WayPoint::ROAD_SEGMENT) {
						const sim_mob::RoadSegment *blocklink = path_inC[i].roadSegment_;
						blacklist.push_back(blocklink);
						// compare nextRootPathLink with blocklink.
						if(nextRootPathLink == blocklink)
						{
							pathSegments.push_back(path_inC);
						}
					}
				}
				// block spurnode. <-- this is for the purpose of looplessness, but is it necessary in our case?
				// find spurpath from spurnode to destination.
				std::vector<sim_mob::WayPoint> p2 = stdir->SearchShortestDrivingPath(
										stdir->DrivingVertex(*spur_node),
										stdir->DrivingVertex(*to),
										blacklist);
				// make complete path.
				p2.insert(p2.begin(),rootPath.begin(),rootPath.end());
				// store rootPath+spurPath to pathIdMap
				// make id for p2
				std::string id = sim_mob::makeWaypointsetString(p2);
				std::map<std::string, std::vector<sim_mob::WayPoint> >::iterator it_id = pathIdMap.find(id);
				if(it_id == pathIdMap.end() ) // never see this path before
				{
					pathIdMap.insert(std::make_pair(id,p2));
					// calculate length
					double l_ = sim_mob::generateSinglePathLength(p2);
					sim_mob::PathLength pl;
					pl.length  = l_;
					pl.path = p2;
					//store p2 in list to sort
					sortList.push_back(pl);
				} // end it_id
				// update path list pathWayPoints.
				pathWayPoints.clear();
				for(int j=0;j<pathSegments.size();j++)
				{
					std::vector<sim_mob::WayPoint> path_inD = pathSegments[j];
					pathWayPoints.push_back(path_inD);
				}
				// update rootpath.
				rootPath.push_back(pathPrevious[i]);
			} // end type_
		} // end for
		if(sortList.size()>0)
		{
			// sort list
			sortList.sort(PathLengthComparator());
			// store pathIdMap[0] to pathFound
			// get lowest cost path and push to pathFound
			sim_mob::PathLength pl_ = *(sortList.begin());
			pathFound.push_back(pl_.path);
			// remove from sortlist
			sortList.pop_front();
			rootPath.clear();
		}
		else
		{
			// if path list pathIdMap is empty.
			break;
		}
		// if pathFound.size = k, return
		if(pathFound.size()==k)
		{
			break;
		}
	} // end while
	return pathFound.size();
}
void sim_mob::K_ShortestPathImpl::storeSegments(std::vector<sim_mob::WayPoint> path)
{
	for(int i=0;i<path.size();++i)
	{
		if (path[i].type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment *roadSeg = path[i].roadSegment_;
			A_Segments.insert(std::make_pair(roadSeg->originalDB_ID.getLogItem(),roadSeg));
		}
	}
}
