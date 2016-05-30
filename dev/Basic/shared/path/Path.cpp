#include "Path.hpp"

#include <boost/foreach.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <cmath>
#include <set>
#include "Common.hpp"
#include <sstream>
#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/TurningGroup.hpp"
#include "geospatial/network/TurningPath.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "logging/Log.hpp"
#include "PathSetParam.hpp"
#include "util/Profiler.hpp"
#include "util/Utils.hpp"

namespace
{
//sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");

double pathCostArray[] = { 0.77, 0.87, 0.98, 1.08, 1.16, 1.23, 1.29, 1.33, 1.37, 1.41, 1.45, 1.49, 1.53, 1.57, 1.61, 1.65, 1.69, 1.72, 1.75, 1.78, 1.81, 1.83,
		1.85, 1.87, 1.88, 1.89, 1.90, 1.91, 1.92, 1.93, 1.94, 1.95, 1.96, 1.97, 1.98, 1.99, 2.00, 2.01, 2.02 };
}

sim_mob::SinglePath::SinglePath() :
		purpose(work), utility(0.0), pathSize(0.0), travelCost(0.0), partialUtility(0.0), signalNumber(0.0), rightTurnNumber(0.0), length(0.0), travelTime(0.0), highWayDistance(
				0.0), validPath(true), minTravelTime(0), minDistance(0), minSignals(0), minRightTurns(0), maxHighWayUsage(0), shortestPath(0),
				path(std::vector<sim_mob::WayPoint>()), isNeedSave2DB(false)
{
}

sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id), utility(source.utility), pathSize(source.pathSize), travelCost(source.travelCost), validPath(source.validPath), signalNumber(
				source.signalNumber), rightTurnNumber(source.rightTurnNumber), length(source.length), travelTime(source.travelTime), pathSetId(
				source.pathSetId), highWayDistance(source.highWayDistance), minTravelTime(source.minTravelTime), minDistance(source.minDistance), minSignals(
				source.minSignals), minRightTurns(source.minRightTurns), maxHighWayUsage(source.maxHighWayUsage), shortestPath(source.shortestPath), partialUtility(
				source.partialUtility), scenario(source.scenario)
{
	isNeedSave2DB = false;

	purpose = sim_mob::work;
}

sim_mob::SinglePath::~SinglePath()
{
	clear();
}

bool sim_mob::SinglePath::includesLinks(const std::set<const sim_mob::Link*>& lnks) const
{
	if (lnks.empty())
	{
		return false;
	} //trivial case
	BOOST_FOREACH(const sim_mob::WayPoint& wp, this->path)
	{
		BOOST_FOREACH(const sim_mob::Link* lnk, lnks)
		{
			if (wp.link == lnk)
			{
				return true;
			}
		}
	}
	return false;
}

bool sim_mob::SinglePath::includesLink(const sim_mob::Link* lnk) const
{
	if (!lnk)
	{
		return false;
	}
	for(const auto& wp : this->path)
	{
		if (wp.link == lnk)
		{
			return true;
		}
	}
	return false;
}

///given a path of alternative nodes and Links, keep Links, loose the nodes
struct LinkFilter
{
	bool operator()(const sim_mob::WayPoint value)
	{
		return value.type == sim_mob::WayPoint::LINK;
	}
};

void sim_mob::SinglePath::filterOutNodes(std::vector<sim_mob::WayPoint>& input, std::vector<sim_mob::WayPoint>& output)
{
	typedef boost::filter_iterator<LinkFilter, std::vector<sim_mob::WayPoint>::iterator> FilterIterator;
	std::copy(FilterIterator(input.begin(), input.end()), FilterIterator(input.end(), input.end()), std::back_inserter(output));
}

void sim_mob::SinglePath::init(std::vector<sim_mob::WayPoint>& wpPath)
{
	//step-1 fill in the path
	filterOutNodes(wpPath, this->path);

	//sanity check
	if (this->path.empty())
	{
		std::stringstream err("");
		err << "empty path [OD:" << this->pathSetId << "][PATH:" << this->id << "][Graph Output type chain:\n";
		if (!wpPath.empty())
		{
			for (std::vector<sim_mob::WayPoint>::iterator it = wpPath.begin(); it != wpPath.end(); it++)
			{
				err << "[" << it->type << "," << it->node << "],";
			}
		}
		std::cerr << "[" << this->pathSetId << "] ERROR,IGNORED PATH:\n" << err.str() << std::endl;
	}

	//right/left turn
	sim_mob::countRightTurnsAndSignals(this);
	//highway distance
	highWayDistance = sim_mob::calculateHighWayDistance(this);
	//length
	length = sim_mob::generatePathLength(path);
	//default travel time
	travelTime = sim_mob::calculateSinglePathDefaultTT(path);
}

void sim_mob::SinglePath::clear()
{
	path.clear();
//	shortestSegPath.clear();
	id = "";
	pathSetId = "";
	utility = 0.0;
	pathSize = 0.0;
	travelCost = 0.0;
	signalNumber = 0.0;
	rightTurnNumber = 0.0;
	length = 0.0;
	travelTime = 0.0;
	highWayDistance = 0.0;
	minTravelTime = 0;
	minDistance = 0;
	minSignals = 0;
	minRightTurns = 0;
	maxHighWayUsage = 0;
}

sim_mob::PathSet::~PathSet()
{
	//logger << "[DELET PATHSET " << id << "] [" << pathChoices.size() << "  SINGLEPATH]" << std::endl;
	BOOST_FOREACH(sim_mob::SinglePath*sp,pathChoices)
	{
		safe_delete_item(sp);
	}
}

short sim_mob::PathSet::addOrDeleteSinglePath(sim_mob::SinglePath* s)
{
	if(s->id.empty()) { return 0; }
	if (!s || s->path.empty()) { return 0; }
	if (s->path.begin()->link->getFromNodeId() != subTrip.origin.node->getNodeId())
	{
		std::cerr << s->scenario << " path begins with " << s->path.begin()->link->getFromNodeId() << " while pathset begins with " << subTrip.origin.node->getNodeId() << std::endl;
		throw std::runtime_error("Mismatch");
	}

	bool res = false;

	{
		boost::unique_lock<boost::shared_mutex> lock(pathChoicesMutex);
		res = pathChoices.insert(s).second;
	}

	if (!res)
	{
		safe_delete_item(s);
		return 0;
	}
	return 1;
}

double sim_mob::calculateHighWayDistance(sim_mob::SinglePath *sp)
{
	if (!sp)
	{
		return 0.0;
	}
	double res = 0.0;
	for (int i = 0; i < sp->path.size(); ++i)
	{
		const sim_mob::Link* lnk = sp->path[i].link;
		if (lnk->getLinkType() == LinkType::LINK_TYPE_EXPRESSWAY)
		{
			res += lnk->getLength();
		}
	}
	return res; //meter
}

void sim_mob::countRightTurnsAndSignals(sim_mob::SinglePath *sp)
{
	if (sp->path.size() < 2) //trivial case
	{
		sp->rightTurnNumber = 0;
		sp->signalNumber = 0;
		return;
	}

	int rightTurnNumber = 0;
	int signalNumber = 0;
	std::vector<sim_mob::WayPoint>::iterator pathIt = sp->path.begin();
	++pathIt;
	for (std::vector<sim_mob::WayPoint>::iterator it = sp->path.begin(); it != sp->path.end(); ++it)
	{
		const Link* currentLink = it->link;
		const Link* targetLink = nullptr;
		if (pathIt != sp->path.end())
		{
			targetLink = pathIt->link;
		}
		else
		{
			break;
		} // already last segment

		const Node* linkEndNode = currentLink->getToNode();
		if (linkEndNode->getTrafficLightId() == 0)
		{
			signalNumber++;
		}

		//If there exists a turning path from the rightmost lane of the last segment of current link
		//to the target link, it is (probably) a right turn
		//NOTE: This logic to identify right turns is very weak. It can actually count left turns in some cases.
		//If anyone is able to explicitly specify right turns as part of the data or come up with a better logic, please do. ~Harish
		const TurningGroup* turnGrp = linkEndNode->getTurningGroup(currentLink->getLinkId(), targetLink->getLinkId());
		const RoadSegment* lastSegInCurrLink = currentLink->getRoadSegments().back();
		unsigned int numLanesInLastSeg = lastSegInCurrLink->getNoOfLanes();
		//the left most lane has index 0 and the right most lane has index (numLanesInLastSeg-1)
		const Lane* rightmostLaneInLastSeg = lastSegInCurrLink->getLane((numLanesInLastSeg - 1));
		const std::map<unsigned int, TurningPath *>* turnPathsFromRightmostLane = turnGrp->getTurningPaths(rightmostLaneInLastSeg->getLaneId());
		if (turnPathsFromRightmostLane)
		{
			rightTurnNumber++;
		}
		++pathIt;
	} //end for
	sp->rightTurnNumber = rightTurnNumber;
	sp->signalNumber = signalNumber;
}

double sim_mob::generatePathLength(const std::vector<sim_mob::WayPoint>& wp)
{
	double res = 0.0;
	for (std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		const sim_mob::Link* lnk = it->link;
		res += lnk->getLength();
	}
	return res; //meter
}

double sim_mob::calculateSinglePathDefaultTT(const std::vector<sim_mob::WayPoint>& wp)
{
	double res = 0.0;
	const TravelTimeManager* ttMgr = sim_mob::TravelTimeManager::getInstance();
	for (std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		const sim_mob::Link* lnk = it->link;
		res += ttMgr->getDefaultLinkTT(lnk);
	}
	return res; //secs
}

std::string sim_mob::makePathString(const std::vector<sim_mob::WayPoint>& wp)
{
	std::stringstream str("");
	std::set<unsigned int> pathLinks;
	for (std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		if (it->type == WayPoint::LINK)
		{
			unsigned int linkId = it->link->getLinkId();
			if(pathLinks.insert(linkId).second)
			{
				str << linkId << ",";
			}
			else
			{
				//there is a link which is traversed twice in the path => loop in path
				return std::string(); //return empty path
			}
		}
	}
	return str.str();
}

std::string sim_mob::makePT_PathString(const std::vector<PT_NetworkEdge> &path)
{
	std::stringstream str("");
	if (path.size() == 0)
	{
		Print()<<"warning: empty output makePT_PathString id"<<std::endl;
	}
	for (std::vector<PT_NetworkEdge>::const_iterator it = path.begin(); it != path.end(); it++)
	{
		str << it->getEdgeId() << ",";
	}
	if (str.str().size() < 1)
	{
		Print()<<"warning: empty output makePT_PathString id"<<std::endl;
	}
	return str.str();

}
std::string sim_mob::makePT_PathSetString(const std::vector<PT_NetworkEdge> &path)
{
	std::stringstream str("");
	if (path.size() == 0)
	{
		Print()<<"warning: empty output makePT_PathSetString id"<<std::endl;
	}
	str << path.front().getStartStop() << ",";
	str << path.back().getEndStop();
	if (str.str().size() < 1)
	{
		Print()<<"warning: empty output makePT_PathSetString id"<<std::endl;
	}
	return str.str();
}

sim_mob::PT_Path::PT_Path() :
		totalDistanceKms(0.0), totalCost(0.0), totalInVehicleTravelTimeSecs(0.0), totalWaitingTimeSecs(0.0), totalWalkingTimeSecs(0.0), totalNumberOfTransfers(
				0), minDistance(false), validPath(false), shortestPath(false), minInVehicleTravelTime(false), minNumberOfTransfers(false), minWalkingDistance(
				false), minTravelOnMRT(false), minTravelOnBus(false), pathSize(0.0), pathTravelTime(0.0), pathModesType(0)
{

}

sim_mob::PT_Path::PT_Path(const std::vector<PT_NetworkEdge> &path) :
		pathEdges(path), totalDistanceKms(0.0), totalCost(0.0), totalInVehicleTravelTimeSecs(0.0), totalWaitingTimeSecs(0.0), totalWalkingTimeSecs(0.0), totalNumberOfTransfers(
				0), minDistance(false), validPath(false), shortestPath(false), minInVehicleTravelTime(false), minNumberOfTransfers(false), minWalkingDistance(
				false), minTravelOnMRT(false), minTravelOnBus(false), pathSize(0.0), pathTravelTime(0.0), pathModesType(0)

{
	double totalBusMRTTravelDistance = 0.0;
	ptPathId = makePT_PathString(pathEdges);
	ptPathSetId = makePT_PathSetString(pathEdges);
	for (std::vector<PT_NetworkEdge>::const_iterator itEdge = pathEdges.begin(); itEdge != pathEdges.end(); itEdge++)
	{
		totalWaitingTimeSecs += itEdge->getWaitTimeSecs();
		totalInVehicleTravelTimeSecs += itEdge->getDayTransitTimeSecs();
		totalWalkingTimeSecs += itEdge->getWalkTimeSecs();
		pathTravelTime += itEdge->getLinkTravelTimeSecs();
		totalDistanceKms += itEdge->getDistKms();
		if (itEdge->getType() == sim_mob::BUS_EDGE || itEdge->getType() == sim_mob::TRAIN_EDGE)
		{
			totalBusMRTTravelDistance += itEdge->getDistKms();
			totalNumberOfTransfers++;
		}
	}

	totalCost = this->getTotalCostByDistance(totalBusMRTTravelDistance);
	if (totalNumberOfTransfers > 0)
	{
		totalNumberOfTransfers = totalNumberOfTransfers - 1;
	}
}

void sim_mob::PT_Path::updatePathEdges()
{
	int edgeId;
	std::stringstream ss(ptPathId);
	pathEdges.clear();
	PT_Network& ptNetwork = PT_Network::getInstance();
	bool hasBusTrip = false;
	bool hasTrainTrip = false;
	while (ss >> edgeId)
	{
		std::map<int, PT_NetworkEdge>::iterator edgeIt = ptNetwork.PT_NetworkEdgeMap.find(edgeId);
		if(edgeIt == ptNetwork.PT_NetworkEdgeMap.end())
		{
			char errBuf[1000];
			sprintf(errBuf, "PT path %s has invalid edge %d", ptPathId.c_str(), edgeId);
			throw std::runtime_error(std::string(errBuf));
		}

		pathEdges.push_back(edgeIt->second);
		hasBusTrip = (hasBusTrip || (edgeIt->second.getType() == sim_mob::PT_EdgeType::BUS_EDGE));
		hasTrainTrip = (hasTrainTrip || (edgeIt->second.getType() == sim_mob::PT_EdgeType::TRAIN_EDGE));

		if (ss.peek() == ',')
		{
			ss.ignore();
		}
	}

	if(hasBusTrip && hasTrainTrip) { pathModesType = 3; }
	else if(hasTrainTrip) { pathModesType = 2; }
	else if(hasBusTrip) { pathModesType = 1; }
	else { pathModesType = 0; }
}

sim_mob::PT_Path::~PT_Path()
{

}

sim_mob::PT_PathSet::PT_PathSet() :
		pathSet(std::set<PT_Path, cmp_path_vector>())
{
}

sim_mob::PT_PathSet::~PT_PathSet()
{
}

double sim_mob::PT_Path::getTotalCostByDistance(double totalDistance)
{
	if (totalDistance <= 3.2)
	{
		return pathCostArray[0];
	}
	else if (totalDistance > 40.2)
	{
		return pathCostArray[38];
	}
	else
	{
		return pathCostArray[(int) (floor(totalDistance - 3.2000000001)) + 1];
	}
}

void sim_mob::PT_PathSet::computeAndSetPathSize()
{
	for (std::set<PT_Path>::iterator itPath = pathSet.begin(); itPath != pathSet.end(); itPath++)
	{
		double pathSize = 0;
		double subPathSize = 0;  // Used to store the path-size component for each edge
		int subN = 0;            // Used to store the number of overlapped edge in choice set
		std::vector<PT_NetworkEdge> edges;
		edges = itPath->getPathEdges();
		double pathTravelTime = itPath->getPathTravelTime();
		for (std::vector<PT_NetworkEdge>::const_iterator itEdge = edges.begin(); itEdge != edges.end(); itEdge++)
		{
			if (pathTravelTime != 0)
			{
				subPathSize = itEdge->getLinkTravelTimeSecs() / pathTravelTime;
			}
			std::stringstream edgestring;
			edgestring << itEdge->getEdgeId() << ",";
			std::string edgeId = edgestring.str();
			for (std::set<PT_Path, cmp_path_vector>::iterator itPathComp = pathSet.begin(); itPathComp != pathSet.end(); itPathComp++)
			{
				if (itPathComp->getPtPathId().find(edgeId) != std::string::npos)
				{
					subN = subN + 1;
				}
			}
			if (subN != 0)
			{
				subPathSize = subPathSize / subN;
			}
			pathSize = pathSize + subPathSize;
		}
		itPath->setPathSize(pathSize);
	}
}

void sim_mob::PT_PathSet::checkPathFeasibilty()
{
	// Implementing path feasibility checks
	// Check 1 : Total Number of transfers < = 4
	// Check 2 : No two consecutive walking edges along the path
	// Check 3 : Doesn't walk back to any simMobility node from bus stop/ MRT station in the middle of the path
	// Check 4 : Total number of bus legs <= 4
	// Check 5 : The path must not involve more than 1 leg with the same bus line

	std::set<PT_Path>::iterator itPathComp = pathSet.begin();
	if (itPathComp == pathSet.end())
	{
		return;
	}
	std::string pathsetId = itPathComp->getPtPathSetId();
	bool incrementFlag;
	while (itPathComp != pathSet.end())
	{
		incrementFlag = false;
		std::set<PT_Path>::iterator tempitPath = itPathComp;
		// Check 1 : Total Number of transfers <= 4
		if(itPathComp->getTotalNumberOfTransfers() > 4)
		{
			// Infeasible path
			itPathComp++;
			pathSet.erase(tempitPath);
			continue;
		}
		sim_mob::PT_EdgeType prevEdgeType = sim_mob::UNKNOWN_EDGE;
		sim_mob::PT_EdgeType currentEdgeType = sim_mob::UNKNOWN_EDGE;
		int simMobilityNodeCount = 0; // simMobility Node counts along the path
		int busLegCount = 0; // Number of buslegs along the path
		std::vector<PT_NetworkEdge> edges;
		edges = itPathComp->getPathEdges();
		std::vector<std::string> buslinesInPath;
		for (std::vector<PT_NetworkEdge>::const_iterator itEdge = edges.begin(); itEdge != edges.end(); itEdge++)
		{
			// Check 2 : No two consecutive walking edges along the path
			currentEdgeType = itEdge->getType();
			if (currentEdgeType == sim_mob::WALK_EDGE && prevEdgeType == sim_mob::WALK_EDGE)
			{
				// Infeasible path
				itPathComp++;
				pathSet.erase(tempitPath);
				incrementFlag = true;
				break;
			}
			prevEdgeType = currentEdgeType;

			// Check 3 : Doesn't walk back to any simMobility node from bus stop/ MRT station in the middle of the path
			if (sim_mob::PT_Network::getInstance().PT_NetworkVertexMap[itEdge->getStartStop()].getStopType() == 0)
			{
				simMobilityNodeCount++;
			}
			if (sim_mob::PT_Network::getInstance().PT_NetworkVertexMap[itEdge->getEndStop()].getStopType() == 0)
			{
				simMobilityNodeCount++;
			}
			if (simMobilityNodeCount > 2)
			{
				// Infeasible path
				itPathComp++;
				pathSet.erase(tempitPath);
				incrementFlag = true;
				break;
			}
			if (itEdge->getType() == sim_mob::BUS_EDGE)
			{
				busLegCount++;
				// Check 4 : Total number of bus legs <= 4
				if (busLegCount > 4)
				{
					// Infeasible path
					itPathComp++;
					pathSet.erase(tempitPath);
					incrementFlag = true;
					break;
				}

				// Check 5 : The path must not involve more than 1 leg with the same bus line
				std::vector<std::string>::iterator busLineIt = std::find(buslinesInPath.begin(), buslinesInPath.end(), itEdge->getServiceLines());
				if(busLineIt == buslinesInPath.end())
				{
					buslinesInPath.push_back(itEdge->getServiceLines());
				}
				else
				{
					// Infeasible path
					itPathComp++;
					pathSet.erase(tempitPath);
					incrementFlag = true;
					break;
				}
			}
		}
		if (!incrementFlag)
		{
			itPathComp++;
		}
	}
	if (pathSet.empty())
	{
		Print() << pathsetId << " has no path left after feasibility check\n" << std::endl;
	}

}

bool sim_mob::cmp_path_vector::operator()(const PT_Path& lhs, const PT_Path& rhs) const
{
	return (lhs.getPtPathId() < rhs.getPtPathId());
}
