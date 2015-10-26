/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "KShortestPathImpl.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "path/PathSetManager.hpp"
#include "path/Path.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"
#include<vector>

using namespace sim_mob;
boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::instance;
sim_mob::K_ShortestPathImpl::K_ShortestPathImpl()
{
	init();
}
sim_mob::K_ShortestPathImpl::~K_ShortestPathImpl() {
	// TODO Auto-generated destructor stub
}
boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::getInstance()
{
	if(!instance)
	{
		instance.reset(new K_ShortestPathImpl());
	}
	return instance;
}
void sim_mob::K_ShortestPathImpl::init()
{
	k = sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().kspLevel;
	stdir = &StreetDirectory::Instance();
}

/// a structure to store paths in order of their length
class BType
{
	typedef std::list<std::vector<sim_mob::WayPoint> >::iterator pathIt;
	typedef std::pair<double, pathIt> Pair;
	struct comp{
		bool operator()(const Pair& lhs,const Pair& rhs) const
		{
			return lhs.first < rhs.first;
		}
	};
	std::list<std::vector<sim_mob::WayPoint> > paths;
	std::vector<Pair> keys;

public:
	BType(){
		clear();
	}

	bool insert(double length, std::vector<sim_mob::WayPoint> & path)
	{
		{
			//this block is a modification to Yen's algorithm to discard the duplicates in the candidate paths' list
			//todo: optimize this duplicate search
			pathIt it = std::find(paths.begin(), paths.end(), path);
			if(it != paths.end())
			{
				return false;
			}
		}
		pathIt it = paths.insert(paths.end(),path);
		keys.push_back(std::make_pair(length, it));
		return true;
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
		std::sort(keys.begin(), keys.end(),comp());
	}

	const std::vector<sim_mob::WayPoint>& getBegin()
	{
		if(empty() || !size())
		{
			throw std::runtime_error("Empty K-Shortest Path intermediary Collections, Check before Fetch");
		}
		return *(keys.begin()->second);
	}

	void eraseBegin()
	{
		paths.erase(keys.begin()->second);
		keys.erase(keys.begin());
	}

};

namespace
{
	std::vector<const sim_mob::RoadSegment*> BL_VECTOR(std::set<const sim_mob::RoadSegment*> &input)
	{
		std::vector<const sim_mob::RoadSegment*> output;
		std::copy(input.begin(), input.end(), std::back_inserter(output));
		return output;
	}
}
/**
 * This method attempt follows He's pseudocode. For comfort of future readers, the namings are exactly same as the document
 */
int sim_mob::K_ShortestPathImpl::getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res)
{
	std::vector< std::vector<sim_mob::WayPoint> > &A = res;//just renaming the variable
	std::vector<const RoadSegment*> bl = std::vector<const RoadSegment*>();//black list
	std::set<const RoadSegment*> BL;
	//	STEP 1: find path A1
	//			Apply any shortest path algorithm (e.g., Dijkstra's) to find the shortest path from O to D,	given link weights W and network graph G.
	//std::vector<sim_mob::WayPoint> temp = stdir->SearchShortestDrivingPath(*from, *to, bl);
	std::vector<sim_mob::WayPoint> temp;
	std::vector<sim_mob::WayPoint> A0;//actually A1 (in the pseudo code)
	sim_mob::SinglePath::filterOutNodes(temp,A0);
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
		//		Set path list C = A.
		std::vector< std::vector<sim_mob::WayPoint> > C = A;
		//		Set RootPath = [].
		std::vector<sim_mob::WayPoint> RootPath = std::vector<sim_mob::WayPoint>();
		//		For i = 0 to size(A,k-1)-1:
		for(int i = 0; i < A[K-1].size(); i++)
		{
			//	nextRootPathLink = A,k-1 [i]
			sim_mob::WayPoint nextRootPathLink = A[K-1][i];
			const sim_mob::Node *SpurNode = nextRootPathLink.roadSegment->getParentLink()->getFromNode();
			//	Find links whose EndNode = SpurNode, and block them.
			getEndSegments(SpurNode,BL);//find and store in the blacklist
			//if(nextRootPathLink.roadSegment_->getStart() == nextRootPathLink.roadSegment_->getLink()->getStart())//this line('if' condition only, not the if block) is an optimization to HE's pseudo code to bypass uninodes
			{
				//	For each path Cj in path list C:
				for(int j = 0; j < C.size(); j++)
				{
					//Block link Cj[i].
					BL.insert(C[j][i].roadSegment);
				}
				//Find shortest path from SpurNode to D, and store it as SpurPath.
				std::vector<sim_mob::WayPoint> SpurPath,temp;
				//temp = stdir->SearchShortestDrivingPath(*SpurNode, *to, BL_VECTOR(BL));
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
		const std::vector<sim_mob::WayPoint> & B0 = B.getBegin();

		A.push_back(B0);
		B.eraseBegin();
		//	Restore blocked links.
		BL.clear();
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
	//std::cout << "Created " << A.size() << " K shortest paths\n";
	return A.size();
}

void sim_mob::K_ShortestPathImpl::getEndSegments(const Node *SpurNode,std::set<const RoadSegment*>& blackList)
{/*
	std::set<const RoadSegment*> endSegments;
	//step-1: get ALL the segments, regardless of spurnode
	const sim_mob::UniNode * uNode = dynamic_cast<const sim_mob::UniNode*>(SpurNode);
	if(uNode)
	{
		const std::vector<const RoadSegment*>& src = uNode->getRoadSegments();
		for(std::vector<const RoadSegment*>::const_iterator it = src.begin(); it != src.end(); it++)
		{
			endSegments.insert(*it);
		}
	}
	else
	{
		const Node * mNode = dynamic_cast <const Node*>(SpurNode);
		const std::set<sim_mob::RoadSegment*>& src = mNode->getRoadSegments();
		for(std::set<RoadSegment*>::const_iterator it = src.begin(); it != src.end(); it++)
		{
			endSegments.insert(*it);
		}
	}
	//step-2: filter out the segments for whom SpurNode is a start node
	for (std::set<const RoadSegment*>::iterator it(endSegments.begin()) ; it != endSegments.end(); ) {
	  if ((*it)->getEnd() !=  SpurNode){
	    endSegments.erase(it++);
	  } else {
	    ++it;
	  }
	}
	//step-3: add the results to blacklist
	for (std::set<const RoadSegment*>::iterator it(endSegments.begin()) ; it != endSegments.end(); it++) {
		blackList.insert(*it);
	}*/
}
void sim_mob::K_ShortestPathImpl::storeSegments(std::vector<sim_mob::WayPoint> path)
{
	for(int i=0;i<path.size();++i)
	{
		if (path[i].type == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment *roadSeg = path[i].roadSegment;
			//A_Segments.insert(std::make_pair(roadSeg->getRoadSegmentId(),roadSeg));
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
		if(!sim_mob::MovementFacet::isConnectedToNextSeg(RootPath.rbegin()->roadSegment, SpurPath.begin()->roadSegment))
		{
			return false;
		}
	}
	return true;
}
