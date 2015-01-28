/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "KShortestPathImpl.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "path/PathSetManager.hpp"
#include "path/Path.hpp"
#include "entities/roles/RoleFacets.hpp"
#include<vector>

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

int sim_mob::K_ShortestPathImpl::getKShortestPaths_2(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res)
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
				if(p2.empty())
				{
					std::cout << "[" << from->getID() << "," << to->getID() << "]Warning-1 : "
							"stdir->SearchShortestDrivingPath[" << spur_node->getID()  << "[" <<
							to->getID() << "] returning no partial path" << std::endl;
				}
				// make complete path.
				p2.insert(p2.begin(),rootPath.begin(),rootPath.end());

				if(p2.empty())
				{
					std::cout << "[" << from->getID() << "," << to->getID() << "]Warning-2 : "
							"stdir->SearchShortestDrivingPath still returning no path" << std::endl;
				}
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

/// a structure to store paths in order of their length
class BType
{
	typedef std::list<std::vector<sim_mob::WayPoint> >::iterator pathIt;
	typedef std::pair<double, pathIt> Pair;
	struct comp{
		bool operator()(const Pair& lhs,const Pair& rhs) const
		{
//			return true;
			return lhs.first/**1000000*/ < rhs.first/**1000000*/;
		}
	};
	std::list<std::vector<sim_mob::WayPoint> > paths;
	std::vector<Pair> keys;
public:
	BType(){
		clear();
	}
	void insert(double length, std::vector<sim_mob::WayPoint> & path)
	{
		pathIt it = paths.insert(paths.end(),path);
		std::cout << "inserting " << length << " and vector size(" << path.size() << ")  " << std::endl;
		keys.push_back(std::make_pair(length, it));
	}
	bool empty()
	{
		return keys.empty();
	}
	int size()
	{
		return keys.size();
	}
	void clear()
	{
		paths.clear();
		keys.clear();
	}
	void sort()
	{

		//debug
		std::cout << keys.size() << "keys before sort : ";
		std::vector<Pair>::iterator it1(keys.begin()),itEnd1(keys.end());
		for(;it1 != itEnd1; it1++)
		{
			std::cout << (*it1).first << ",";
		}
		std::cout << "\n";
		//debug..
		std::sort(keys.begin(), keys.end(),comp());
		//debug
		std::cout << keys.size() << " keys sorted : ";
		std::vector<Pair>::iterator it(keys.begin()),itEnd(keys.end());
		for(;it != itEnd; it++)
		{
			std::cout << (*it).first << ",";
		}
		std::cout << "\n";
		//debug...
	}
	const std::vector<sim_mob::WayPoint>& get()
	{
		if(empty() || !size())
		{
			throw std::runtime_error("Empty K-Shortest Path intermediary Collections, Check before Fetch");
		}
		int i = 0;
		std::cout << "getting B.begin:" << keys.size() << std::endl;
		BOOST_FOREACH(Pair &pair, keys)
		{
			std::cout << i++ << std::endl;
			std::cout << pair.first << std::endl;
//			std::cout << pair.second->size() << std::endl;
		}
		std::cout << "getting B.begin first:" <<  (keys.begin()->first) << std::endl;
		std::cout << "getting B.begin second of size:" <<  (keys.begin()->second)->size() << std::endl;
		return *(keys.begin()->second);
	}
	void eraseBegin()
	{
		paths.erase(keys.begin()->second);
		keys.erase(keys.begin());

		//debug
		std::cout << "keys after erase begin : ";
		std::vector<Pair>::iterator it(keys.begin()),itEnd(keys.end());
		for(;it != itEnd; it++)
		{
			std::cout << (*it).first << ",";
		}
		std::cout << "\n";
	}

};
/**
 * This method attempt followes He's pseudocode. For comfort of future readers, the namings are exactly same as the document
 */
int sim_mob::K_ShortestPathImpl::getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res)
{
//	std::stringstream log("");
//	log << "ksp-" << from->getID() << "," << to->getID() ;
//	sim_mob::BasicLogger & logger = sim_mob::Logger::log(log.str());
	//logger << from->getID() << "," << to->getID() << "\n";
	std::vector< std::vector<sim_mob::WayPoint> > &A = res;//just renaming the variable
	std::vector<const RoadSegment*> bl;//black list
	//	STEP 1: find path A1
	//			Apply any shortest path algorithm (e.g., Dijkstra's) to find the shortest path from O to D,	given link weights W and network graph G.
	std::vector<sim_mob::WayPoint> temp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*from),stdir->DrivingVertex(*to),bl);
	std::vector<sim_mob::WayPoint> A0;//actually A1 (in the pseudo code)
	sim_mob::SinglePath::filterOutNodes(temp,A0);
	//logger << "shortest path with nodes : " << sim_mob::printWPpath(temp) << "\n\n";
	//logger << "shortest path segments : " << printWPpath(A0) << "\n\n";
	//sanity check
	if(A0.empty())
	{
		return 0;
	}
	//			Store it in path list A as A1
	A.push_back(A0);
	//			Set path list B = []
	BType B ;


	//STEP 2: find path A,k , where k = 2, 3, ..., K.
	int K = 1; //k = 2
	while(true)
	{
		//logger << "K=" << K << "\n";
		//		Set path list C = A.
		std::vector< std::vector<sim_mob::WayPoint> > C = A;
		//		Set RootPath = [].
		std::vector<sim_mob::WayPoint> RootPath = std::vector<sim_mob::WayPoint>();
		//		For i = 0 to size(A,k-1)-1:
		for(int i = 0; i < A[K-1].size(); i++)
		{
			//	nextRootPathLink = A,k-1 [i]
			sim_mob::WayPoint nextRootPathLink = A[K-1][i];
			const sim_mob::Node *SpurNode = nextRootPathLink.roadSegment_->getStart();
			////logger << "[spurnode:segment]:[" << SpurNode->getID() << "," << nextRootPathLink.roadSegment_->getId() << "]\n";
			//	Find links whose EndNode = SpurNode, and block them.
			getEndSegments(SpurNode,bl);//find and store in the blacklist
			//	For each path Cj in path list C:
			for(int j = 0; j < C.size(); j++)
			{
				//Block link Cj[i].
				bl.push_back(C[j][i].roadSegment_);
			}
			//Find shortest path from SpurNode to D, and store it as SpurPath.
			std::vector<sim_mob::WayPoint> SpurPath,temp;
			temp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*SpurNode),stdir->DrivingVertex(*to),bl);
			sim_mob::SinglePath::filterOutNodes(temp,SpurPath);
			std::vector<sim_mob::WayPoint> TotalPath = std::vector<sim_mob::WayPoint>();
			if(validatePath(RootPath, SpurPath))
			{
				//	Set TotalPath = RootPath + SpurPath.
				TotalPath.insert(TotalPath.end(), RootPath.begin(),RootPath.end());
				TotalPath.insert(TotalPath.end(), SpurPath.begin(), SpurPath.end());
				//	Add TotalPath to path list B.
				B.insert(sim_mob::generateSinglePathLength(TotalPath),TotalPath);

			}
			//	For each path Cj in path list C:
			for(int j = 0; j < C.size(); j++)
			{
				// If Cj[i] != nextRootPathLink:
				if(C[j][i] != nextRootPathLink)
				{
					//Delete Cj	from C.
					C.erase(C.begin() + j);
					j--;
				}
			}
			//	Add nextRootPathLink to RootPath
			RootPath.push_back(nextRootPathLink);
		}//for

		//	If B = []:
		if(B.empty())
		{
			//break
			break;
		}
		//	Sort path list B by path weight.
		B.sort();
		//	Add B[0] to path list A, and delete it from path list B.
		const std::vector<sim_mob::WayPoint> & B0 = B.get();

		//logger << "got B0:" << std::endl;
		//logger << "B0.size():" << B0.size() << " \npushing" << std::endl;
		A.push_back(B0);
		B.eraseBegin();
		//	Restore blocked links.
		bl.clear();
		// If size(A) < k:
		if(A.size() < k)//mind the lower/upper case of K!
		{
			// Repeat Step 2.
			K++;
			continue;
		}
		else
		{
			// else break
			break;
		}
	}//while
	//	The final path list A contains the K shortest paths (if possible) from O to D.
	return A.size();
}

void sim_mob::K_ShortestPathImpl::getEndSegments(const Node *SpurNode,std::vector<const RoadSegment*>& endSegments)
{
	//get ALL the segments, regardless of spurnode
	const sim_mob::UniNode * uNode = dynamic_cast<const sim_mob::UniNode*>(SpurNode);
	if(uNode)
	{
		const std::vector<const RoadSegment*>& src = uNode->getRoadSegments();
		std::copy(src.begin(), src.end(),std::back_inserter(endSegments));
	}
	else
	{
		const sim_mob::MultiNode * mNode = dynamic_cast <const sim_mob::MultiNode*>(SpurNode);
		const std::set<sim_mob::RoadSegment*>& src = mNode->getRoadSegments();
		std::copy(src.begin(), src.end(),std::back_inserter(endSegments));
	}
	//filter out the segments for whom SpurNode is a start node
	for (std::vector<const RoadSegment*>::iterator it(endSegments.begin()) ; it != endSegments.end(); ) {
	  if ((*it)->getEnd() !=  SpurNode){
	    it = endSegments.erase(it);
	  } else {
	    ++it;
	  }
	}
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

bool sim_mob::K_ShortestPathImpl::validatePath(const std::vector<sim_mob::WayPoint> &RootPath, const std::vector<sim_mob::WayPoint>&SpurPath)
{
	if(SpurPath.empty())
	{
		return false;
	}
	if(!RootPath.empty())
	{
		if(!sim_mob::MovementFacet::isConnectedToNextSeg(RootPath.rbegin()->roadSegment_, SpurPath.begin()->roadSegment_))
		{
			return false;
		}
	}
	return true;
}
