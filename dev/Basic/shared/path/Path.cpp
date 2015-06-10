#include "Path.hpp"

#include <boost/foreach.hpp>
#include <math.h>
#include <boost/iterator/filter_iterator.hpp>
#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/WayPoint.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "PathSetParam.hpp"
#include "util/Utils.hpp"

namespace{
sim_mob::BasicLogger & logger = sim_mob::Logger::log("pathset.log");

const double HIGHWAY_SPEED = 60.0; //kmph

double pathCostArray[] {0.77,0.87,0.98,1.08,1.16,1.23,1.29,1.33,1.37,1.41,1.45,1.49,1.53,
								   1.57,1.61,1.65,1.69,1.72,1.75,1.78,1.81,1.83,1.85,1.87,1.88,1.89,
								   1.90,1.91,1.92,1.93,1.94,1.95,1.96,1.97,1.98,1.99,2.00,2.01,2.02
								 };
}

sim_mob::SinglePath::SinglePath() : purpose(work),utility(0.0),pathSize(0.0),travelCost(0.0),partialUtility(0.0),
signalNumber(0.0),rightTurnNumber(0.0),length(0.0),travelTime(0.0),highWayDistance(0.0),valid_path(true),
isMinTravelTime(0),isMinDistance(0),isMinSignal(0),isMinRightTurn(0),isMaxHighWayUsage(0),
isShortestPath(0), index(-1),path(std::vector<sim_mob::WayPoint>()),isNeedSave2DB(false){
}

sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id),
		utility(source.utility),pathSize(source.pathSize),
		travelCost(source.travelCost),valid_path(source.valid_path),
		signalNumber(source.signalNumber),
		rightTurnNumber(source.rightTurnNumber),
		length(source.length),travelTime(source.travelTime),
		pathSetId(source.pathSetId),highWayDistance(source.highWayDistance),
		isMinTravelTime(source.isMinTravelTime),isMinDistance(source.isMinDistance),isMinSignal(source.isMinSignal),
		isMinRightTurn(source.isMinRightTurn),isMaxHighWayUsage(source.isMaxHighWayUsage),isShortestPath(source.isShortestPath),
		partialUtility(source.partialUtility), index(source.index), scenario(source.scenario)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;
}

sim_mob::SinglePath::~SinglePath() {
	clear();
}

bool sim_mob::SinglePath::includesRoadSegment(const std::set<const sim_mob::RoadSegment*>& segs) const
{
	if(segs.empty()) { return false; } //trivial case
	std::stringstream pathSegId(""), inputSegId("");
	BOOST_FOREACH(const sim_mob::WayPoint& wp, this->path)
	{
		pathSegId.str(std::string());
		pathSegId << wp.roadSegment_->getId();
		BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs)
		{
			inputSegId.str(std::string());
			inputSegId << seg->getId();
			if(pathSegId.str() == inputSegId.str() ) { return true; }
		}
	}
	return false;
}

///given a path of alternative nodes and segments, keep segments, loose the nodes
struct segFilter{
		bool operator()(const sim_mob::WayPoint value){
			return value.type_ == sim_mob::WayPoint::ROAD_SEGMENT;
		}
};
void sim_mob::SinglePath::filterOutNodes(std::vector<sim_mob::WayPoint>& input, std::vector<sim_mob::WayPoint>& output)
{
	typedef boost::filter_iterator<segFilter,std::vector<sim_mob::WayPoint>::iterator> FilterIterator;
	std::copy(FilterIterator(input.begin(), input.end()),FilterIterator(input.end(), input.end()),std::back_inserter(output));
}

void sim_mob::SinglePath::init(std::vector<sim_mob::WayPoint>& wpPools)
{
	//step-1 fill in the path
	filterOutNodes(wpPools, this->path);
	//sanity check
	if(this->path.empty())
	{
	   std::stringstream err("");
	   err << "empty path [OD:" << this->pathSetId << "][PATH:"  << this->id << "][Graph Oputpout type chain:\n";
		if(wpPools.size())
		{
			for(std::vector<sim_mob::WayPoint>::iterator it = wpPools.begin(); it != wpPools.end(); it++)
			{
				err << "[" << it->type_ << "," << it->node_ << "],";
			}
		}
	   std::cerr << "[" << this->pathSetId << "] ERROR,IGNORED PATH:\n" << err.str() << std::endl;
	}
	//step-1.5 fill in the linkPath
	{
		sim_mob::Link* currLink = nullptr;
		for(std::vector<sim_mob::WayPoint>::iterator it = path.begin(); it != path.end(); it++)
		{
			sim_mob::Link* link = it->roadSegment_->getLink();
			if(currLink != link)
			{
				linkPath.push_back(link);
				currLink = link;
			}
		}
	}
	//step-1.6 fill in the segSet
	{
		for(std::vector<sim_mob::WayPoint>::iterator it = path.begin(); it != path.end(); it++)
		{
			segSet.insert(it->roadSegment_);
		}
	}

	//right/left turn
	sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(this);
	//highway distance
	highWayDistance = sim_mob::calculateHighWayDistance(this);
	//length
	length = sim_mob::generateSinglePathLength(path);
	//default travel time
	travelTime = sim_mob::calculateSinglePathDefaultTT(path);
}

void sim_mob::SinglePath::clear()
{
	path.clear();
//	shortestSegPath.clear();
	id="";
	pathSetId="";
	utility = 0.0;
	pathSize = 0.0;
	travelCost=0.0;
	signalNumber=0.0;
	rightTurnNumber=0.0;
	length=0.0;
	travelTime=0.0;
	highWayDistance=0.0;
	isMinTravelTime=0;
	isMinDistance=0;
	isMinSignal=0;
	isMinRightTurn=0;
	isMaxHighWayUsage=0;
}
uint32_t sim_mob::SinglePath::getSize(){

	uint32_t sum = 0;
	sum += sizeof(WayPoint) * path.size(); // std::vector<sim_mob::WayPoint> shortestWayPointpath;
//	sum += sizeof(const RoadSegment*) * shortestSegPath.size(); // std::set<const RoadSegment*> shortestSegPath;
	sum += sizeof(boost::shared_ptr<sim_mob::PathSet>); // boost::shared_ptr<sim_mob::PathSet>pathSet; // parent
	sum += sizeof(const sim_mob::RoadSegment*); // const sim_mob::RoadSegment* excludeSeg; // can be null
	sum += sizeof(const sim_mob::Node *); // const sim_mob::Node *fromNode;
	sum += sizeof(const sim_mob::Node *); // const sim_mob::Node *toNode;

	sum += sizeof(double); // double highWayDistance;
	sum += sizeof(bool); // bool isMinTravelTime;
	sum += sizeof(bool); // bool isMinDistance;
	sum += sizeof(bool); // bool isMinSignal;
	sum += sizeof(bool); // bool isMinRightTurn;
	sum += sizeof(bool); // bool isMaxHighWayUsage;
	sum += sizeof(bool); // bool isShortestPath;

	sum += sizeof(bool); // bool isNeedSave2DB;
	sum += id.length(); // std::string id;   //id: seg1id_seg2id_seg3id
	sum += pathSetId.length(); // std::string pathset_id;
	sum += sizeof(double); // double utility;
	sum += sizeof(double); // double pathsize;
	sum += sizeof(double); // double travel_cost;
	sum += sizeof(int); // int signalNumber;
	sum += sizeof(int); // int rightTurnNumber;
	sum += scenario.length(); // std::string scenario;
	sum += sizeof(double); // double length;
	sum += sizeof(double); // double travle_time;
	sum += sizeof(sim_mob::TRIP_PURPOSE); // sim_mob::TRIP_PURPOSE purpose;
	logger << "SinglePath size bytes:" << sum << "\n" ;
	return sum;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sim_mob::PathSet::~PathSet()
{
	//logger << "[DELET PATHSET " << id << "] [" << pathChoices.size() << "  SINGLEPATH]" << std::endl;
	BOOST_FOREACH(sim_mob::SinglePath*sp,pathChoices)
	{
		safe_delete_item(sp);
	}
}

uint32_t sim_mob::PathSet::getSize(){
	uint32_t sum = 0;
		sum += sizeof(bool);// isInit;
		sum += sizeof(bool);//bool hasBestChoice;
		sum += sizeof(WayPoint) * (bestPath ? bestPath->size() : 0);//std::vector<sim_mob::WayPoint> bestWayPointpath;  //best choice
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *fromNode;
		sum += sizeof(const sim_mob::Node *);//const sim_mob::Node *toNode;
		sum += sizeof(SinglePath*);//SinglePath* oriPath;  // shortest path with all segments
		//std::map<std::string,sim_mob::SinglePath*> SinglePathPool;//unused so far
//		typedef std::map<std::string,sim_mob::SinglePath*>::value_type tt;
//		BOOST_FOREACH(tt & pair_,SinglePathPool)
//		{
//			sum += pair_.first.length();
//			sum += sizeof(pair_.second);
//		}
		//std::set<sim_mob::SinglePath*, sim_mob::SinglePath> pathChoices;
		sim_mob::SinglePath* sp;
		BOOST_FOREACH(sp,pathChoices)
		{
			uint32_t t = sp->getSize();
			sum += t;//real singlepath size
		}
		sum += sizeof(bool);//bool isNeedSave2DB;
		sum += sizeof(double);//double logsum;
		sum += sizeof(const sim_mob::SubTrip*);//const sim_mob::SubTrip* subTrip;
		sum += id.length();//std::string id;
//		sum += fromNodeId.length();//std::string fromNodeId;
//		sum += toNodeId.length();//std::string toNodeId;
		sum += excludedPaths.length();//std::string excludedPaths;
		sum += scenario.length();//std::string scenario;
		sum += sizeof(int);//int hasPath;
//		sum += sizeof(sim_mob::PathSetManager *);//PathSetManager *psMgr;
		logger << "pathset_cached_bytes :" << sum << "\n" ;
		return sum;
}

bool sim_mob::PathSet::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs)
{
	BOOST_FOREACH(sim_mob::SinglePath *sp, pathChoices)
	{
		BOOST_FOREACH(sim_mob::WayPoint &wp, sp->path)
		{
			BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs)
			{
				if(wp.roadSegment_ == seg)
				{
					return true;
				}
			}
		}
	}
	return false;
}


void sim_mob::PathSet::excludeRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs)
{
	std::set<sim_mob::SinglePath*>::iterator it(pathChoices.begin());
	for(; it != pathChoices.end();)
	{
		if((*it)->includesRoadSegment(segs))
		{
			pathChoices.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

short sim_mob::PathSet::addOrDeleteSinglePath(sim_mob::SinglePath* s)
{
	if(!s && s->path.empty())
	{
		return 0;
	}
	if(s->path.begin()->roadSegment_->getStart()->getID() != subTrip.fromLocation.node_->getID())
	{
		std::cerr << s->scenario << " path begins with " << s->path.begin()->roadSegment_->getStart()->getID() << " while pathset begins with " << subTrip.fromLocation.node_->getID() << std::endl;
		throw std::runtime_error("Mismatch");
	}

	bool res = false;
	{
		boost::unique_lock<boost::shared_mutex> lock(pathChoicesMutex);
		res = pathChoices.insert(s).second;
	}
	if(!res)
	{
		safe_delete_item(s);
		return 0;
	}
	return 1;
}


double sim_mob::calculateHighWayDistance(sim_mob::SinglePath *sp)
{
	double res=0;
	if(!sp) return 0.0;
	for(int i=0;i<sp->path.size();++i)
	{
		const sim_mob::RoadSegment* seg = sp->path[i].roadSegment_;
		if(seg->isHighway())
		{
			res += seg->getLength();
		}
	}
	return res/100.0; //cm -> meter
}

size_t sim_mob::getLaneIndex2(const sim_mob::Lane* l){
	if (l) {
		const sim_mob::RoadSegment* r = l->getRoadSegment();
		std::vector<sim_mob::Lane*>::const_iterator it( r->getLanes().begin()), itEnd(r->getLanes().end());
		for (size_t i = 0; it != itEnd; it++, i++) {
			if (*it == l) {
				return i;
			}
		}
	}
	return -1; //NOTE: This might not do what you expect! ~Seth
}

void sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp)
{
	if (sp->path.size() < 2)
	{
		sp->rightTurnNumber=0;
		sp->signalNumber=0;
		return;
	}

	int rightTurnNumber = 0;
	int signalNumber = 0;
	std::vector<sim_mob::WayPoint>::iterator pathIt = sp->path.begin();
	++pathIt;
	for(std::vector<sim_mob::WayPoint>::iterator it=sp->path.begin(); it!=sp->path.end(); ++it)
	{
		const RoadSegment* currentSeg = it->roadSegment_;
		const RoadSegment* targetSeg = NULL;
		if(pathIt != sp->path.end()) { targetSeg = pathIt->roadSegment_; }
		else { break; } // already last segment

		if(currentSeg->getEnd() == currentSeg->getLink()->getEnd()) // intersection
		{
			const sim_mob::MultiNode* linkEndNode = dynamic_cast<const sim_mob::MultiNode*> (currentSeg->getEnd());
			if(linkEndNode) // should always be true, but just double checking
			{
				if(linkEndNode->isSignalized()) { signalNumber++; }
				// get lane connector
				const std::set<sim_mob::LaneConnector*>& lcs = linkEndNode->getOutgoingLanes(currentSeg);
				for (std::set<LaneConnector*>::const_iterator it2 = lcs.begin(); it2 != lcs.end(); it2++) {
					if((*it2)->getLaneTo()->getRoadSegment() == targetSeg)
					{
						int laneIndex = sim_mob::getLaneIndex2((*it2)->getLaneFrom());
						if(laneIndex<2)//most left lane
						{
							rightTurnNumber++;
							break;
						}
					}//end if targetSeg
				}// end for lcs
			}// end if linkEndNode
			else { throw std::runtime_error("end of link is not a Multinode"); }
		}//end currEndNode
		++pathIt;
	}//end for
	sp->rightTurnNumber=rightTurnNumber;
	sp->signalNumber=signalNumber;
}

/**
 * Returns path length in meter
 * @param collection of road segments wrapped in waypoint
 * @return total length of the path in meter
 */
double sim_mob::generateSinglePathLength(const std::vector<sim_mob::WayPoint>& wp)// unit is meter
{
	double res = 0.0;
	for(std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		const sim_mob::RoadSegment* seg = it->roadSegment_;
		res += seg->getLength();
	}
	return res/100.0; //meter
}

double sim_mob::calculateSinglePathDefaultTT(const std::vector<sim_mob::WayPoint>& wp)
{
	double res = 0.0;
	for(std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		const sim_mob::RoadSegment* rs = it->roadSegment_;
		res += sim_mob::PathSetParam::getInstance()->getDefSegTT(rs);
	}
	return res; //hours
}


std::string sim_mob::makeWaypointsetString(const std::vector<sim_mob::WayPoint>& wp)
{
	std::stringstream str("");
	if(wp.size()==0)
	{
		sim_mob::Logger::log("pathset.log") << "warning: empty input for makeWaypointsetString" << std::endl;
	}

	for(std::vector<sim_mob::WayPoint>::const_iterator it = wp.begin(); it != wp.end(); it++)
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT)
		{
			str << it->roadSegment_->getId() << ",";
		} // if ROAD_SEGMENT
	}

	if(str.str().size()<1)
	{
		// when same f,t node, it happened
		sim_mob::Logger::log("pathset.log") << "warning: empty output makeWaypointsetString id" << std::endl;
	}

	return str.str();
}

std::string sim_mob::makePT_PathString(const std::vector<PT_NetworkEdge> &path)
{
	std::stringstream str("");
	if(path.size()==0)
	{
		std::cout<<"warning: empty output makePT_PathString id"<<std::endl;
	}
	for(std::vector<PT_NetworkEdge>::const_iterator it = path.begin();it!=path.end();it++)
	{
		str<<it->getEdgeId()<<",";
	}
	if(str.str().size()<1)
	{
		std::cout<<"warning: empty output makePT_PathString id"<<std::endl;
	}
	return str.str();

}
std::string sim_mob::makePT_PathSetString(const std::vector<PT_NetworkEdge> &path)
{
	std::stringstream str("");
	if(path.size()==0)
	{
		std::cout<<"warning: empty output makePT_PathSetString id"<<std::endl;
	}
	str<<path.front().getStartStop()<<",";
	str<<path.back().getEndStop();
	if(str.str().size()<1)
	{
		std::cout<<"warning: empty output makePT_PathSetString id"<<std::endl;
	}
	return str.str();
}

sim_mob::PT_Path::PT_Path() :
				totalDistanceKms(0.0),
				totalCost(0.0),
				totalInVehicleTravelTimeSecs(0.0),
				totalWaitingTimeSecs(0.0),
				totalWalkingTimeSecs(0.0),
				totalNumberOfTransfers(0),minDistance(false),validPath(false),shortestPath(false),
				minInVehicleTravelTime(false),minNumberOfTransfers(false),minWalkingDistance(false),
				minTravelOnMRT(false),minTravelOnBus(false),pathSize(0.0)
{

}

sim_mob::PT_Path::PT_Path (const std::vector<PT_NetworkEdge> &path) : pathEdges(path),
		totalDistanceKms(0.0),
		totalCost(0.0),
		totalInVehicleTravelTimeSecs(0.0),
		totalWaitingTimeSecs(0.0),
		totalWalkingTimeSecs(0.0),
		totalNumberOfTransfers(0),minDistance(false),validPath(false),shortestPath(false),
		minInVehicleTravelTime(false),minNumberOfTransfers(false),minWalkingDistance(false),
		minTravelOnMRT(false),minTravelOnBus(false),pathSize(0.0)

{
	double totalBusMRTTravelDistance=0.0;
	ptPathId=makePT_PathString(pathEdges);
	ptPathSetId=makePT_PathSetString(pathEdges);
	for(std::vector<PT_NetworkEdge>::const_iterator itEdge=pathEdges.begin();itEdge!=pathEdges.end();itEdge++)
	{
		totalWaitingTimeSecs+=itEdge->getWaitTimeSecs();
		totalInVehicleTravelTimeSecs+=itEdge->getDayTransitTimeSecs();
		totalWalkingTimeSecs+=itEdge->getWalkTimeSecs();
		pathTravelTime+=itEdge->getLinkTravelTimeSecs();
		totalDistanceKms+=itEdge->getDistKms();
		if(itEdge->getType()=="Bus" || itEdge->getType()=="RTS")
		{
			totalBusMRTTravelDistance+=itEdge->getDistKms();
	           	totalNumberOfTransfers++;
		}
	}
		
	totalCost=this->getTotalCostByDistance(totalBusMRTTravelDistance);
	if(totalNumberOfTransfers > 0)
	{
		totalNumberOfTransfers = totalNumberOfTransfers -1;
	}
}
void sim_mob::PT_Path::updatePathEdges()
{
	int edgeId;
	std::stringstream ss(ptPathId);
	pathEdges.clear();
	while (ss >> edgeId) {
		pathEdges.push_back(PT_Network::getInstance().PT_NetworkEdgeMap[edgeId]);
		if (ss.peek() == ','){
		   ss.ignore();
		}
	}
}
sim_mob::PT_Path::~PT_Path()
{

}
sim_mob::PT_PathSet::PT_PathSet():pathSet(std::set<PT_Path,cmp_path_vector>())
{}

sim_mob::PT_PathSet::~PT_PathSet()
{}

double sim_mob::PT_Path::getTotalCostByDistance(double totalDistance)
{
	if(totalDistance<=3.2)
	{
		return pathCostArray[0];
	}
	else if(totalDistance>40.2)
	{
		return pathCostArray[38];
	}
	else
	{
		return pathCostArray[(int)(floor(totalDistance-3.2000000001))+1];
	}
}


void sim_mob::PT_PathSet::computeAndSetPathSize()
{
	for(std::set<PT_Path>::iterator itPath =pathSet.begin();itPath!=pathSet.end();itPath++)
	{
		double pathSize=0;
		double subPathSize=0;  // Used to store the path-size component for each edge
		int subN=0;            // Used to store the number of overlapped edge in choice set
		std::vector<PT_NetworkEdge> edges;
		edges = itPath->getPathEdges();
		for(std::vector<PT_NetworkEdge>::const_iterator itEdge=edges.begin();itEdge!=edges.end();itEdge++)
		{
			double pathTravelTime = itPath->getPathTravelTime();
			if(pathTravelTime != 0)
			{
				subPathSize=itEdge->getLinkTravelTimeSecs()/pathTravelTime;
			}
			std::stringstream edgestring;
			edgestring<<itEdge->getEdgeId()<<",";
			std::string edgeId= edgestring.str();
			for(std::set<PT_Path,cmp_path_vector>::iterator itPathComp =pathSet.begin();itPathComp!=pathSet.end();itPathComp++)
			{
				if (itPathComp->getPtPathId().find(edgeId) != std::string::npos)
				{
					subN=subN+1;
				}
			}
			if(subN != 0)
			{
				subPathSize=subPathSize/subN;
			}
			pathSize=pathSize+subPathSize;
		}
		itPath->setPathSize(pathSize);
	}
}
bool sim_mob::cmp_path_vector::operator()(const PT_Path A, const PT_Path B) const {

	return A.getPtPathId() < B.getPtPathId();

}
