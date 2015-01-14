#include "Path.hpp"
#include <boost/foreach.hpp>
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/WayPoint.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/LaneConnector.hpp"
#include "util/Utils.hpp"
#include <boost/iterator/filter_iterator.hpp>


namespace{
sim_mob::BasicLogger & logger = sim_mob::Logger::log("path_set");
}

sim_mob::SinglePath::SinglePath() : purpose(work),utility(0.0),pathSize(0.0),travelCost(0.0),
signalNumber(0.0),rightTurnNumber(0.0),length(0.0),travleTime(0.0),highWayDistance(0.0),valid_path(true),
isMinTravelTime(0),isMinDistance(0),isMinSignal(0),isMinRightTurn(0),isMaxHighWayUsage(0),
isShortestPath(0), index(-1),path(std::vector<sim_mob::WayPoint>()),isNeedSave2DB(false){
}

sim_mob::SinglePath::SinglePath(const SinglePath& source) :
		id(source.id),
		utility(source.utility),pathSize(source.pathSize),
		travelCost(source.travelCost),valid_path(source.valid_path),
		signalNumber(source.signalNumber),
		rightTurnNumber(source.rightTurnNumber),
		length(source.length),travleTime(source.travleTime),
		pathSetId(source.pathSetId),highWayDistance(source.highWayDistance),
		isMinTravelTime(source.isMinTravelTime),isMinDistance(source.isMinDistance),isMinSignal(source.isMinSignal),
		isMinRightTurn(source.isMinRightTurn),isMaxHighWayUsage(source.isMaxHighWayUsage),isShortestPath(source.isShortestPath)
{
	isNeedSave2DB=false;

	purpose = sim_mob::work;
}

sim_mob::SinglePath::~SinglePath(){
	clear();
}


bool sim_mob::SinglePath::includesRoadSegment(const std::set<const sim_mob::RoadSegment*> & segs, bool dbg, std::stringstream *out){
	if(!this->path.size())
	{
		int i = 0;
	}
	BOOST_FOREACH(sim_mob::WayPoint &wp, this->path){
		BOOST_FOREACH(const sim_mob::RoadSegment* seg, segs){
			if(dbg){
				*out << "checking " << wp.roadSegment_->getId() << " against " <<  seg->getId() << "\n";
			}
			std::stringstream hack1(""),hack2("");
			hack1 << wp.roadSegment_->getId();
			hack2 << seg->getId();
			if(hack1.str() == hack2.str() ){
				return true;
			}
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
	if(this->path.empty())
	{
	   std::string err = "empty path for OD:" + this->pathSetId + "--"  + this->id;
	   throw std::runtime_error(err);
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

	//step-2 right/left turn
	sim_mob::calculateRightTurnNumberAndSignalNumberByWaypoints(this);
	//step-3 highway distance
	highWayDistance = sim_mob::calculateHighWayDistance(this);
	//step-4 length
	length = sim_mob::generateSinglePathLength(path);
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
	travleTime=0.0;
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
	fromNode = NULL;
	toNode = NULL;
	subTrip = NULL;
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

void sim_mob::PathSet::addOrDeleteSinglePath(sim_mob::SinglePath* s)
{
	if(!s)
	{
		return;
	}
	if(!pathChoices.insert(s).second)
	{
		safe_delete_item(s);
	}
}


double sim_mob::calculateHighWayDistance(sim_mob::SinglePath *sp)
{
	double res=0;
	if(!sp) return 0.0;
	for(int i=0;i<sp->path.size();++i)
	{
		sim_mob::WayPoint& w = sp->path[i];
		if (w.type_ == sim_mob::WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w.roadSegment_;
			if(seg->maxSpeed >= 60)
			{
				res += seg->length;
			}
		}
	}
	return res/100.0; //meter
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
	if(sp->path.size()<2)
	{
		sp->rightTurnNumber=0;
		sp->signalNumber=0;
		return ;
	}
	int res=0;
	int signalNumber=0;
	std::vector<sim_mob::WayPoint>::iterator itt=sp->path.begin();
	++itt;
	for(std::vector<sim_mob::WayPoint>::iterator it=sp->path.begin();it!=sp->path.end();++it)
	{
		const RoadSegment* currentSeg = it->roadSegment_;
		const RoadSegment* targetSeg = NULL;
		if(itt!=sp->path.end())
		{

			targetSeg = itt->roadSegment_;
		}
		else // already last segment
		{
			break;
		}

		if(currentSeg->getEnd() == currentSeg->getLink()->getEnd()) // intersection
		{
			signalNumber++;
			// get lane connector
			const std::set<sim_mob::LaneConnector*>& lcs = dynamic_cast<const sim_mob::MultiNode*> (currentSeg->getEnd())->getOutgoingLanes(currentSeg);
			for (std::set<LaneConnector*>::const_iterator it2 = lcs.begin(); it2 != lcs.end(); it2++) {
				if((*it2)->getLaneTo()->getRoadSegment() == targetSeg)
				{
					int laneIndex = sim_mob::getLaneIndex2((*it2)->getLaneFrom());
					if(laneIndex<2)//most left lane
					{
						res++;
						break;
					}
				}//end if targetSeg
			}// end for lcs
//			}// end if lcs
		}//end currEndNode
		++itt;
	}//end for
	sp->rightTurnNumber=res;
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
		res += seg->length;
	}
	return res/100.0; //meter
}


std::string sim_mob::makeWaypointsetString(std::vector<sim_mob::WayPoint>& wp)
{
	std::string str;
	if(wp.size()==0)
	{
		sim_mob::Logger::log("path_set")<<"warning: empty input for makeWaypointsetString"<<std::endl;
	}

	for(std::vector<sim_mob::WayPoint>::iterator it = wp.begin(); it != wp.end(); it++)
	{
		if (it->type_ == WayPoint::ROAD_SEGMENT)
		{
			std::string tmp = it->roadSegment_->originalDB_ID.getLogItem();
			str += sim_mob::Utils::getNumberFromAimsunId(tmp) + ",";
		} // if ROAD_SEGMENT
	}

	if(str.size()<1)
	{
		// when same f,t node, it happened
		sim_mob::Logger::log("path_set")<<"warning: empty output makeWaypointsetString id"<<std::endl;
	}
	return str;
}
