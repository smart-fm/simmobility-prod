/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "A_StarShortestPathImpl.hpp"

#include <cmath>

//TODO: Prune this include list later; it was copied directly from StreetDirectory.cpp
#include "buffering/Vector2D.hpp"
#include "entities/TrafficWatch.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/ZebraCrossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "util/OutputUtil.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


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

namespace {


//Retrieve the Bus Stop's position along the RoadSegment, accounting for the offset and
//  all polylines.
//TODO: This is somewhat backwards; we actually start with the BusStop's position, and need to
//      calculate its normal intersection with the nearest polyline segment. ~Seth
/*Point2D GetBusStopPosition(const RoadSegment* road, centimeter_t offset) {
	const vector<Lane*>& lanes = road->getLanes();
	const Lane* outerLane = lanes[lanes.size() - 1];

	// Walk along the left lane and stop after we have walked <offset> centimeters.
	// The bus stop should be at that point.
	const vector<Point2D>& polyline = outerLane->getPolyline();
	Point2D p1;
	Point2D p2;
	double len;

	for (size_t i=0; i < polyline.size() - 1;) {
		p1 = polyline[i];
		p2 = polyline[i + 1];
		len = sqrt(sim_mob::dist(p1, p2));
		if (offset - len < 0) {
			//Walking to <p2> would be a bit too far.
			break;
		}

		offset -= len;
		++i;
	}

	// Walk to the point before <p2> that would end at <offset>.
	double ratio = offset / len;
	centimeter_t x = p1.getX() + ratio * (p2.getX() - p1.getX());
	centimeter_t y = p1.getY() + ratio * (p2.getY() - p1.getY());
	return Point2D(x, y);
}*/

// Return true if <p1> and <p2> are within <distance>.  Quick and dirty calculation of the
// distance between <p1> and <p2>.
/*bool IsCloseBy(const Point2D& p1, const Point2D& p2, centimeter_t distance) {
	return     (std::abs(p1.getX() - p2.getX()) < distance)
			&& (std::abs(p1.getY() - p2.getY()) < distance);
}*/

} //End un-named namespace



//Helper: Add an edge, approximate the distance.
void sim_mob::A_StarShortestPathImpl::AddSimpleEdge(StreetDirectory::Graph& graph, StreetDirectory::Vertex& fromV, StreetDirectory::Vertex& toV, sim_mob::WayPoint wp)
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
    	procAddDrivingLinks(drivingMap_, (*iter)->getPath(), nodeLookup);
    }

    //Now add all Intersection edges (lane connectors)
    for (map<const Node*, VertexLookup>::const_iterator it=nodeLookup.begin(); it!=nodeLookup.end(); it++) {
    	procAddDrivingLaneConnectors(drivingMap_, dynamic_cast<const MultiNode*>(it->first), nodeLookup);
    }

    //Now add BusStops (this mutates the network slightly, by segmenting Edges where a BusStop is located).
    for (vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingBusStops(drivingMap_, (*iter)->getPath(), nodeLookup);
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


void sim_mob::A_StarShortestPathImpl::procAddDrivingBusStops(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup)
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

			//At this point, we have the Bus Stop, as well as the Road Segment that it appears on.
			//We need to do two things:
			//  1) Segment the current Edge into two smaller edges; one before the BusStop and one after.
			//  2) Add a Vertex for the stop itself, and connect an incoming and outgoing Edge to it.
			//Note that both of these tasks require calculating normal intersection of the BusStop and the RoadSegment.
			Point2D bstopPoint(bstop->xPos, bstop->yPos);
			DynamicVector roadSegVec(rs->getStart()->location, rs->getEnd()->location);
			Point2D newSegPt;

			//For now, this is optional.
			try {
				newSegPt = normal_intersect(bstopPoint, roadSegVec);
			} catch (std::exception& ex) {
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

			//Retrieve the original "start" and "end" Nodes for this segment.
			StreetDirectory::Vertex fromVertex = FindStartingVertex(rs, nodeLookup);
			StreetDirectory::Vertex toVertex = FindEndingVertex(rs, nodeLookup);

			//Add the BusStop vertex. This node is unique per BusStop per SEGMENT, since it allows a loopback.
			//For  now, it makes no sense to put a path to the Bus Stop on the reverse segment (cars need to park on
			// the correct side of the road), but for path finding we might want to consider it later.
			//TODO: This will require a lookup
			StreetDirectory::Vertex busV = boost::add_vertex(const_cast<StreetDirectory::Graph &>(graph));
			boost::put(boost::vertex_name, const_cast<StreetDirectory::Graph &>(graph), busV, bstopPoint);

			//Add the new route. (from->mid->bus->mid->to)
			AddSimpleEdge(graph, fromVertex, midV, WayPoint(rs));
			AddSimpleEdge(graph, midV, busV, WayPoint(bstop));
			AddSimpleEdge(graph, busV, midV, WayPoint(bstop));
			AddSimpleEdge(graph, midV, toVertex, WayPoint(rs));
		}
	}
}


void sim_mob::A_StarShortestPathImpl::procAddDrivingLinks(StreetDirectory::Graph& graph, const vector<RoadSegment*>& roadway, const map<const Node*, VertexLookup>& nodeLookup)
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
			std::cout <<"Warning: Road Segment's nodes have no known mapped vertices (1)." <<std::endl;
			continue;
		}

		//For simply nodes, this will be sufficient.
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

				//We'll create a fake Node for this location (so it'll be represented properly). Once we've fully switched to the
				//  new algorithm, we'll have to switch this to a value-based type; using "new" will leak memory.
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
			std::cout <<"Warning: Road Segment's nodes have no known mapped vertices (2)." <<std::endl;
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
				std::cout <<"Warning: Road Segment has a Crossing, but neither a start nor end MultiNode. Skipping for now." <<std::endl;
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

vector<WayPoint> sim_mob::A_StarShortestPathImpl::GetShortestDrivingPath(const Node& fromNode, const Node& toNode) const
{
    if (&fromNode == &toNode) {
        return std::vector<WayPoint>();
    }

    // Convert the fromNode and toNode (positions in 2D geometry) to vertices in the drivingMap_
    // graph.  It is possible that fromNode and toNode are not represented by any vertex in the
    // graph.
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator fromVertexIt = drivingNodeLookup_.find(&fromNode);
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator toVertexIt = drivingNodeLookup_.find(&toNode);

    // If fromNode and toNode are not represented by any vertex in the graph, then throw an
    // error message.
    if (fromVertexIt==drivingNodeLookup_.end() || toVertexIt==drivingNodeLookup_.end()) {
    	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
    	//  does not, it is not an error (but it means no path can possibly be found).
    	return std::vector<WayPoint>();
    }

    //NOTE: choiceSet[] is an interesting optimization, but we don't need to save cycles (and we definitely need to save memory).
    //      The within-day choice set model should have this kind of optimization; for us, we will simply search each time.
    //TODO: Perhaps caching the most recent X searches might be a good idea, though. ~Seth.
    return searchShortestPath(drivingMap_, fromVertexIt->second.first, toVertexIt->second.second);
}



//Perform an A* search of our graph
vector<WayPoint> sim_mob::A_StarShortestPathImpl::searchShortestPath(const StreetDirectory::Graph& graph, const StreetDirectory::Vertex& fromVertex, const StreetDirectory::Vertex& toVertex) const
{
	vector<WayPoint> res;
	std::list<StreetDirectory::Vertex> partialRes;

	//Use A* to search for a path
	//Taken from: http://www.boost.org/doc/libs/1_38_0/libs/graph/example/astar-cities.cpp
	//...which is available under the terms of the Boost Software License, 1.0
	vector<StreetDirectory::Vertex> p(boost::num_vertices(graph));  //Output variable
	vector<double> d(boost::num_vertices(graph));  //Output variable
	try {
		boost::astar_search(
			graph,
			fromVertex,
			distance_heuristic<StreetDirectory::Graph, double>(&graph, toVertex),
			boost::predecessor_map(&p[0]).distance_map(&d[0]).visitor(astar_goal_visitor<StreetDirectory::Vertex>(toVertex))
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
					LogOut("ERROR: Boost can't find an edge that it should know about." <<std::endl);
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


vector<WayPoint> sim_mob::A_StarShortestPathImpl::shortestWalkingPath(const Point2D& fromPoint, const Point2D& toPoint) const
{
    if (fromPoint == toPoint)
        return std::vector<WayPoint>();

    // Convert the fromPoint and toPoint (positions in 2D geometry) to vertices in the walkingMap_
    // graph.  The fromVertex and toVertex corresponds to Node objects that are closest to the
    // fromPoint and toPoint positions in the graph.
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator fromVertexIt = searchVertex(walkingNodeLookup_, fromPoint);
    map<const Node*, std::pair<StreetDirectory::Vertex, StreetDirectory::Vertex> >::const_iterator toVertexIt = searchVertex(walkingNodeLookup_, toPoint);

    //Might not exist.
    if (fromVertexIt==walkingNodeLookup_.end() || toVertexIt==walkingNodeLookup_.end()) {
    	return std::vector<WayPoint>();
    }

    //Shorthand; maintain our sanity
    StreetDirectory::Vertex fromVertex = fromVertexIt->second.first;
    StreetDirectory::Vertex toVertex = toVertexIt->second.second;

    //Find the path between these.
    std::vector<WayPoint> path = searchShortestPath(walkingMap_, fromVertex, toVertex);
    if (path.empty()) {
        return path;
    }

    // If the fromPoint is not exactly located at fromVertex, then we need to insert a Waypoint
    // that directs the pedestrian to move from fromPoint to fromVertex by some undefined mean.
    // Similarly if toPoint is not located exactly at toVertex, then we append a WayPpint to
    // move from tiVertex to toPoint.
    std::vector<WayPoint> result;
    const double MIN_SPLIT_DIST = 5 * 100; //If the user has to walk more than 5m, consider it a new point in the graph.
    Point2D fromNodePt = boost::get(boost::vertex_name, walkingMap_, fromVertex);
    if (sim_mob::dist(fromNodePt, fromPoint) > MIN_SPLIT_DIST) {
    	result.push_back(WayPoint(fromNodePt));
    }
    result.insert(result.end(), path.begin(), path.end());
    Point2D toNodePt = boost::get(boost::vertex_name, walkingMap_, toVertex);
    if (sim_mob::dist(toNodePt, toPoint) > MIN_SPLIT_DIST) {
        result.push_back(WayPoint(toNodePt));
    }
    return result;
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



void sim_mob::A_StarShortestPathImpl::printDrivingGraph() const
{
	printGraph("driving", drivingMap_);
}

void sim_mob::A_StarShortestPathImpl::printWalkingGraph() const
{
	printGraph("walking", walkingMap_);
}



void sim_mob::A_StarShortestPathImpl::printGraph(const std::string& graphType, const StreetDirectory::Graph& graph) const
{
	//Print an identifier
	LogOutNotSync("(\"sd-graph\""
		<<","<<0
		<<","<<&graph
		<<",{"
		<<"\"type\":\""<<graphType
		<<"\"})"<<std::endl);

	//Print each vertex
	//NOTE: Vertices appear to just be integers in boost's adjacency lists.
	//      Not sure if we can rely on this (we can use property maps if necessary).
	{
    StreetDirectory::Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter) {
    	StreetDirectory::Vertex v = *iter;
    	const Point2D pt = boost::get(boost::vertex_name, graph, v);
    	LogOutNotSync("(\"sd-vertex\""
    		<<","<<0
    		<<","<<v
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"xPos\":\""<<pt.getX()
    		<<"\",\"yPos\":\""<<pt.getY()
    		<<"\"})"<<std::endl);
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
    	LogOutNotSync("(\"sd-edge\""
    		<<","<<0
    		<<","<<id++
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"fromVertex\":\""<<srcV
    		<<"\",\"toVertex\":\""<<destV
    		<<"\"})"<<std::endl);
    }
    }
}



/******************************************************************************************************************************
 * NOTE:
 ******************************************************************************************************************************
 * The following comments contain old code from when StreetDirectoryImpl was using a different DAG structure and lookup.
 * Normally, we would delete commented code. However, please DO NOT delete this until we are sure that all functionality
 * has been accounted for. In particular:
 *    1) BusStops used to be part of the vertex graph. Unfortunately, this would cause vehicles on "segment X" to get the following
 *       three WayPoints in order: WayPoint("segment X"), WayPoint("bus stop Y"), WayPoint("segment X")  ---the second
 *       Segment WayPoint corresponded to the remaining Segment after the BusStop. This was causing all sorts of errors.
 *       Make sure we can look up BusStops (somehow; doesn't have to be this way exactly) before deleting this code.
 *    2) Similar to the BusStop case, consider what would happen if there was a ZebraCrossing in the middle of a RoadSegment
 *       (i.e., there's no UniNode there, just a random crossing). In this case, the returned path for any route will involve the
 *       RoadSegment only ONCE (either both "halves" will be combined, if the Zebra Crossing is not taken, or there will be one
 *       half or the other, if the Zebra Crossing IS taken). Make sure we can handle this intelligently (just put it as a filter
 *       when we are building the resulting WayPoint result set).
 *    3) The previous algorithm used was Dijkstra's algorithm, which builds a single lookup table for the entire Network.
 *       While this had the advantage of allowing very fast (origin->dest) lookup, it would be infeasibly for large networks
 *       due to memory requirements. We should provide some kind of (origin->dest) cache; however, it's not so simple, since
 *       we will have multiple agents accessing this function in a multi-threaded manner. The only viable to solution is to
 *       store a temporary cache of (origin->dest) lookups at the *Worker* level, and dispatch to StreetDirectory only
 *       if the Worker can't find the given path in its cache (and then it adds it, of course).
 *
 * Thanks,
 * Seth
 *****************************************************************************************************************************/


#define HIDE_OLD_CODE

#ifndef HIDE_OLD_CODE

// Search the graph for a vertex located at a Node that is within <distance> from <point>
// and return that Node, if any; otherwise return 0.
Node const * StreetDirectory::ShortestPathImpl::findVertex(Graph const & graph, Point2D const & point,
                                              centimeter_t distance) const
{
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, graph, v);
        if (closeBy(node->location, point, distance))
            return node;
    }
    return nullptr;
}

// If there is a Node in the drivingMap_ that is within 0.5 meter from <point>, return it;
// otherwise return a new "internal" node located at <point>.  "Internal" means the node exists
// only in the StreetDirectory::ShortestPathImpl object to be used as a location for a vertex.
//
// This function is a hack.  We can't use uni-nodes as vertices in the drivingMap_ graph and
// yet the lanes polylines are not "connected".  That is, the last point of the lane's polyline
// in one road-segment is not the first point of the lane's polyline in the next road-segment.
// I think the gap is small, hopefully it is less than 0.5 meter.
Node const * StreetDirectory::ShortestPathImpl::findNode(Point2D const & point)
{
    Node const * node = findVertex(drivingMap_, point, 50);
    if (node)
        return node;

    Node * n = new UniNode(point.getX(), point.getY());
    nodes_.push_back(n);
    return n;
}

// Build up the drivingMap_ and walkingMap_ graphs.
//
// Road segments are the only edges in the drivingMap_ graph.  Multi-nodes are inserted as
// vertices in the graph; it is assumed that vehicles are allowed to make U-turns at the
// intersections if the link is bi-directional.  Uni-nodes in bi-directional links are split into
// 2 vertices, without any edge between the 2 vertices; at the current moment, vehicles are not
// allowed to make U-turns at uni-nodes.  We need to fix this; we need to add an edge from one
// vertex to the other if traffic is allowed to move in that direction.  The edge length would be
// 0 since the 2 vertices are located at the same uni-node.
//
// For the walkingMap_ graph, the side walks in each road segment, crossings at the
// intersections, and zebra crossings are edges.  Since pedestrians are not supposed to be
// walking on the road dividers between 2 side-by-side road-segments, the road divider is
// not treated as an edge.  Hence each link, whether bidirectional or one-way, has exactly
// 2 side walk edges.
//
// If there is a bus stop, the road segment and the side walk is split into 2 edges, split at
// the bus stop.  Therefore shortestDrivingPath() would include a bus stop as a waypoint,
// which would be useful for the BusDriver model but may not be needed by the Driver model.
// TODO: The above may need to change. ~Seth
//
// We assume that if a link has any crossing (signalized or zebra crossing), the link is
// split into several road-segments, split at the crossing.  Hence we assume that the
// crossing at the beginning or end of a road segment.  Therefore, side walks are not split
// by crossings.  This assumption needs to be reviewed.
// TODO: Zebra crossings complicate this; we should never have the same segment twice in a row in
//       a returned path. ~Seth

inline void
StreetDirectory::ShortestPathImpl::process(std::vector<RoadSegment*> const & roads, bool isForward)
{
	for (size_t i = 0; i < roads.size(); ++i)
	{
		linkCrossingToRoadSegment(const_cast<RoadSegment*>(roads[i]),isForward);
		process(roads[i], isForward);
	}
}

void StreetDirectory::ShortestPathImpl::initNetworkOld(const std::vector<Link*>& links)
{
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter)
    {
        Link const * link = *iter;
        process(link->getPath(true), true);
        process(link->getPath(false), false);
    }
}

void StreetDirectory::ShortestPathImpl::linkCrossingToRoadSegment(RoadSegment *road, bool isForward)
{
	centimeter_t offset = 0;
	while (offset < road->length)
	{
		RoadItemAndOffsetPair pair = road->nextObstacle(offset, isForward);
		if (0 == pair.item)
		{
			offset = road->length;
			break;
		}


		if (Crossing * crossing = const_cast<Crossing*>(dynamic_cast<Crossing const *>(pair.item)))
		{
			crossing->setRoadSegment(road);
		}

		offset = pair.offset + 1;
	}
}


void
StreetDirectory::ShortestPathImpl::process(RoadSegment const * road, bool isForward)
{
	double avgSpeed;
	std::map<const RoadSegment*, double>::iterator avgSpeedRSMapIt;
    // If this road-segment is inside a one-way Link, then there should be 2 side-walks.
    std::vector<Lane*> const & lanes = road->getLanes();
    for (size_t i = 0; i < lanes.size(); ++i)
    {
        Lane const * lane = lanes[i];
        if (lane->is_pedestrian_lane())
        {
            addSideWalk(lane, road->length);
        }
    }
    Node const * node1 = road->getStart();
    if (dynamic_cast<UniNode const *>(node1))
    {
        // If this road-segment is side-by-side to another road-segment going in the other
        // direction, we cannot insert this uni-node into the drivingMap_ graph.  Otherwise,
        // vehicles on both road-segments would be allowed to make U-turns at the uni-node,
        // which may not correct.  Currently we do not have info from the database about the
        // U-turns at the uni-nodes.  Instead of using the uni-node as a vertex in the drivingMap_,
        // we choose a point in one of the lane's polyline.
        std::vector<Point2D> const & polyline = road->getLanes()[0]->getPolyline();
        Point2D point = polyline[0];
        node1 = findNode(point);
    }

    centimeter_t offset = 0;
    while (offset < road->length)
    {
        RoadItemAndOffsetPair pair = road->nextObstacle(offset, isForward);
        if (0 == pair.item)
        {
        	offset = road->length;
            break;
        }


        if (Crossing const * crossing = dynamic_cast<Crossing const *>(pair.item))
        {
            // In a bi-directional link, a crossing would span the 2 side-by-side road-segments.
            // Therefore, we may have already inserted this crossing into the walkingMap_ when
            // we process the previous road segment.  We check if it is the graph, continuing only
            // if it wasn't.
            bool notInWalkingMap = true;
            Graph::edge_iterator iter, end;
            for (boost::tie(iter, end) = boost::edges(walkingMap_); iter != end; ++iter)
            {
                Edge e = *iter;
                WayPoint const & wp = boost::get(boost::edge_name, walkingMap_, e);
                if (WayPoint::CROSSING == wp.type_ && crossing == wp.crossing_)
                {
                    notInWalkingMap = false;
                    break;
                }
            }

            if (notInWalkingMap)
            {
                centimeter_t length = sqrt(hypot(crossing->nearLine.first, crossing->nearLine.second));
                addCrossing(crossing, length);
            }
        }
      else if (BusStop const * busStop = dynamic_cast<BusStop const *>(pair.item))
        {
            const Point2D pos = getBusStopPosition(road, offset);
            Node * node2 = new UniNode(pos.getX(), pos.getY());
            addRoadEdge(node1, node2, WayPoint(busStop), offset);


            avgSpeed = 100*road->maxSpeed/3.6;
            if(avgSpeed<=0)
            	avgSpeed = 10;
            addRoadEdgeWithTravelTime(node1, node2, WayPoint(busStop), offset/avgSpeed);
            nodes_.push_back(node2);
            node1 = node2;
        }
        offset = pair.offset + 1;
    }

    Node const * node2 = road->getEnd();
    if (dynamic_cast<UniNode const *>(node2))
    {
        // See comment above about the road-segment's start-node.
    	std::vector<Point2D> const & polyline = road->getLanes()[0]->getPolyline();
        Point2D point = polyline[polyline.size() - 1];
        node2 = findNode(point);
    }


    avgSpeed = 100*road->maxSpeed/3.6;
    if(avgSpeed<=0)
    	avgSpeed = 10;

//    std::cout<<"node1 "<<node1->location.getX()<<" to node2 "<<node2->location.getX()<<" is "<<offset/(100*road->maxSpeed/3.6)<<std::endl;
    addRoadEdge(node1, node2, WayPoint(road), offset);
//    addRoadEdgeWithTravelTime(node1, node2, WayPoint(road), offset/avgSpeed);
}

// Search for <node> in <graph>.  If any vertex in <graph> has <node> attached to it, return it;
// otherwise insert a new vertex (with <node> attached to it) and return it.
StreetDirectory::ShortestPathImpl::Vertex
StreetDirectory::ShortestPathImpl::findVertex(Graph const & graph, Node const * node)
const
{
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * n = boost::get(boost::vertex_name, graph, v);
        if (node == n)
            return v;
    }

    Vertex v = boost::add_vertex(const_cast<Graph &>(graph));
    boost::put(boost::vertex_name, const_cast<Graph &>(graph), v, node);
    return v;
}

// Insert a directed edge into the drivingMap_ graph from <node1> to <node2>, which represent
// vertices in the graph.  <wp> is attached to the edge as its name property and <length> as
// its weight property.
void StreetDirectory::ShortestPathImpl::addRoadEdge(Node const * node1, Node const * node2,
                                               WayPoint const & wp, centimeter_t length)
{
    Vertex u = findVertex(drivingMap_, node1);
    Vertex v = findVertex(drivingMap_, node2);

    Edge edge;
    bool ok;
    boost::tie(edge, ok) = boost::add_edge(u, v, drivingMap_);
    boost::put(boost::edge_name, drivingMap_, edge, wp);
    boost::put(boost::edge_weight, drivingMap_, edge, length);

    //NOTE: With some combinations of boost+gcc+optimizations, this sometimes adds
    //      a null node. Rather than silently crashing, we will directly check the
    //      added value here and explicitly fail if corruption occurred. ~Seth
    //VERY IMPORTANT NOTE: If you are using boost 1.42.0, gcc 4.5.2, and -O2, gcc will
    //     optimize out the previous boost::put and you wiil get a lot of WayPoint::Invalid
    //     edges. The following lines of code ensure that, by checking the value of the inserted
    //     WayPoint, it is not optimized away. This is a bug in gcc, so please do not remove the
    //     following lines of code. ~Seth
    WayPoint cp = boost::get(boost::edge_name, drivingMap_, edge);
    if (cp.type_ != wp.type_) {
    	throw std::runtime_error("StreetDirectory::addRoadEdge; boost::put corrupted data."
    		"This sometimes happens with certain versions of boost, gcc, and optimization level 2.");
    }
}


// Insert a directed edge into the drivingMap_ graph from <node1> to <node2>, which represent
// vertices in the graph.  <wp> is attached to the edge as its name property and <length> as
// its weight property.
void
StreetDirectory::ShortestPathImpl::addRoadEdgeWithTravelTime(Node const * node1, Node const * node2,
                                               WayPoint const & wp, double travelTime)
{
    Vertex u = findVertex(drivingMap_, node1);
    Vertex v = findVertex(drivingMap_, node2);

    Edge edge;
    bool ok;
    boost::tie(edge, ok) = boost::add_edge(u, v, drivingMap_);
    boost::put(boost::edge_name, drivingMap_, edge, wp);
    boost::put(boost::edge_weight, drivingMap_, edge, travelTime);

    //NOTE: With some combinations of boost+gcc+optimizations, this sometimes adds
    //      a null node. Rather than silently crashing, we will directly check the
    //      added value here and explicitly fail if corruption occurred. ~Seth
    //VERY IMPORTANT NOTE: If you are using boost 1.42.0, gcc 4.5.2, and -O2, gcc will
    //     optimize out the previous boost::put and you wiil get a lot of WayPoint::Invalid
    //     edges. The following lines of code ensure that, by checking the value of the inserted
    //     WayPoint, it is not optimized away. This is a bug in gcc, so please do not remove the
    //     following lines of code. ~Seth
    WayPoint cp = boost::get(boost::edge_name, drivingMap_, edge);
    if (cp.type_ != wp.type_) {
    	throw std::runtime_error("StreetDirectory::addRoadEdge; boost::put corrupted data."
    		"This sometimes happens with certain versions of boost, gcc, and optimization level 2.");
    }
}


// If there is a Node in the walkingMap_ that is within 10 meters from <point>, return the
// vertex with that node; otherwise create a new "internal" node located at <point>, insert a
// vertex with the new node, and return the vertex.  "Internal" means the node exists only in the
// StreetDirectory::ShortestPathImpl object to be used as a location for a vertex.
//
// This function is a hack.  The side-walks and crossings are not aligned, that is, they are not
// connected.  The gaps are quite large.  I hope that 10 meters would be a good choice.  It should
// be ok if there really is a crossing at the end of sidewalk (or vice versa, a side-walk at the
// end of the crossing).  But it may be too large that the function incorrectly returns a vertex
// that is on the opposite of a narrow road-segment.
StreetDirectory::ShortestPathImpl::Vertex
StreetDirectory::ShortestPathImpl::findVertex(Point2D const & point)
{
    Node const * node = findVertex(walkingMap_, point, 1000);
    if (!node)
    {
        Node * n = new UniNode(point.getX(), point.getY());
        nodes_.push_back(n);
        node = n;
    }

    return findVertex(walkingMap_, node);
}

// Insert a directed edge into the walkingMap_ graph from one end of <sideWalk> to the other end,
// both ends represent the vertices in the graph.  <sideWalk> is attached to the edge at its name
// property and <length> as its weight property.
//
// Side-walks are bi-directional for pedestrians.  So 2 edges are inserted for the 2 directions.
void
StreetDirectory::ShortestPathImpl::addSideWalk(Lane const * sideWalk, centimeter_t length)
{
    std::vector<Point2D> const & polyline = sideWalk->getPolyline();
    Vertex u = findVertex(polyline[0]);
    Vertex v = findVertex(polyline[polyline.size() - 1]);

    Edge edge;
    bool ok;
    WayPoint wp(sideWalk);

    boost::tie(edge, ok) = boost::add_edge(u, v, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);

    // Side walks are bi-directional for pedestrians.  Add another edge for the other direction.
    wp.directionReverse = true;
    boost::tie(edge, ok) = boost::add_edge(v, u, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);
}

// Insert a directed edge into the walkingMap_ graph from one end of <crossing> to the other end,
// both ends represent the vertices in the graph.  <crossing> is attached to the edge at its name
// property and <length> as its weight property.
//
// Crossings are bi-directional for pedestrians.  So 2 edges are inserted for the 2 directions.
void
StreetDirectory::ShortestPathImpl::addCrossing(Crossing const * crossing, centimeter_t length)
{
    Vertex u = findVertex(crossing->nearLine.first);
    Vertex v = findVertex(crossing->nearLine.second);

    Edge edge;
    bool ok;
    WayPoint wp(crossing);

    boost::tie(edge, ok) = boost::add_edge(u, v, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);

    // Crossings are bi-directional for pedestrians.  Add another edge for the other direction.
    wp.directionReverse = true;
    boost::tie(edge, ok) = boost::add_edge(v, u, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);
}


// Find the vertices in the drivingMap_ graph that represent <fromNode> and <toNode> and return
// them in <fromVertex> and <toVertex> respectively.  If the node is not represented by any vertex,
// then the returned vertex is set to a value larger than the number of vertices in the graph.
void StreetDirectory::ShortestPathImpl::getVertices(Vertex & fromVertex, Vertex & toVertex,
                                               Node const & fromNode, Node const & toNode) const
{
    Graph::vertices_size_type graphSize = boost::num_vertices(drivingMap_);
    fromVertex = graphSize + 1;
    toVertex = graphSize + 1;

    Node const * from = (dynamic_cast<UniNode const *>(&fromNode)) ? &fromNode : nullptr;
    Node const * to = (dynamic_cast<UniNode const *>(&toNode)) ? &toNode : nullptr;

    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(drivingMap_); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, drivingMap_, v);
        // Uni-nodes were never inserted into the drivingMap_, but some other points close to
        // them.  Hopefully, they are within 10 meters and that the correct vertex is returned,
        // even in narrow links.
        if (from && closeBy(from->location, node->location, 1000))
        {
            fromVertex = v;
        }
        else if (to && closeBy(to->location, node->location, 1000))
        {
            toVertex = v;
        }
        else if (node == &fromNode)
        {
            fromVertex = v;
        }
        else if (node == &toNode)
        {
            toVertex = v;
        }
        if (fromVertex < graphSize && toVertex < graphSize)
            break;
    }
}

// Check that <fromVertex> and <toVertex> are valid vertex in the drivingMap_ graph (ie. if they
// are between 0 and the number of vertices in the graph).  If not, throw an error message.
// Since the error message is intended for the modellers and <fromVertex> and <toVertex> are internal
// data, the message is formatted with info from <fromNode> and <toNode>.
// Returns false in some cases to indicate that the nodes exist, but cannot be found.
bool
StreetDirectory::ShortestPathImpl::checkVertices(Vertex const fromVertex, Vertex const toVertex,
                                                 Node const & fromNode, Node const & toNode)
const
{
    Graph::vertices_size_type graphSize = boost::num_vertices(drivingMap_);
    if (fromVertex > graphSize || toVertex > graphSize)
    {
    	bool critical = true;
        std::ostringstream stream;
        stream << "StreetDirectory::shortestDrivingPath: ";
        if (fromVertex > graphSize)
        {
        	critical = true;
            stream << "fromNode=" << fromNode.location << " is not part of the known road network ";

            //Check if the ConfigManager can find it.
            if (ConfigParams::GetInstance().getNetwork().locateNode(fromNode.location, true, 10)) {
            	critical = false;
            	stream <<"  (...but it is listed in the RoadNetwork)";
            }
        }
        if (toVertex > graphSize)
        {
        	critical = true;
            stream << "toNode=" << toNode.location << " is not part of the known road network";

            //Check if the ConfigManager can find it.
            if (ConfigParams::GetInstance().getNetwork().locateNode(toNode.location, true, 10)) {
            	critical = false;
            	stream <<"  (...but it is listed in the RoadNetwork)";
            }
        }

        //Should we actually throw this
        if (critical) {
        	throw std::runtime_error(stream.str().c_str());
        }

        //Either way it's a problem.
        return false;
    }

    //Nothing wrong here.
    return true;
}

// Computes the shortest path from <fromVertex> to <toVertex> in the graph.  If <toVertex> is not
// reachable from <fromVertex>, then return an empty array.
std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::shortestPath(Vertex const fromVertex, Vertex const toVertex,
                                                Graph const & graph)
const
{
    // The code here is based on the example in the book "The Boost Graph Library" by
    // Jeremy Siek, et al.
    std::vector<Vertex> parent(boost::num_vertices(graph));
    for (Graph::vertices_size_type i = 0; i < boost::num_vertices(graph); ++i) {
        parent[i] = i;
    }

    // If I have counted them correctly, dijkstra_shortest_paths() function has 12 parameters,
    // 10 of which have default values.  The Boost Graph Library has a facility (called named
    // parameter) that resembles python keyword argument.  You can pass an argument to one of
    // those parameters that have default values without worrying about the order of the parameters.
    // In the following, only the predecessor_map parameter is named and the parent array is
    // passed in as this parameter; the other parameters take their default values.
    boost::dijkstra_shortest_paths(graph, fromVertex, boost::predecessor_map(&parent[0]));

    return extractShortestPath(fromVertex, toVertex, parent, graph);
}

std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::extractShortestPath(Vertex const fromVertex,
                                                       Vertex const toVertex,
                                                       std::vector<Vertex> const & parent,
                                                       Graph const & graph)
const
{
    // The code here is based on the example in the book "The Boost Graph Library" by
    // Jeremy Siek, et al.  The dijkstra_shortest_path() function was called with the p = parent
    // array passed in as the predecessor_map paramter.  The shortest path from s = fromVertex
    // to v = toVertex consists of the vertices v, p[v], p[p[v]], ..., and so on until s is
    // reached, in reverse order (text from the Boost Graph Library documentation).  If p[u] = u
    // then u is either the source vertex (s) or a vertex that is not reachable from s.  Therefore
    // the while loop stops when p[u] = u.  If the while loop does not terminates with u equal to
    // fromVertex, then toVertex is not reachable from fromVertex and we return an empty path.

    Vertex v = toVertex;
    Vertex u = parent[v];
    std::vector<WayPoint> result;
    while (u != v)
    {
        Edge e;
        bool exists;
        boost::tie(e, exists) = boost::edge(u, v, graph);

        // Stop the loop if there is no path from u to v.
        if (!exists) {
            break;
        }
        WayPoint wp = boost::get(boost::edge_name, graph, e);
        result.push_back(wp);
        v = u;
        u = parent[v];
    }
    if (u != fromVertex) {
        return std::vector<WayPoint>();
    }

    // result contains the waypoints in the reverse order.  We return them in the correct order.
    return std::vector<WayPoint>(result.rbegin(), result.rend());
}


std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::extractShortestPath(Vertex const fromVertex,
                                                       Vertex const toVertex,
                                                       std::vector<Vertex> const & parent,
                                                       Graph const & graph,std::vector<Edge> & edges)
const
{
    // The code here is based on the example in the book "The Boost Graph Library" by
    // Jeremy Siek, et al.  The dijkstra_shortest_path() function was called with the p = parent
    // array passed in as the predecessor_map paramter.  The shortest path from s = fromVertex
    // to v = toVertex consists of the vertices v, p[v], p[p[v]], ..., and so on until s is
    // reached, in reverse order (text from the Boost Graph Library documentation).  If p[u] = u
    // then u is either the source vertex (s) or a vertex that is not reachable from s.  Therefore
    // the while loop stops when p[u] = u.  If the while loop does not terminates with u equal to
    // fromVertex, then toVertex is not reachable from fromVertex and we return an empty path.

    edges.clear();
    Vertex v = toVertex;
    Vertex u = parent[v];
    std::vector<WayPoint> result;
    while (u != v)
    {
        Edge e;
        bool exists;
        boost::tie(e, exists) = boost::edge(u, v, graph);
        edges.push_back(e);
        // Stop the loop if there is no path from u to v.
        if (!exists)
            break;
        WayPoint wp = boost::get(boost::edge_name, graph, e);
        result.push_back(wp);
        v = u;
        u = parent[v];
    }
    if (u != fromVertex)
        return std::vector<WayPoint>();

    // result contains the waypoints in the reverse order.  We return them in the correct order.
    return std::vector<WayPoint>(result.rbegin(), result.rend());
}


// Get the vertices in the walkingMap_ graph that are closest to <fromPoint> and <toPoint>
// and return them in <fromVertex> and <toVertex> respectively.
void StreetDirectory::ShortestPathImpl::getVertices(Vertex & fromVertex, Vertex & toVertex,
                                               Point2D const & fromPoint, Point2D const & toPoint) const
{
    double d1 = std::numeric_limits<double>::max();
    double d2 = std::numeric_limits<double>::max();
    double h;
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(walkingMap_); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, walkingMap_, v);
        h = hypot(fromPoint, node->location);
        if (d1 > h)
        {
            d1 = h;
            fromVertex = v;
        }
        h = hypot(toPoint, node->location);
        if (d2 > h)
        {
            d2 = h;
            toVertex = v;
        }
    }
}


void StreetDirectory::ShortestPathImpl::clearChoiceSet()
{
	choiceSet.clear();

}


void StreetDirectory::ShortestPathImpl::GeneratePathChoiceSet()
{
	clearChoiceSet();
	Graph::vertex_iterator iter, end;
	for (boost::tie(iter, end) = boost::vertices(drivingMap_); iter != end; ++iter)
	{
		Vertex v = *iter;
		Node const * node = boost::get(boost::vertex_name, drivingMap_, v);
		std::vector<std::vector<std::vector<WayPoint> > > paths_from_v;

		std::vector<Vertex> parent(boost::num_vertices(drivingMap_));
		for (Graph::vertices_size_type i = 0; i < boost::num_vertices(drivingMap_); ++i)
			parent[i] = i;

		//std::cout<<"vertex "<<v<<std::endl;
		boost::dijkstra_shortest_paths(drivingMap_, v, boost::predecessor_map(&parent[0]));
		for (Graph::vertices_size_type i = 0; i < boost::num_vertices(drivingMap_); ++i)
		{
			std::vector<std::vector<WayPoint> > temp_paths;
			std::vector<Edge> edges;
			temp_paths.push_back(extractShortestPath(v, i, parent, drivingMap_,edges));
			for(size_t j=0; j<edges.size(); j++)
			{
				centimeter_t length = boost::get(boost::edge_weight, drivingMap_, edges.at(j));
				boost::put(boost::edge_weight, drivingMap_, edges.at(j), 10*length);
				std::vector<Vertex> parent(boost::num_vertices(drivingMap_));
				for (Graph::vertices_size_type k = 0; k < boost::num_vertices(drivingMap_); ++k)
					parent[k] = k;
				boost::dijkstra_shortest_paths(drivingMap_, v, boost::predecessor_map(&parent[0]));
				std::vector<WayPoint> path = extractShortestPath(v, i, parent, drivingMap_);
				if(!checkIfExist(temp_paths, path))
					temp_paths.push_back(path);
				boost::put(boost::edge_weight, drivingMap_, edges.at(j), length);
			}
			//std::cout<<"size of path "<<temp_paths.size()<<std::endl;
			paths_from_v.push_back(temp_paths);
		}
		choiceSet.push_back(paths_from_v);
	}
}


#endif

#undef HIDE_OLD_CODE
