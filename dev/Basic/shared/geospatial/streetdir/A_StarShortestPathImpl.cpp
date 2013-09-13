/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "A_StarShortestPathImpl.hpp"

#include <cmath>

//for caching
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

//TODO: Prune this include list later; it was copied directly from StreetDirectory.cpp
#include "entities/TrafficWatch.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/ZebraCrossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "logging/Log.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


boost::shared_mutex sim_mob::A_StarShortestPathImpl::GraphSearchMutex_;
namespace {
 typedef boost::unordered_map< std::string, std::vector<WayPoint> > CacheMap;
 boost::thread_specific_ptr<CacheMap> cache;

 void initCache() {
	 //The first time called by the current thread then just create one.
	 if (!cache.get()){
		 cache.reset(new CacheMap());
	 }
 }
}

sim_mob::A_StarShortestPathImpl::A_StarShortestPathImpl(const RoadNetwork& network)
{
	initDrivingNetworkNew(network.getLinks());
	initWalkingNetworkNew(network.getLinks());
}

/*sim_mob::A_StarShortestPathImpl::~A_StarShortestPathImpl()
{
    // Cleanup to avoid memory leakage.
    for (size_t i = 0; i < nodes_.size(); ++i) {
        Node * node = nodes_[i];
        delete node;
    }
    nodes_.clear();
}*/


//Helper: Add an edge, approximate the distance.
StreetDirectory::Edge sim_mob::A_StarShortestPathImpl::AddSimpleEdge(StreetDirectory::Graph& graph, StreetDirectory::Vertex& fromV, StreetDirectory::Vertex& toV, sim_mob::WayPoint wp)
{
	StreetDirectory::Edge edge;
	bool ok;
	double currDist = sim_mob::dist(
		boost::get(boost::vertex_name, graph, fromV),
		boost::get(boost::vertex_name, graph, toV)
	);
    boost::tie(edge, ok) = boost::add_edge(fromV, toV, graph);
    boost::put(boost::edge_name, graph, edge, wp);
    boost::put(boost::edge_weight, graph, edge, currDist);
    return edge;
}


//Helper: Find the vertex for the starting node for a given road segment.
StreetDirectory::Vertex sim_mob::A_StarShortestPathImpl::FindStartingVertex(const sim_mob::RoadSegment* rs, const std::map<const Node*, VertexLookup>& nodeLookup)
{
	map<const Node*, A_StarShortestPathImpl::VertexLookup>::const_iterator from = nodeLookup.find(rs->getStart());
	if (from==nodeLookup.end()) {
		throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
	}
	if (from->second.vertices.empty()) {
		throw std::runtime_error("Road Segment's to node has no known mapped vertices");
	}

	//For simply nodes, this will be sufficient.
	StreetDirectory::Vertex fromVertex = from->second.vertices.front().v;

	//If there are multiple options, search for the right one.
	//To accomplish this, just match our "before/after" tagged data. Note that before/after may be null.
	if (from->second.vertices.size()>1) {
		bool error=true;
		for (std::vector<A_StarShortestPathImpl::NodeDescriptor>::const_iterator it=from->second.vertices.begin(); it!=from->second.vertices.end(); it++) {
			if (rs == it->after) {
				fromVertex = it->v;
				error = false;
			}
		}
		if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"from\" vertex map."); }
	}

	return fromVertex;
}


StreetDirectory::Vertex sim_mob::A_StarShortestPathImpl::FindEndingVertex(const sim_mob::RoadSegment* rs, const std::map<const Node*, VertexLookup>& nodeLookup)
{
	map<const Node*, A_StarShortestPathImpl::VertexLookup>::const_iterator to = nodeLookup.find(rs->getEnd());
	if (to==nodeLookup.end()) {
		throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
	}
	if (to->second.vertices.empty()) {
		throw std::runtime_error("Road Segment's to node has no known mapped vertices");
	}

	//For simply nodes, this will be sufficient.
	StreetDirectory::Vertex toVertex = to->second.vertices.front().v;

	//If there are multiple options, search for the right one.
	//To accomplish this, just match our "before/after" tagged data. Note that before/after may be null.
	if (to->second.vertices.size()>1) {
		bool error=true;
		for (std::vector<A_StarShortestPathImpl::NodeDescriptor>::const_iterator it=to->second.vertices.begin(); it!=to->second.vertices.end(); it++) {
			if (rs == it->before) {
				toVertex = it->v;
				error = false;
			}
		}
		if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"to\" vertex map."); }
	}

	return toVertex;
}


void sim_mob::A_StarShortestPathImpl::initDrivingNetworkNew(const vector<Link*>& links)
{
	//Various lookup structures
	map<const Node*, VertexLookup> nodeLookup;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingNodes(drivingMap_, (*iter)->getPath(), nodeLookup);
    }

    //Proceed through our Links, adding each RoadSegment path. Split vertices as required.
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingLinks(drivingMap_, (*iter)->getPath(), nodeLookup, drivingSegmentLookup_);
    }

    //Now add all Intersection edges (lane connectors)
    for (map<const Node*, VertexLookup>::const_iterator it=nodeLookup.begin(); it!=nodeLookup.end(); it++) {
    	procAddDrivingLaneConnectors(drivingMap_, dynamic_cast<const MultiNode*>(it->first), nodeLookup);
    }

    //Now add BusStops (this mutates the network slightly, by segmenting Edges where a BusStop is located).
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingBusStops(drivingMap_, (*iter)->getPath(), nodeLookup, drivingBusStopLookup_, drivingSegmentLookup_);
    }

    //Finally, add our "master" node vertices
    procAddStartNodesAndEdges(drivingMap_, nodeLookup, drivingNodeLookup_);
}

void sim_mob::A_StarShortestPathImpl::initWalkingNetworkNew(const vector<Link*>& links)
{
	//Various lookup structures
	map<const Node*, VertexLookup> nodeLookup;

	{
	//Building MultiNodes requires one additional step.
	map<const Node*, VertexLookup> unresolvedNodes;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingNodes(walkingMap_, (*iter)->getPath(), nodeLookup, unresolvedNodes);
    }

    //Resolve MultiNodes here:
    procResolveWalkingMultiNodes(walkingMap_, unresolvedNodes, nodeLookup);
	}

    //Proceed through our Links, adding each RoadSegment path. Split vertices as required.
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingLinks(walkingMap_, (*iter)->getPath(), nodeLookup);
    }

    //Now add all Crossings
    {
    set<const Crossing*> completedCrossings;
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingCrossings(walkingMap_, (*iter)->getPath(), nodeLookup, completedCrossings);
    }
    }

    //Now add BusStops (this mutates the network slightly, by segmenting Edges where a BusStop is located).
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingBusStops(walkingMap_, (*iter)->getPath(), nodeLookup, walkingBusStopLookup_);
    }

    //Finally, add our "master" node vertices
    procAddStartNodesAndEdges(walkingMap_, nodeLookup, walkingNodeLookup_);
}



void sim_mob::A_StarShortestPathImpl::procAddDrivingNodes(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Scan each pair of RoadSegments at each Node (the Node forms the joint between these two). This includes "null" options (for the first/last node).
	//So, (null, X) is the first Node (before segment X), and (Y, null) is the last one. (W,Z) is the Node between segments W and Z.
	for (size_t i=0; i<=roadway.size(); i++) {
		//before/after/node/isUni forms a complete Node descriptor.
		NodeDescriptor nd;
		nd.before = (i==0) ? nullptr : const_cast<RoadSegment*>(roadway.at(i-1));
		nd.after  = (i>=roadway.size()) ? nullptr : const_cast<RoadSegment*>(roadway.at(i));
		const Node* origNode = nd.before ? nd.before->getEnd() : nd.after->getStart();
		if (nodeLookup.count(origNode)==0) {
			nodeLookup[origNode] = VertexLookup();
			nodeLookup[origNode].origNode = origNode;
			nodeLookup[origNode].isUni = dynamic_cast<const UniNode*>(origNode);
		}

		//Construction varies drastically depending on whether or not this is a UniNode
		if (nodeLookup[origNode].isUni) {
			//We currently don't allow U-turns at UniNodes, so for now each unique Node descriptor represents a unique path.
			nd.v = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			nodeLookup[origNode].vertices.push_back(nd);

			//We'll create a fake Node for this location (so it'll be represented properly). Once we've fully switched to the
			//  new algorithm, we'll have to switch this to a value-based type; using "new" will leak memory.
			Point2D newPos;
			//TODO: re-enable const after fixing RoadNetwork's sidewalks.
			if (!nd.before && nd.after) {
				newPos = const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front();
			} else if (nd.before && !nd.after) {
				newPos = const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back();
			} else {
				//Estimate
				DynamicVector vec(const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back(), const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front());
				vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
				newPos = Point2D(vec.getX(), vec.getY());
			}

			//Node* vNode = new UniNode(newPos.getX(), newPos.getY());
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), nd.v, newPos);
		} else {
			//Each incoming and outgoing RoadSegment has exactly one Node at the Intersection. In this case, the unused before/after
			//   RoadSegment is used to identify whether this is an incoming or outgoing Vertex.
			nd.v = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			nodeLookup[origNode].vertices.push_back(nd);

			//Our Node positions are actually the same compared to UniNodes; we may merge this code later.
			Point2D newPos;
			if (!nd.before && nd.after) {
				newPos = const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front();
			} else if (nd.before && !nd.after) {
				newPos = const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back();
			} else {
				//This, however, is different.
				throw std::runtime_error("MultiNode vertices can't have both \"before\" and \"after\" segments.");
			}

			//Node* vNode = new UniNode(newPos.getX(), newPos.getY());
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), nd.v, newPos);
		}
	}
}


void sim_mob::A_StarShortestPathImpl::procAddDrivingBusStops(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup, std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup, std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >& resSegLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Iterate through all obstacles on all RoadSegments and find BusStop obstacles.
	for (vector<RoadSegment*>::const_iterator rsIt=roadway.begin(); rsIt!=roadway.end(); rsIt++) {
		const RoadSegment* rs = *rsIt;
		for (map<centimeter_t, const RoadItem*>::const_iterator it=rs->obstacles.begin(); it!=rs->obstacles.end(); it++) {
			const BusStop* bstop = dynamic_cast<const BusStop*>(it->second);
			if (!bstop) {
				continue;
			}

			//Retrieve the original "start" and "end" Nodes for this segment.
			StreetDirectory::Vertex fromVertex = FindStartingVertex(rs, nodeLookup);
			StreetDirectory::Vertex toVertex = FindEndingVertex(rs, nodeLookup);

			//At this point, we have the Bus Stop, as well as the Road Segment that it appears on.
			//We need to do two things:
			//  1) Segment the current Edge into two smaller edges; one before the BusStop and one after.
			//  2) Add a Vertex for the stop itself, and connect an incoming and outgoing Edge to it.
			//Note that both of these tasks require calculating normal intersection of the BusStop and the RoadSegment.
			Point2D bstopPoint(bstop->xPos, bstop->yPos);
			DynamicVector roadSegVec(
				boost::get(boost::vertex_name, graph, fromVertex),
				boost::get(boost::vertex_name, graph, toVertex)
			);
			Point2D newSegPt;

			//For now, this is optional.
			try {
				newSegPt = normal_intersect(bstopPoint, roadSegVec);
			} catch (std::exception& ex) {
				Warn() <<"Normal intersection could not be found for bus stop: " <<bstop->id <<std::endl
				       <<ex.what() <<std::endl;
				continue;
			}

			//Note that, in terms of "segmenting", we can either *actually* split the segment, or we can add
			//  a second, segmented version of the Road Segment on top of the original. This is helpful in terms
			//  of allowing us to "add on" additional data without requiring a collation function, but it means
			//  that we will need master nodes for Bus Stops (to prevent U-turns). (Actually, preventing U-turns will
			//  likely necessitate a master node anyway). In addition, this might make the network confusing to view.
			//We choose to "add a layer" to the network, since modifying the data directly makes it harder to
			//  find the same Segment later (we'd need a "lookup" structure for segments, which becomes difficult to maintain).

			//This node has no associated "lookup" or "original" values, since it's artificial.
			StreetDirectory::Vertex midV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), midV, newSegPt);

			//Add the BusStop vertex. This node is unique per BusStop per SEGMENT, since it allows a loopback.
			//For  now, it makes no sense to put a path to the Bus Stop on the reverse segment (cars need to park on
			// the correct side of the road), but for path finding we might want to consider it later.
			StreetDirectory::Vertex busSrcV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), busSrcV, bstopPoint);

			StreetDirectory::Vertex busSinkV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), busSinkV, bstopPoint);

			//Add the Bus vertex to a lookup
			if (resLookup.count(bstop)>0) {
				throw std::runtime_error("Duplicate Bus Stop in lookup.");
			}
			resLookup[bstop] = std::make_pair(busSrcV, busSinkV);

			//Add the new route. (from->mid->bus; bus->mid->to)
			StreetDirectory::Edge e1 = AddSimpleEdge(graph, fromVertex, midV, WayPoint(rs));
			AddSimpleEdge(graph, midV, busSinkV, WayPoint(bstop));

			AddSimpleEdge(graph, busSrcV, midV, WayPoint(bstop));
			StreetDirectory::Edge e2 = AddSimpleEdge(graph, midV, toVertex, WayPoint(rs));

			//Save them in our lookup.
		    resSegLookup[rs].insert(e1);
		    resSegLookup[rs].insert(e2);
		}
	}
}


namespace {

//Helper function: Retrieve a set of sidewalk lane pairs (fromLane, toLane) given two RoadSegments.
//If both inputs are non-null, then from/to *must* exist (e.g., UniNodes).
//TODO: Right now, this function is quite hackish, and only checks the outer and inner lanes.
//      We need to improve it to work for any number of sidewalk lanes (e.g., median sidewalks), but
//      for now we don't even have the data.
//TODO: The proper way to do this is with an improved version of UniNode lane connectors.
vector< std::pair<int, int> > GetSidewalkLanePairs(const RoadSegment* before, const RoadSegment* after) {
	//Error check: at least one segment must exist
	if (!before && !after) { throw std::runtime_error("Can't GetSidewalkLanePairs on two null segments."); }

	//Store two partial lists
	vector<int> beforeLanes;
	vector<int> afterLanes;
	vector< std::pair<int, int> > res;

	//Build up before
	if (before) {
		for (size_t i=0; i<before->getLanes().size(); i++) {
			if (before->getLanes().at(i)->is_pedestrian_lane()) {
				beforeLanes.push_back(i);
			}
		}
	}

	//Build up after
	if (after) {
		for (size_t i=0; i<after->getLanes().size(); i++) {
			if (after->getLanes().at(i)->is_pedestrian_lane()) {
				afterLanes.push_back(i);
			}
		}
	}

	//It's possible that we have no results
	if ((before&&beforeLanes.empty()) || (after&&afterLanes.empty())) {
		return res;
	}

	//If we have both before and after, only pairs can be added (no null values).
	// We can manage this implicitly by either counting up or down, and stopping when we have no more values.
	// For now, we just ensure they're equal or add NONE
	if (before && after) {
		if (beforeLanes.size()==afterLanes.size()) {
			for (size_t i=0; i<beforeLanes.size(); i++) {
				res.push_back(std::make_pair(beforeLanes.at(i), afterLanes.at(i)));
			}
		}
		return res;
	}

	//Otherwise, just build a partial list
	for (size_t i=0; i<beforeLanes.size() || i<afterLanes.size(); i++) {
		if (before) {
			res.push_back(std::make_pair(beforeLanes.at(i), -1));
		} else {
			res.push_back(std::make_pair(-1, afterLanes.at(i)));
		}
	}
	return res;
}

} //End un-named namespace


void sim_mob::A_StarShortestPathImpl::procAddWalkingBusStops(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup, std::map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Iterate through all obstacles on all RoadSegments and find BusStop obstacles.
	for (vector<RoadSegment*>::const_iterator rsIt=roadway.begin(); rsIt!=roadway.end(); rsIt++) {
		//Exit early, if necessary
		const RoadSegment* rs = *rsIt;
		std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(rs, nullptr);
		if (lanePairs.empty()) {
			continue;
		}

		for (map<centimeter_t, const RoadItem*>::const_iterator it=rs->obstacles.begin(); it!=rs->obstacles.end(); it++) {
			const BusStop* bstop = dynamic_cast<const BusStop*>(it->second);
			if (!bstop) {
				continue;
			}

			//At this point, our goal is still to retrieve the from/to Vertices. However, lanes make this
			// more complicated. The easiest solution is to compute the midpoint of each potential Lane
			// candidate, and then choose the closest one. This won't affect performance much, as each
			// RoadSegment will usually only have 1 or 2 sidewalks.
			std::map<const Node*, VertexLookup>::const_iterator fromIt = nodeLookup.find(rs->getStart());
			std::map<const Node*, VertexLookup>::const_iterator toIt = nodeLookup.find(rs->getEnd());
			if (fromIt==nodeLookup.end() || toIt==nodeLookup.end()) {
				throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
			}
			if (fromIt->second.vertices.empty() || toIt->second.vertices.empty()) {
				Warn() <<"Warning: Road Segment's nodes have no known mapped vertices (3)." <<std::endl;
				continue;
			}

			//Helper data
			Point2D bstopPoint(bstop->xPos, bstop->yPos);

			//What we're searching for.
			StreetDirectory::Vertex fromV;
			StreetDirectory::Vertex toV;

			//Our quality control.
			Point2D midPt;
			double minDist = -1; //<0 == error

			//Use this to ensure that we aren't getting the wrong RoadSegment as an artifact.
			bool error = false;

			//Start searching!
			for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
				int laneID = it->first;

				//Find a "from" and "to" Vertex that match this laneID
				for (std::vector<NodeDescriptor>::const_iterator it1=fromIt->second.vertices.begin(); it1!=fromIt->second.vertices.end(); it1++) {
					if ((rs==it1->after && laneID==it1->afterLaneID) || (rs==it1->before && laneID==it1->beforeLaneID)) {
						NodeDescriptor fromCandidate = *it1;
						for (std::vector<NodeDescriptor>::const_iterator it2=toIt->second.vertices.begin(); it2!=toIt->second.vertices.end(); it2++) {
							if ((rs==it2->before && laneID==it2->beforeLaneID) || (rs==it2->after && laneID==it2->afterLaneID)) {
								NodeDescriptor toCandidate = *it2;

								//At this point, fromCandidate and toCandidate represent a viable from/to pair on a RoadSegment representing the same Lane.
								DynamicVector laneSegVect(
									boost::get(boost::vertex_name, graph, fromCandidate.v),
									boost::get(boost::vertex_name, graph, toCandidate.v)
								);
								Point2D candMidPt;

								//For now, this is optional.
								try {
									candMidPt = normal_intersect(bstopPoint, laneSegVect);
								} catch (std::exception& ex) {
									Warn() <<"Normal intersection could not be found for bus stop: " <<bstop->id <<std::endl
									       <<ex.what() <<std::endl;
									error = true;
									continue;
								}

								//Is it closer?
								double candDist = sim_mob::dist(candMidPt, bstopPoint);
								if ((minDist<0) || (candDist<minDist)) {
									minDist = candDist;
									midPt = candMidPt;
									fromV = fromCandidate.v;
									toV = toCandidate.v;
								}
							}
						}
					}
				}
			}


			//Did we find anything?
			if ((minDist<0) || error) {
				//For now it's not an error.
				Warn() <<"Warning: BusStop on WalkingPath could not find any candidate Lanes." <<std::endl;
				continue;
			}

			//Now our algorithm proceeds much the same as before, except that Sidewalks are bi-directional in nature.
			StreetDirectory::Vertex midSinkV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), midSinkV, midPt);
			StreetDirectory::Vertex midSrcV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), midSrcV, midPt);

			//Add the BusStop vertex. This node is unique per BusStop per SEGMENT, since it allows a loopback.
			//For  now, it makes no sense to put a path to the Bus Stop on the reverse segment (cars need to park on
			// the correct side of the road), but for path finding we might want to consider it later.
			StreetDirectory::Vertex busSrcV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), busSrcV, bstopPoint);

			StreetDirectory::Vertex busSinkV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), busSinkV, bstopPoint);

			//Add the Bus vertex to a lookup
			if (resLookup.count(bstop)>0) {
				throw std::runtime_error("Duplicate Bus Stop in lookup.");
			}
			resLookup[bstop] = std::make_pair(busSrcV, busSinkV);

			//Add the new route. (from->mid->bus; to->mid->bus)
			AddSimpleEdge(graph, fromV, midSinkV, WayPoint(rs));
			AddSimpleEdge(graph, toV, midSinkV, WayPoint(rs));
			AddSimpleEdge(graph, midSinkV, busSinkV, WayPoint(bstop));

			//Add the new route. (bus->mid->from; bus->mid->to)
			AddSimpleEdge(graph, busSrcV, midSrcV, WayPoint(bstop));
			AddSimpleEdge(graph, midSrcV, fromV, WayPoint(rs));
			AddSimpleEdge(graph, midSrcV, toV, WayPoint(rs));
		}
	}
}



void sim_mob::A_StarShortestPathImpl::procAddDrivingLinks(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup, std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >& resSegLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Here, we are simply assigning one Edge per RoadSegment in the Link. This is mildly complicated by the fact that a Node*
	//  may be represented by multiple vertices; overall, though, it's a conceptually simple procedure.
	for (vector<RoadSegment*>::const_iterator it=roadway.begin(); it!=roadway.end(); it++) {
		const RoadSegment* rs = *it;
		map<const Node*, VertexLookup>::const_iterator from = nodeLookup.find(rs->getStart());
		map<const Node*, VertexLookup>::const_iterator to = nodeLookup.find(rs->getEnd());
		if (from==nodeLookup.end() || to==nodeLookup.end()) {
			throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
		}
		if (from->second.vertices.empty() || to->second.vertices.empty()) {
			Warn() <<"Warning: Road Segment's nodes have no known mapped vertices (1)." <<std::endl;
			continue;
		}

		//For simple nodes, this will be sufficient.
		StreetDirectory::Vertex fromVertex = from->second.vertices.front().v;
		StreetDirectory::Vertex toVertex = to->second.vertices.front().v;

		//If there are multiple options, search for the right one.
		//To accomplish this, just match our "before/after" tagged data. Note that before/after may be null.
		if (from->second.vertices.size()>1) {
			bool error=true;
			for (std::vector<NodeDescriptor>::const_iterator it=from->second.vertices.begin(); it!=from->second.vertices.end(); it++) {
				if (rs == it->after) {
					fromVertex = it->v;
					error = false;
				}
			}
			if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"from\" vertex map."); }
		}
		if (to->second.vertices.size()>1) {
			bool error=true;
			for (std::vector<NodeDescriptor>::const_iterator it=to->second.vertices.begin(); it!=to->second.vertices.end(); it++) {
				if (rs == it->before) {
					toVertex = it->v;
					error = false;
				}
			}
			if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"to\" vertex map."); }
		}

		//Create an edge.
	    StreetDirectory::Edge edge;
	    bool ok;
	    boost::tie(edge, ok) = boost::add_edge(fromVertex, toVertex, graph);
	    boost::put(boost::edge_name, graph, edge, WayPoint(rs));
	    boost::put(boost::edge_weight, graph, edge, rs->length);

	    //Save this in our lookup.
	    resSegLookup[rs].insert(edge);
	}
}

void sim_mob::A_StarShortestPathImpl::procAddDrivingLaneConnectors(StreetDirectory::Graph& graph, const MultiNode* node, const map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip nulled Nodes (may be UniNodes).
	if (!node) {
		return;
	}

	//We actually only care about RoadSegment->RoadSegment connections.
	set< std::pair<RoadSegment*, RoadSegment*> > connectors;
	for (map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator conIt=node->getConnectors().begin(); conIt!=node->getConnectors().end(); conIt++) {
		for (set<sim_mob::LaneConnector*>::const_iterator it=conIt->second.begin(); it!=conIt->second.end(); it++) {
			connectors.insert(std::make_pair((*it)->getLaneFrom()->getRoadSegment(), (*it)->getLaneTo()->getRoadSegment()));
		}
	}

	//Now, add each "RoadSegment" connector.
	for (set< std::pair<RoadSegment*, RoadSegment*> >::iterator it=connectors.begin(); it!=connectors.end(); it++) {
		//Sanity check:
		if (it->first->getEnd()!=node || it->second->getStart()!=node) {
			throw std::runtime_error("Node/Road Segment mismatch in Edge constructor.");
		}

		//Various bookkeeping requirements:
		std::pair<StreetDirectory::Vertex, bool> fromVertex;
		fromVertex.second = false;
		std::pair<StreetDirectory::Vertex, bool> toVertex;
		toVertex.second = false;
		std::map<const Node*, VertexLookup>::const_iterator vertCandidates = nodeLookup.find(node);
		if (vertCandidates==nodeLookup.end()) {
			throw std::runtime_error("Intersection's Node is unknown by the vertex map.");
		}

		//Find the "from" and "to" segments' associated end vertices. Keep track of each.
		for (vector<NodeDescriptor>::const_iterator ndIt=vertCandidates->second.vertices.begin(); ndIt!=vertCandidates->second.vertices.end(); ndIt++) {
			if (it->first == ndIt->before) {
				fromVertex.first = ndIt->v;
				fromVertex.second = true;
			}
			if (it->second == ndIt->after) {
				toVertex.first = ndIt->v;
				toVertex.second = true;
			}
		}

		//Ensure we have both
		if (!fromVertex.second || !toVertex.second) {
			//std::cout <<"ERROR_2906" <<std::endl; continue;
			throw std::runtime_error("Lane connector has no associated vertex.");
		}

		//Create an edge.
	    StreetDirectory::Edge edge;
	    bool ok;
	    boost::tie(edge, ok) = boost::add_edge(fromVertex.first, toVertex.first, graph);

	    //Calculate the edge length. Treat this as a Node WayPoint.
	    WayPoint revWP(node);
	    revWP.directionReverse = true;
	    DynamicVector lc(fromVertex.second, toVertex.second);
	    boost::put(boost::edge_name, graph, edge, revWP);
	    boost::put(boost::edge_weight, graph, edge, lc.getMagnitude());
	}
}




void sim_mob::A_StarShortestPathImpl::procAddWalkingNodes(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, map<const Node*, VertexLookup>& nodeLookup, map<const Node*, VertexLookup>& tempNodes)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Scan each pair of RoadSegments at each Node (the Node forms the joint between these two). This includes "null" options (for the first/last node).
	//So, (null, X) is the first Node (before segment X), and (Y, null) is the last one. (W,Z) is the Node between segments W and Z.
	for (size_t i=0; i<=roadway.size(); i++) {
		//before/after/node/isUni forms a complete Node descriptor.
		NodeDescriptor nd;
		nd.before = (i==0) ? nullptr : const_cast<RoadSegment*>(roadway.at(i-1));
		nd.after  = (i>=roadway.size()) ? nullptr : const_cast<RoadSegment*>(roadway.at(i));
		const Node* origNode = nd.before ? nd.before->getEnd() : nd.after->getStart();
		if (nodeLookup.count(origNode)==0) {
			nodeLookup[origNode] = VertexLookup();
			nodeLookup[origNode].origNode = origNode;
			nodeLookup[origNode].isUni = dynamic_cast<const UniNode*>(origNode);
		}

		//Construction varies drastically depending on whether or not this is a UniNode
		if (nodeLookup[origNode].isUni) {
			//There may be several (currently 0, 1 or 2) Pedestrian lanes connecting at this Node. We'll need a Node for each,
			//  since Pedestrians can't normally cross Driving lanes without jaywalking.
			vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(nd.before, nd.after);

			//Add each potential lane Vertex
			for (vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
				//Copy this node descriptor, modify it by adding in the from/to lanes.
				NodeDescriptor newNd(nd);
				newNd.beforeLaneID = it->first;
				newNd.afterLaneID = it->second;
				newNd.v = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
				nodeLookup[origNode].vertices.push_back(newNd);

				Point2D newPos;
				//TODO: re-enable const after fixing RoadNetwork's sidewalks.
				if (!nd.before && nd.after) {
					newPos = const_cast<RoadSegment*>(nd.after)->getLanes().at(it->second)->getPolyline().front();
				} else if (nd.before && !nd.after) {
					newPos = const_cast<RoadSegment*>(nd.before)->getLanes().at(it->first)->getPolyline().back();
				} else {
					//Estimate
					DynamicVector vec(const_cast<RoadSegment*>(nd.before)->getLanes().at(it->first)->getPolyline().back(), const_cast<RoadSegment*>(nd.after)->getLanes().at(it->second)->getPolyline().front());
					vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
					newPos = Point2D(vec.getX(), vec.getY());
				}

				//Node* vNode = new UniNode(newPos.getX(), newPos.getY());
				boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), newNd.v, newPos);
			}
		} else {
			//MultiNodes are much more complex. For now, we just collect all vertices into a "potential" list.
			//Fortunately, we only have to scan one list this time.
			std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(nd.before, nd.after);

			//Make sure our temp lookup list has this.
			if (tempNodes.count(origNode)==0) {
				tempNodes[origNode] = VertexLookup();
				tempNodes[origNode].origNode = origNode;
				tempNodes[origNode].isUni = nodeLookup[origNode].isUni;
			}

			for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
				//Copy this node descriptor, modify it by adding in the from/to lanes.
				NodeDescriptor newNd(nd);
				newNd.beforeLaneID = nd.before ? it->first : -1;
				newNd.afterLaneID = nd.after ? it->second : -1;
				//newNd.v = boost::add_vertex(const_cast<Graph &>(graph)); //Don't add it yet.

				//Our Node positions are actually the same compared to UniNodes; we may merge this code later.
				Point2D newPos;
				//TODO: re-enable const after fixing RoadNetwork's sidewalks.
				if (!nd.before && nd.after) {
					newPos = const_cast<RoadSegment*>(nd.after)->getLanes().at(newNd.afterLaneID)->getPolyline().front();
				} else if (nd.before && !nd.after) {
					newPos = const_cast<RoadSegment*>(nd.before)->getLanes().at(newNd.beforeLaneID)->getPolyline().back();
				} else {
					//This, however, is different.
					throw std::runtime_error("MultiNode vertices can't have both \"before\" and \"after\" segments.");
				}

				//Save in an alternate location for now, since we'll merge these later.
				newNd.tempPos = newPos;
				tempNodes[origNode].vertices.push_back(newNd); //Save in our temp list.
			}
		}
	}
}

namespace {
//Helper (we can't use std::set, so we use vector::find)
template <class T>
bool VectorContains(const vector<T>& vec, const T& value) {
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

//Helper: do these two segments start/end at the same pair of nodes?
bool SegmentCompare(const RoadSegment* self, const RoadSegment* other) {
	if ((self->getStart()==other->getStart()) && (self->getEnd()==other->getEnd())) {
		return true;  //Exact same
	}
	if ((self->getStart()==other->getEnd()) && (self->getEnd()==other->getStart())) {
		return true;  //Reversed, but same
	}
	return false; //Different.
}
} //End un-named namespace

void sim_mob::A_StarShortestPathImpl::procResolveWalkingMultiNodes(StreetDirectory::Graph& graph, const map<const Node*, VertexLookup>& unresolvedNodes, map<const Node*, VertexLookup>& nodeLookup)
{
	//We need to merge the potential vertices at all unresolved MultiNodes. At the moment, this requires some geometric assumption (but for roundabouts, later, this will no longer be acceptible)
	for (map<const Node*, VertexLookup>::const_iterator mnIt=unresolvedNodes.begin(); mnIt!=unresolvedNodes.end(); mnIt++) {
		//First, we need to compute the distance between every pair of Vertices.
		const Node* node = mnIt->first;
		map<double, std::pair<NodeDescriptor, NodeDescriptor> > distLookup;
		for (vector<NodeDescriptor>::const_iterator it1=mnIt->second.vertices.begin(); it1!=mnIt->second.vertices.end(); it1++) {
			for (vector<NodeDescriptor>::const_iterator it2=it1+1; it2!=mnIt->second.vertices.end(); it2++) {
				//No need to be exact here; if there are collisions, simply modify the result until it's unique.
				double dist = sim_mob::dist(it1->tempPos, it2->tempPos);
				while (distLookup.count(dist)>0) {
					dist += 0.000001;
				}

				//Save it.
				distLookup[dist] = std::make_pair(*it1, *it2);
			}
		}

		//Nothing to do?
		if (distLookup.empty()) {
			continue;
		}

		//Iterate in order, pairing the two closest elements if their total distance is less than 1/2 of the maximum distance.
		//Note that map::begin/end is essentially in order. (Also, we keep a list of what's been tagged already).
		map<double, std::pair<NodeDescriptor, NodeDescriptor> >::const_iterator lastValue = distLookup.end();
		lastValue--;
		double maxDist = lastValue->first / 2.0;
		vector<NodeDescriptor> alreadyMerged;
		for (map<double, std::pair<NodeDescriptor, NodeDescriptor> >::const_iterator it=distLookup.begin(); it!=distLookup.end(); it++) {
			//Find a Vertex we haven't merged yet.
			if (VectorContains(alreadyMerged, it->second.first) || VectorContains(alreadyMerged, it->second.second)) {
				continue;
			}

			//Now check the distance between our two candidate Vertices.
			if (it->first > maxDist) {
				break; //All distances after this will be greater, since the map is sorted.
			}

			//Create a new Node Descriptor for this merged Node. "before" and "after" are arbitrary, since Pedestrians can walk bidirectionally on their edges.
			NodeDescriptor newNd;
			newNd.before = it->second.first.before ? it->second.first.before : it->second.first.after;
			newNd.after = it->second.second.before ? it->second.second.before : it->second.second.after;
			newNd.beforeLaneID = it->second.first.before ? it->second.first.beforeLaneID : it->second.first.afterLaneID;
			newNd.afterLaneID = it->second.second.before ? it->second.second.beforeLaneID : it->second.second.afterLaneID;

			//Heuristical check: If the two segments in question start/end at the same pair of nodes, then don't add this
			//  (we must use Crossings in this case).
			//Note that this won't catch cases where additional Nodes are added, but it will also never cause any harm.
			if (SegmentCompare(newNd.before, newNd.after)) {
				continue;
			}

			//Add it to our boost::graph
			newNd.v = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));

			//Put the actual point halfway between the two candidate points.
			DynamicVector vec(it->second.first.tempPos, it->second.second.tempPos);
			vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
			//Node* vNode = new UniNode(vec.getX(), vec.getY());   //TODO: Leaks memory!
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), newNd.v, Point2D(vec.getX(), vec.getY()));

			//Tag each unmerged Vertex so that we don't re-use them.
			alreadyMerged.push_back(it->second.first);
			alreadyMerged.push_back(it->second.second);

			//Also add this to our list of known vertices, so that we can find it later.
			nodeLookup[node].vertices.push_back(newNd);
		}

		//Finally, some Nodes may not have been merged at all. Just add these as-is.
		for (std::vector<NodeDescriptor>::const_iterator it=mnIt->second.vertices.begin(); it!=mnIt->second.vertices.end(); it++) {
			if (VectorContains(alreadyMerged, *it)) {
				continue;
			}

			//before/after should be set properly in this case.
			NodeDescriptor newNd(*it);
			newNd.v = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			//Node* vNode = new UniNode(it->tempPos.getX(), it->tempPos.getY());   //TODO: Leaks memory!
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), newNd.v, it->tempPos);

			//Save it for later.
			nodeLookup[node].vertices.push_back(newNd);
		}
	}
}



void sim_mob::A_StarShortestPathImpl::procAddWalkingLinks(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Here, we are simply assigning one Edge per RoadSegment in the Link. This is mildly complicated by the fact that a Node*
	//  may be represented by multiple vertices; overall, though, it's a conceptually simple procedure.
	//Note that Walking edges are two-directional; for now, we accomplish this by adding 2 edges (we can change it to an undirected graph later).
	for (std::vector<RoadSegment*>::const_iterator it=roadway.begin(); it!=roadway.end(); it++) {
		//Retrieve the lane pairs and return early if there are none. This allows us to avoid generating warnings
		//  when there *is* no associated Vertex for a given Segment, for whatever reason.
		const RoadSegment* rs = *it;
		std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(rs, nullptr);
		if (lanePairs.empty()) {
			continue;
		}

		std::map<const Node*, VertexLookup>::const_iterator from = nodeLookup.find(rs->getStart());
		std::map<const Node*, VertexLookup>::const_iterator to = nodeLookup.find(rs->getEnd());
		if (from==nodeLookup.end() || to==nodeLookup.end()) {
			throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
		}
		if (from->second.vertices.empty() || to->second.vertices.empty()) {
			Warn() <<"Warning: Road Segment's nodes have no known mapped vertices (2)." <<std::endl;
			continue;
		}

		//Of course, we still need to deal with Lanes
		for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
			int laneID = it->first;
			//For simply nodes, this will be sufficient.
			StreetDirectory::Vertex fromVertex = from->second.vertices.front().v;
			StreetDirectory::Vertex toVertex = to->second.vertices.front().v;

			//If there are multiple options, search for the right one.
			//Note that for walking nodes, before OR after may match (due to the way we merge MultiNodes).
			//Note that before/after may be null.
			if (from->second.vertices.size()>1) {
				bool error=true;
				for (std::vector<NodeDescriptor>::const_iterator it=from->second.vertices.begin(); it!=from->second.vertices.end(); it++) {
					if ((rs==it->after && laneID==it->afterLaneID) || (rs==it->before && laneID==it->beforeLaneID)) {
						fromVertex = it->v;
						error = false;
					}
				}
				if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"from\" vertex map."); }
			}
			if (to->second.vertices.size()>1) {
				bool error=true;
				for (std::vector<NodeDescriptor>::const_iterator it=to->second.vertices.begin(); it!=to->second.vertices.end(); it++) {
					if ((rs==it->before && laneID==it->beforeLaneID) || (rs==it->after && laneID==it->afterLaneID)) {
						toVertex = it->v;
						error = false;
					}
				}
				if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"to\" vertex map."); }
			}

			//Create an edge.
			{
			StreetDirectory::Edge edge;
			bool ok;
			boost::tie(edge, ok) = boost::add_edge(fromVertex, toVertex, graph);
			boost::put(boost::edge_name, graph, edge, WayPoint(rs->getLanes().at(laneID)));
			boost::put(boost::edge_weight, graph, edge, rs->length);
			}

			//Create the reverse edge
			{
			StreetDirectory::Edge edge;
			bool ok;
			WayPoint revWP(rs->getLanes().at(laneID));
			revWP.directionReverse = true;
			boost::tie(edge, ok) = boost::add_edge(toVertex, fromVertex, graph);
			boost::put(boost::edge_name, graph, edge, revWP);
			boost::put(boost::edge_weight, graph, edge, rs->length);
			}
		}
	}
}


void sim_mob::A_StarShortestPathImpl::procAddWalkingCrossings(StreetDirectory::Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::set<const Crossing*>& completed)
{
	//Skip empty paths
	if (roadway.empty()) {
		return;
	}

	//We need to scan each RoadSegment in our roadway for any possible Crossings. The "nextObstacle" function can do this.
	for (vector<RoadSegment*>::const_iterator segIt=roadway.begin(); segIt!=roadway.end(); segIt++) {
		//NOTE: For now, it's just easier to scan the obstacles list manually.
		for (map<centimeter_t, const RoadItem*>::const_iterator riIt=(*segIt)->obstacles.begin(); riIt!=(*segIt)->obstacles.end(); riIt++) {
			//Check if it's a crossing; check if we've already processed it; tag it.
			const Crossing* cr = dynamic_cast<const Crossing*>(riIt->second);
			if (!cr || completed.find(cr)!=completed.end()) {
				continue;
			}
			completed.insert(cr);

			//At least one of the Segment's endpoints must be a MultiNode. Pick the closest one.
			//TODO: Currently we can only handle Crossings at the ends of RoadSegments.
			//      Zebra crossings require either a UniNode or a different approach entirely.
			const MultiNode* atNode = StreetDirectory::FindNearestMultiNode(*segIt, cr);
			if (!atNode) {
				//TODO: We have a UniNode with a crossing; we should really add this later.
				Warn() <<"Warning: Road Segment has a Crossing, but neither a start nor end MultiNode. Skipping for now." <<std::endl;
				continue;
			}

			//Crossings must from one Lane to another Lane in order to be useful.
			//Therefore, we need to find the "reverse" road segment to this one.
			//This can technically be the same segment, if it's a one-way street (it'll have a different laneID).
			//We will still use the same "from/to" syntax to keep things simple.
			const RoadSegment* fromSeg = *segIt;
			int fromLane = -1;
			for (size_t i=0; i<fromSeg->getLanes().size(); i++) {
				if (fromSeg->getLanes().at(i)->is_pedestrian_lane()) {
					fromLane = i;
					break;
				}
			}
			if (fromLane==-1) { throw std::runtime_error("Sanity check failed: Crossing should not be generated with no sidewalk lane."); }

			//Now find the "to" lane. This is optional.
			//TODO: This all needs to be stored at a higher level later; a Crossing doesn't always have to cross Segments with
			//      the same start/end Nodes.
			const RoadSegment* toSeg = nullptr;
			int toLane = -1;
			for (set<RoadSegment*>::const_iterator it=atNode->getRoadSegments().begin(); toLane==-1 && it!=atNode->getRoadSegments().end(); it++) {
				//Light matching criteria
				toSeg = *it;
				if ((toSeg->getStart()==fromSeg->getStart() && toSeg->getEnd()==fromSeg->getEnd()) ||
					(toSeg->getStart()==fromSeg->getEnd() && toSeg->getEnd()==fromSeg->getStart())) {
					//Scan lanes until we find an empty one (this covers the case where fromSeg and toSeg are the same).
					for (size_t i=0; i<toSeg->getLanes().size(); i++) {
						if (toSeg->getLanes().at(i)->is_pedestrian_lane()) {
							//Avoid adding the exact same from/to pair:
							if (fromSeg==toSeg && fromLane==i) {
								continue;
							}

							//It's unique; add it.
							toLane = i;
							break;
						}
					}
				}
			}

			//If we have something, add this crossing as a pair of edges.
			if (toLane!=-1) {
				//First, retrieve the fromVertex and toVertex
				std::pair<StreetDirectory::Vertex, bool> fromVertex;
				fromVertex.second = false;
				std::pair<StreetDirectory::Vertex, bool> toVertex;
				toVertex.second = false;
				map<const Node*, VertexLookup>::const_iterator vertCandidates = nodeLookup.find(atNode);
				if (vertCandidates==nodeLookup.end()) {
					throw std::runtime_error("Intersection's Node is unknown by the vertex map.");
				}

				//Find the "from" and "to" segments' associated end vertices.
				//In this case, we only need a weak guarantee (e.g., that ONE of the before/after pair matches our segment).
				//(But we also need the strong guarantee of Lane IDs).
				for (vector<NodeDescriptor>::const_iterator ndIt=vertCandidates->second.vertices.begin(); ndIt!=vertCandidates->second.vertices.end(); ndIt++) {
					if ((fromSeg==ndIt->before && fromLane==ndIt->beforeLaneID) || (fromSeg==ndIt->after && fromLane==ndIt->afterLaneID)) {
						fromVertex.first = ndIt->v;
						fromVertex.second = true;
					}
					if ((toSeg==ndIt->before && toLane==ndIt->beforeLaneID) || (toSeg==ndIt->after && toLane==ndIt->afterLaneID)) {
						toVertex.first = ndIt->v;
						toVertex.second = true;
					}
				}

				//Ensure we have both
				if (!fromVertex.second || !toVertex.second) {
					throw std::runtime_error("Crossing has no associated vertex.");
				}

				//Estimate the length of the crossing.
				double length = sim_mob::dist(cr->nearLine.first, cr->nearLine.second);

				//Create an edge.
				{
				StreetDirectory::Edge edge;
				bool ok;
				boost::tie(edge, ok) = boost::add_edge(fromVertex.first, toVertex.first, graph);
				boost::put(boost::edge_name, graph, edge, WayPoint(cr));
				boost::put(boost::edge_weight, graph, edge, length);
				}

				//Create the reverse edge
				{
				StreetDirectory::Edge edge;
				bool ok;
				WayPoint revWP(cr);
				revWP.directionReverse = true;
				boost::tie(edge, ok) = boost::add_edge(toVertex.first, fromVertex.first, graph);
				boost::put(boost::edge_name, graph, edge, revWP);
				boost::put(boost::edge_weight, graph, edge, length);
				}
			}
		}
	}
}


void sim_mob::A_StarShortestPathImpl::procAddStartNodesAndEdges(StreetDirectory::Graph& graph, const map<const Node*, VertexLookup>& allNodes, map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >& resLookup)
{
	//This one's easy: Add a single Vertex to represent the "center" of each Node, and add outgoing edges (one-way only) to each Vertex that Node knows about.
	//Such "master" vertices are used for path finding; e.g., "go from Node X to node Y".
	for (std::map<const Node*, VertexLookup>::const_iterator it=allNodes.begin(); it!=allNodes.end(); it++) {
		//Add the master vertices.
		StreetDirectory::Vertex source = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
		StreetDirectory::Vertex sink = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
		resLookup[it->first] = std::make_pair(source, sink);
		boost::put(boost::vertex_name, graph, source, it->first->location);
		boost::put(boost::vertex_name, graph, sink, it->first->location);

		//Link to each child vertex. Assume a trivial distance.
		for (std::vector<NodeDescriptor>::const_iterator it2=it->second.vertices.begin(); it2!=it->second.vertices.end(); it2++) {
			{
			//From source to "other"
				StreetDirectory::Edge edge;
			bool ok;
			boost::tie(edge, ok) = boost::add_edge(source, it2->v, graph);
			boost::put(boost::edge_name, graph, edge, WayPoint(it->first));
			boost::put(boost::edge_weight, graph, edge, 1);
			}
			{
			//From "other" to sink
			StreetDirectory::Edge edge;
			bool ok;
			WayPoint revWP(it->first);
			revWP.directionReverse = true;
			boost::tie(edge, ok) = boost::add_edge(it2->v, sink, graph);
			boost::put(boost::edge_name, graph, edge, revWP);
			boost::put(boost::edge_weight, graph, edge, 1);
			}
		}
	}
}



void sim_mob::A_StarShortestPathImpl::updateEdgeProperty()
{
	double avgSpeed, travelTime;
	std::map<const RoadSegment*, double>::iterator avgSpeedRSMapIt;
	StreetDirectory::Graph::edge_iterator iter, end;
	for (boost::tie(iter, end) = boost::edges(drivingMap_); iter != end; ++iter)
	{
//		std::cout<<"edge"<<std::endl;
		StreetDirectory::Edge e = *iter;
		WayPoint wp = boost::get(boost::edge_name, drivingMap_, e);
		if (wp.type_ != WayPoint::ROAD_SEGMENT)
			continue;
		const RoadSegment * rs = wp.roadSegment_;
		avgSpeedRSMapIt = sim_mob::TrafficWatch::instance().getAvgSpeedRS().find(rs);
		if(avgSpeedRSMapIt != sim_mob::TrafficWatch::instance().getAvgSpeedRS().end())
			avgSpeed = avgSpeedRSMapIt->second;
		else
			avgSpeed = 100*rs->maxSpeed/3.6;
		if(avgSpeed<=0)
			avgSpeed = 10;
		travelTime = rs->length / avgSpeed;
		boost::put(boost::edge_weight, drivingMap_, e, travelTime);
	}
}


StreetDirectory::VertexDesc sim_mob::A_StarShortestPathImpl::DrivingVertex(const Node& n) const
{
	StreetDirectory::VertexDesc res;

    //Convert the node (position in 2D geometry) to a vertex in the map.
	//It is possible that fromNode and toNode are not represented by any vertex in the graph.
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator vertexIt = drivingNodeLookup_.find(&n);
    if (vertexIt!=drivingNodeLookup_.end()) {
    	res.valid = true;
    	res.source = vertexIt->second.first;
    	res.sink = vertexIt->second.second;
    	return res;
    }

	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
	//  does not, it is not an error (but it means no path can possibly be found).
    return res;
}

StreetDirectory::VertexDesc sim_mob::A_StarShortestPathImpl::WalkingVertex(const Node& n) const
{
	StreetDirectory::VertexDesc res;

    //Convert the node (position in 2D geometry) to a vertex in the map.
	//It is possible that fromNode and toNode are not represented by any vertex in the graph.
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator vertexIt = walkingNodeLookup_.find(&n);
    if (vertexIt!=walkingNodeLookup_.end()) {
    	res.valid = true;
    	res.source = vertexIt->second.first;
    	res.sink = vertexIt->second.second;
    	return res;
    }

	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
	//  does not, it is not an error (but it means no path can possibly be found).
    return res;
}

StreetDirectory::VertexDesc sim_mob::A_StarShortestPathImpl::DrivingVertex(const BusStop& b) const
{
	StreetDirectory::VertexDesc res;

    //Convert the node (position in 2D geometry) to a vertex in the map.
	//It is possible that fromNode and toNode are not represented by any vertex in the graph.
	map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator vertexIt = drivingBusStopLookup_.find(&b);
    if (vertexIt!=drivingBusStopLookup_.end()) {
    	res.valid = true;
    	res.source = vertexIt->second.first;
    	res.sink = vertexIt->second.second;
    	return res;
    }

	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
	//  does not, it is not an error (but it means no path can possibly be found).
    return res;
}

StreetDirectory::VertexDesc sim_mob::A_StarShortestPathImpl::WalkingVertex(const BusStop& b) const
{
	StreetDirectory::VertexDesc res;

    //Convert the node (position in 2D geometry) to a vertex in the map.
	//It is possible that fromNode and toNode are not represented by any vertex in the graph.
	map<const BusStop*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator vertexIt = walkingBusStopLookup_.find(&b);
    if (vertexIt!=walkingBusStopLookup_.end()) {
    	res.valid = true;
    	res.source = vertexIt->second.first;
    	res.sink = vertexIt->second.second;
    	return res;
    }

	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
	//  does not, it is not an error (but it means no path can possibly be found).
    return res;
}

vector<WayPoint> sim_mob::A_StarShortestPathImpl::GetShortestDrivingPath(StreetDirectory::VertexDesc from, StreetDirectory::VertexDesc to, vector<const RoadSegment*> blacklist) const
{
	//Anything invalid?
	if (!(from.valid && to.valid)) {
		return vector<WayPoint>();
	}

	//Same vertex?
	StreetDirectory::Vertex fromV = from.source;
	StreetDirectory::Vertex toV = to.sink;
	if (fromV==toV) {
		return vector<WayPoint>();
	}

	initCache(); //inits cache if it does not exist
	const Point2D from_pt = boost::get(boost::vertex_name, drivingMap_, fromV);
	const Point2D to_pt = boost::get(boost::vertex_name, drivingMap_, toV);
	std::stringstream keystr;
	keystr << from_pt.getX() << from_pt.getY() << to_pt.getX() << to_pt.getY();
	std::string key = keystr.str();

	//Convert the blacklist into a list of blocked Vertices.
	set<StreetDirectory::Edge> blacklistV;
	for (vector<const RoadSegment*>::iterator it=blacklist.begin(); it!=blacklist.end(); it++) {
		std::map<const RoadSegment*, std::set<StreetDirectory::Edge> >::const_iterator lookIt = drivingSegmentLookup_.find(*it);
		if (lookIt!=drivingSegmentLookup_.end()) {
			blacklistV.insert(lookIt->second.begin(), lookIt->second.end());
		}
	}

    //NOTE: choiceSet[] is an interesting optimization, but we don't need to save cycles (and we definitely need to save memory).
    //      The within-day choice set model should have this kind of optimization; for us, we will simply search each time.
    //TODO: Perhaps caching the most recent X searches might be a good idea, though. ~Seth.
	if (blacklistV.empty()) {
		boost::unordered_map< std::string, std::vector<WayPoint> >::const_iterator itr = cache.get()->find(key);
		if(itr != cache.get()->end()){
			return itr->second;
		}
		else{
			vector<WayPoint> data = searchShortestPath(drivingMap_, fromV, toV);
			cache.get()->insert(std::make_pair(key, data));
			return data;
		}
	} else {
		return searchShortestPathWithBlacklist(drivingMap_, fromV, toV, blacklistV);
	}
}


vector<WayPoint> sim_mob::A_StarShortestPathImpl::GetShortestWalkingPath(StreetDirectory::VertexDesc from, StreetDirectory::VertexDesc to) const
{
	//Anything invalid?
	if (!(from.valid && to.valid)) {
		return vector<WayPoint>();
	}

	//Same vertex?
	StreetDirectory::Vertex fromV = from.source;
	StreetDirectory::Vertex toV = to.sink;
	if (fromV==toV) {
		return vector<WayPoint>();
	}

    //NOTE: choiceSet[] is an interesting optimization, but we don't need to save cycles (and we definitely need to save memory).
    //      The within-day choice set model should have this kind of optimization; for us, we will simply search each time.
    //TODO: Perhaps caching the most recent X searches might be a good idea, though. ~Seth.
    return searchShortestPath(walkingMap_, fromV, toV);
}




//Perform A* search, but modify the graph before/after to avoid a blacklist (and then restore it). Also checks if a blacklisted element
// was eventually chosen (this means there's no alternative) and clears the results list in that case.
//NOTE: This function and searchShortestPath() perform locking, but ONLY with respect to each other, and ONLY
//      if the blacklist function is called (as a writer). This *should* incur no penalty when reading.
std::vector<WayPoint> sim_mob::A_StarShortestPathImpl::searchShortestPathWithBlacklist(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex, const std::set<StreetDirectory::Edge>& blacklist) const
{
	//Lock for upgradable access, then upgrade to exclusive access.
	//NOTE: Locking is probably not necessary, but for now I'd rather just be extra-safe.
	boost::upgrade_lock<boost::shared_mutex> lock(GraphSearchMutex_);
	boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

	//Filter it.
	blacklist_edge_constraint filter(blacklist);
	boost::filtered_graph<StreetDirectory::Graph, blacklist_edge_constraint> filtered(graph, filter);


	////////////////////////////////////////
	// TODO: This code is copied (since filtered_graph is not the same as adjacency_list) from searchShortestPath.
	////////////////////////////////////////
	vector<WayPoint> res;
	std::list<StreetDirectory::Vertex> partialRes;

	vector<StreetDirectory::Vertex> p(boost::num_vertices(filtered));  //Output variable
	vector<double> d(boost::num_vertices(filtered));  //Output variable

	//Use A* to search for a path
	//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
	//...which is available under the terms of the Boost Software License, 1.0
	try {
		boost::astar_search(
				filtered,
			fromVertex,
			distance_heuristic_filtered(&filtered, toVertex),
			boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(astar_goal_visitor(toVertex))
		);
	} catch (found_goal& goal) {
		//Build backwards.
		for (StreetDirectory::Vertex v=toVertex;;v=p[v]) {
			partialRes.push_front(v);
		    if(p[v] == v) {
		    	break;
		    }
		}

		//Now build forwards.
		std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
		for (std::list<StreetDirectory::Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
			//Add this edge.
			if (prev!=partialRes.end()) {
				//This shouldn't fail.
				std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, filtered);
				if (!edge.second) {
					Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
					return std::vector<WayPoint>();
				}

				//Retrieve, add this edge's WayPoint.
				WayPoint wp = boost::get(boost::edge_name, filtered, edge.first);
				res.push_back(wp);
			}

			//Save for later.
			prev = it;
		}
	}

	/////////////////////////////
	// END COPIED CODE
	/////////////////////////////
	return res;
}



//Perform an A* search of our graph
//NOTE: This function and searchShortestPathWithBlacklist() perform locking, but ONLY with respect to each other, and ONLY
//      if the blacklist function is called (as a writer). This *should* incur no penalty when reading.
vector<WayPoint> sim_mob::A_StarShortestPathImpl::searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex) const
{
	vector<WayPoint> res;
	std::list<StreetDirectory::Vertex> partialRes;

	//Lock for read access.
	{
	boost::shared_lock<boost::shared_mutex> lock(GraphSearchMutex_);

	//Use A* to search for a path
	//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
	//...which is available under the terms of the Boost Software License, 1.0
	vector<StreetDirectory::Vertex> p(boost::num_vertices(graph));  //Output variable
	vector<double> d(boost::num_vertices(graph));  //Output variable
	try {
		boost::astar_search(
			graph,
			fromVertex,
			distance_heuristic_graph(&graph, toVertex),
			boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(astar_goal_visitor(toVertex))
		);
	} catch (found_goal& goal) {
		//Build backwards.
		for (StreetDirectory::Vertex v=toVertex;;v=p[v]) {
			partialRes.push_front(v);
		    if(p[v] == v) {
		    	break;
		    }
		}

		//Now build forwards.
		std::list<StreetDirectory::Vertex>::const_iterator prev = partialRes.end();
		for (std::list<StreetDirectory::Vertex>::const_iterator it=partialRes.begin(); it!=partialRes.end(); it++) {
			//Add this edge.
			if (prev!=partialRes.end()) {
				//This shouldn't fail.
				std::pair<StreetDirectory::Edge, bool> edge = boost::edge(*prev, *it, graph);
				if (!edge.second) {
					Warn() <<"ERROR: Boost can't find an edge that it should know about." <<std::endl;
					return std::vector<WayPoint>();
				}

				//Retrieve, add this edge's WayPoint.
				WayPoint wp = boost::get(boost::edge_name, graph, edge.first);
				res.push_back(wp);
			}

			//Save for later.
			prev = it;
		}
	}
	} //End boost mutex lock for read access.

	return res;
}





map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >::const_iterator
sim_mob::A_StarShortestPathImpl::searchVertex(const map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >& srcNodes, const Point2D& point) const
{
	typedef  map<const Node*, std::pair<StreetDirectory::Vertex,StreetDirectory::Vertex> >::const_iterator  NodeLookupIter;

	double minDist = std::numeric_limits<double>::max();
	NodeLookupIter minItem = srcNodes.end();
	for (NodeLookupIter it=srcNodes.begin(); it!=srcNodes.end(); it++) {
		double currDist = sim_mob::dist(point, it->first->getLocation());
		if (currDist < minDist) {
			minDist = currDist;
			minItem = it;
		}
	}

	return minItem;
}


bool sim_mob::A_StarShortestPathImpl::checkIfExist(std::vector<std::vector<WayPoint> > & paths, std::vector<WayPoint> & path)
{
	for(size_t i=0;i<paths.size();i++)
	{
		std::vector<WayPoint> temp = paths.at(i);
		if(temp.size()!=path.size())
			continue;
		bool same = true;
		for(size_t j=0;j<temp.size();j++)
		{
			if(temp.at(j).roadSegment_ != path.at(j).roadSegment_)
			{
				same = false;
				break;
			}
		}
		if(same)
			return true;
	}
	return false;
}



void sim_mob::A_StarShortestPathImpl::printDrivingGraph(std::ostream& outFile) const
{
	printGraph(outFile, "driving", drivingMap_);
}

void sim_mob::A_StarShortestPathImpl::printWalkingGraph(std::ostream& outFile) const
{
	printGraph(outFile, "walking", walkingMap_);
}



void sim_mob::A_StarShortestPathImpl::printGraph(std::ostream& outFile, const std::string& graphType, const StreetDirectory::Graph& graph) const
{
	//Print an identifier
	outFile <<"(\"sd-graph\""
		<<","<<0
		<<","<<&graph
		<<",{"
		<<"\"type\":\""<<graphType
		<<"\"})"<<std::endl;

	//Print each vertex
	//NOTE: Vertices appear to just be integers in boost's adjacency lists.
	//      Not sure if we can rely on this (we can use property maps if necessary).
	{
    StreetDirectory::Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter) {
    	StreetDirectory::Vertex v = *iter;
    	const Point2D pt = boost::get(boost::vertex_name, graph, v);
    	outFile <<"(\"sd-vertex\""
    		<<","<<0
    		<<","<<v
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"xPos\":\""<<pt.getX()
    		<<"\",\"yPos\":\""<<pt.getY()
    		<<"\"})"<<std::endl;
    }
	}

    //Print each edge
	//NOTE: Edges are currently identified by their "from/to" nodes (as a pair), so we'll just make up a
	//      suitable ID for them (it doesn't actually matter).
    {
    	StreetDirectory::Graph::edge_iterator iter, end;
    unsigned int id=0;
    for (boost::tie(iter, end) = boost::edges(graph); iter != end; ++iter) {
    	StreetDirectory::Edge ed = *iter;
    	StreetDirectory::Vertex srcV = boost::source(ed, graph);
    	StreetDirectory::Vertex destV = boost::target(ed, graph);
    	outFile <<"(\"sd-edge\""
    		<<","<<0
    		<<","<<id++
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"fromVertex\":\""<<srcV
    		<<"\",\"toVertex\":\""<<destV
    		<<"\"})"<<std::endl;
    }
    }
}


