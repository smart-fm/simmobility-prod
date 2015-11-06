/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "KShortestPathImpl.hpp"

#include <list>
#include <utility>
#include <vector>
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "path/PathSetManager.hpp"
#include "path/Path.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"
#include "StreetDirectory.hpp"

using namespace sim_mob;

boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::instance;

sim_mob::K_ShortestPathImpl::K_ShortestPathImpl() : k(sim_mob::ConfigManager::GetInstance().FullConfig().getPathSetConf().kspLevel)
{
	const RoadNetwork* rn = RoadNetwork::getInstance();
	const std::map<unsigned int, TurningGroup *>& turningGroupsMap = rn->getMapOfIdvsTurningGroups();
	for(std::map<unsigned int, TurningGroup *>::const_iterator tgIt=turningGroupsMap.begin(); tgIt!=turningGroupsMap.end(); tgIt++)
	{
		const TurningGroup* tg = tgIt->second;
	}
}

sim_mob::K_ShortestPathImpl::~K_ShortestPathImpl()
{
}

boost::shared_ptr<K_ShortestPathImpl> sim_mob::K_ShortestPathImpl::getInstance()
{
	if(!instance)
	{
		instance.reset(new K_ShortestPathImpl());
	}
	return instance;
}

/// a structure to store paths in order of their length
class BType
{
private:
	typedef std::list<std::vector<sim_mob::WayPoint> >::iterator PathIt;
	typedef std::pair<double, PathIt> LengthPathIteratorPair;

	struct comp
	{
		bool operator()(const LengthPathIteratorPair& lhs, const LengthPathIteratorPair& rhs) const
		{
			return lhs.first < rhs.first;
		}
	};

	std::list<std::vector<sim_mob::WayPoint> > paths;
	std::vector<LengthPathIteratorPair> keys;

public:
	BType()
	{
		clear();
	}

	bool insert(double length, std::vector<sim_mob::WayPoint> &path)
	{
		{
			//this block is a modification to Yen's algorithm to discard the duplicates in the candidate paths' list
			//todo: optimize this duplicate search
			PathIt it = std::find(paths.begin(), paths.end(), path);
			if(it != paths.end())
			{
				return false;
			}
		}
		PathIt it = paths.insert(paths.end(),path);
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
			throw std::runtime_error("Empty K-Shortest path intermediary collections, check before fetch");
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
	std::vector<const sim_mob::Link*> BL_VECTOR(std::set<const sim_mob::Link*> &input)
	{
		std::vector<const sim_mob::Link*> output;
		std::copy(input.begin(), input.end(), std::back_inserter(output));
		return output;
	}
}
/**
 * This method attempt follows He's pseudocode. For comfort of future readers, the namings are exactly same as the document
 */
int sim_mob::K_ShortestPathImpl::getKShortestPaths(const sim_mob::Node *from, const sim_mob::Node *to, std::vector< std::vector<sim_mob::WayPoint> > &res)
{
	StreetDirectory& stdir = StreetDirectory::Instance();
	std::vector< std::vector<sim_mob::WayPoint> > &A = res;//just renaming the variable
	std::vector<const Link*> bl;//black list
	std::set<const Link*> blSet;
	//	STEP 1: find path A1
	//			Apply any shortest path algorithm (e.g., Dijkstra's) to find the shortest path from O to D,	given link weights W and network graph G.
	std::vector<sim_mob::WayPoint> temp = stdir.SearchShortestDrivingPath(*from, *to, bl);
	std::vector<sim_mob::WayPoint> A0;//actually A1 (in the pseudo code)
	sim_mob::SinglePath::filterOutNodes(temp, A0);
	//sanity check
	if(A0.empty())
	{
		return 0;
	}

	// Store it in path list A as A1
	A.push_back(A0);

	// Set path list B = []
	BType B ;

	//STEP 2: find paths AK , where K = 2, 3, ..., k.
	int K = 1; //k = 2
	while(true)
	{
		// Set path list C = A.
		std::vector< std::vector<sim_mob::WayPoint> > C = A;
		// Set RootPath = [].
		std::vector<sim_mob::WayPoint> rootPath;
		// For i = 0 to size(A,k-1)-1:
		for(int i = 0; i < A[K-1].size(); i++)
		{
			// nextRootPathLink = A,k-1 [i]
			sim_mob::WayPoint nextRootPathLink = A[K-1][i];
			const sim_mob::Node *spurNode = nextRootPathLink.link->getFromNode();

			// Find links whose EndNode = SpurNode, and block them.
			getUpstreamLinks(spurNode, blSet); //find and store in the blacklist
			//if(nextRootPathLink.roadSegment_->getStart() == nextRootPathLink.roadSegment_->getLink()->getStart())//this line('if' condition only, not the if block) is an optimization to HE's pseudo code to bypass uninodes
			{
				//	For each path Cj in path list C:
				for(int j = 0; j < C.size(); j++)
				{
					//Block link Cj[i].
					blSet.insert(C[j][i].link);
				}
				//Find shortest path from SpurNode to D, and store it as SpurPath.
				temp.clear();
				temp = stdir.SearchShortestDrivingPath(*spurNode, *to, BL_VECTOR(blSet));
				std::vector<sim_mob::WayPoint> spurPath;
				sim_mob::SinglePath::filterOutNodes(temp, spurPath);
				std::vector<sim_mob::WayPoint> totalPath;
				if(validatePath(rootPath, spurPath))
				{
					//	Set TotalPath = RootPath + SpurPath.
					totalPath.insert(totalPath.end(), rootPath.begin(),rootPath.end());
					totalPath.insert(totalPath.end(), spurPath.begin(), spurPath.end());
					//	Add TotalPath to path list B.
					B.insert(sim_mob::generateSinglePathLength(totalPath), totalPath);
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
			rootPath.push_back(nextRootPathLink);
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
		blSet.clear();
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

void sim_mob::K_ShortestPathImpl::getUpstreamLinks(const Node *spurNode, std::set<const Link*>& upLinks) const
{
	std::set<const RoadSegment*> endSegments;
	//step-1: get ALL the segments, regardless of spurnode
	const sim_mob::UniNode * uNode = dynamic_cast<const sim_mob::UniNode*>(spurNode);
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
		const Node * mNode = dynamic_cast <const Node*>(spurNode);
		const std::set<sim_mob::RoadSegment*>& src = mNode->getRoadSegments();
		for(std::set<RoadSegment*>::const_iterator it = src.begin(); it != src.end(); it++)
		{
			endSegments.insert(*it);
		}
	}
	//step-2: filter out the segments for whom SpurNode is a start node
	for (std::set<const RoadSegment*>::iterator it(endSegments.begin()) ; it != endSegments.end(); ) {
	  if ((*it)->getEnd() !=  spurNode){
	    endSegments.erase(it++);
	  } else {
	    ++it;
	  }
	}
	//step-3: add the results to blacklist
	for (std::set<const RoadSegment*>::iterator it(endSegments.begin()) ; it != endSegments.end(); it++) {
		blackList.insert(*it);
	}
}

bool sim_mob::K_ShortestPathImpl::validatePath(const std::vector<sim_mob::WayPoint> &rootPath, const std::vector<sim_mob::WayPoint> &spurPath)
{
	if(spurPath.empty())
	{
		return false;
	}

	if(!rootPath.empty())
	{
		if(!sim_mob::MovementFacet::isConnectedToNextSeg(rootPath.rbegin()->roadSegment, spurPath.begin()->roadSegment))
		{
			return false;
		}
	}
	return true;
}
