/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "PrintNetwork.hpp"

#include "conf/Config.hpp"
#include "entities/signal/Signal.hpp"
#include "logging/Log.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "util/DynamicVector.hpp"
#include "metrics/Length.hpp"



using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


sim_mob::PrintNetwork::PrintNetwork(const Config& cfg) : cfg(cfg)
{
	//Print the network legacy format to logout.
	out.open("out.network.txt");
	LogNetworkLegacyFormat();
	out.close();
}


void sim_mob::PrintNetwork::LogNetworkLegacyFormat() const
{
	//Avoid any output calculations if output is disabled.
	if (cfg.OutputDisabled()) { return; }

	//Print the header, and simulation-wide properties.
	LogLegacySimulationProps();

	//Print signals.
	//TODO: It is not clear if this is the appropriate place to print signal information, as
	//      Nodes have not yet been printed (and items should generally be printed in order).
	//TODO: Our PrintNetwork() constructor should take a Signals array to print, rather than
	//      relying on the static Signal::all_signals_
	LogLegacySignalProps();

	//Print Uni/Multi nodes, and build an index of Segments/Connectors as you go.
	set<const RoadSegment*> cachedSegments;
	set<LaneConnector*> cachedConnectors;
	LogLegacyUniNodeProps(cachedSegments);
	LogLegacyMultiNodeProps(cachedSegments, cachedConnectors);

	//Print Links.
	LogLegacyLinks();

	//Print segments; build up a list of crossings/bus stops as you go.
	set<const Crossing*> cachedCrossings;
	set<const BusStop*> cachedBusStops;
	for (set<const RoadSegment*>::const_iterator it=cachedSegments.begin(); it!=cachedSegments.end(); it++) {
		LogLegacySegment(*it, cachedCrossings, cachedBusStops);
		LogLegacySegPolyline(*it);
		LogLegacySegLanes(*it);
	}

	//Print crossings, bus stops, lane connectors
	for (set<const Crossing*>::iterator it=cachedCrossings.begin(); it!=cachedCrossings.end(); it++) {
		LogLegacyCrossing(*it);
	}
	for (set<const BusStop*>::iterator it = cachedBusStops.begin(); it!=cachedBusStops.end(); it++) {
		LogLegacyBusStop(*it);
	}
	for (set<LaneConnector*>::const_iterator it=cachedConnectors.begin(); it!=cachedConnectors.end(); it++) {
		LogLegacyLaneConnectors(*it);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph(out);
	StreetDirectory::instance().printWalkingGraph(out);
}



void sim_mob::PrintNetwork::LogLegacySimulationProps() const
{
	//Initial message
	out <<"Printing node network" <<std::endl;
	out <<"NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<std::endl;

	//Print some properties of the simulation itself
	out <<"(\"simulation\", 0, 0, {";
	out <<"\"frame-time-ms\":\"" <<cfg.simulation().baseGranularity.ms() <<"\",";
	out <<"})" <<std::endl;
}


void sim_mob::PrintNetwork::LogLegacySignalProps() const
{
	//Save signal information.
	for (vector<Signal*>::const_iterator it = Signal::all_signals_.begin(); it!=Signal::all_signals_.end(); it++) {
		out <<(*it)->toString() <<std::endl;
	}
}


void sim_mob::PrintNetwork::LogLegacyUniNodeProps(set<const RoadSegment*>& cachedSegments) const
{
	//Print all Uni-Nodes, caching RoadSegments as you go.
	const set<UniNode*>& nodes = cfg.network().getUniNodes();
	for (set<UniNode*>::const_iterator it=nodes.begin(); it!=nodes.end(); it++) {
		out <<"(\"uni-node\", 0, " <<*it <<", {";
		out <<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			out <<(*it)->originalDB_ID.getLogItem();
		}
		out <<"})" <<std::endl;

		//Cache all segments at this Node.
		vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}
	}
}


void sim_mob::PrintNetwork::LogLegacyMultiNodeProps(set<const RoadSegment*>& cachedSegments, set<LaneConnector*>& cachedConnectors) const
{
	//Print all MultiNodes, caching segments as you go.
	const vector<MultiNode*>& nodes = cfg.network().getNodes();
	for (vector<MultiNode*>::const_iterator it=nodes.begin(); it!=nodes.end(); it++) {
		const MultiNode* node = *it;
		out <<"(\"multi-node\", 0, " <<node <<", {";
		out <<"\"xPos\":\"" <<node->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<node->location.getY() <<"\",";
		if (!node->originalDB_ID.getLogItem().empty()) {
			out <<node->originalDB_ID.getLogItem();
		}
		out <<"})" <<std::endl;


		//Cache all segments
		for (set<RoadSegment*>::const_iterator i2=node->getRoadSegments().begin(); i2!=node->getRoadSegments().end(); ++i2) {
			cachedSegments.insert(*i2);
		}

		//Now cache all lane connectors at this node
		for (set<RoadSegment*>::const_iterator rsIt=node->getRoadSegments().begin(); rsIt!=node->getRoadSegments().end(); rsIt++) {
			if (node->hasOutgoingLanes(*rsIt)) {
				for (set<LaneConnector*>::iterator i2=node->getOutgoingLanes(*rsIt).begin(); i2!=node->getOutgoingLanes(*rsIt).end(); i2++) {
					cachedConnectors.insert(*i2);
				}
			}
		}
	}
}


void sim_mob::PrintNetwork::LogLegacyLinks() const
{
	const vector<Link*>& links = cfg.network().getLinks();
	for (vector<Link*>::const_iterator it=links.begin(); it!=links.end(); it++) {
		out <<"(\"link\", 0, " <<*it <<", {";
		out <<"\"road-name\":\"" <<(*it)->roadName <<"\",";
		out <<"\"start-node\":\"" <<(*it)->getStart() <<"\",";
		out <<"\"end-node\":\"" <<(*it)->getEnd() <<"\",";
		out <<"\"fwd-path\":\"[";
		for (vector<RoadSegment*>::const_iterator segIt=(*it)->getPath().begin(); segIt!=(*it)->getPath().end(); segIt++) {
			out <<*segIt <<",";
		}
		out <<"]\",";
		out <<"})" <<std::endl;
	}
}




void sim_mob::PrintNetwork::LogLegacySegment(const RoadSegment* const rs, set<const Crossing*>& cachedCrossings, set<const BusStop*>& cachedBusStops) const
{
	out <<"(\"road-segment\", 0, " <<rs <<", {";
	out <<"\"parent-link\":\"" <<rs->getLink() <<"\",";
	out <<"\"max-speed\":\"" <<rs->maxSpeed <<"\",";
	out <<"\"width\":\"" <<rs->width <<"\",";
	out <<"\"lanes\":\"" <<rs->getLanes().size() <<"\",";
	out <<"\"from-node\":\"" <<rs->getStart() <<"\",";
	out <<"\"to-node\":\"" <<rs->getEnd() <<"\",";
	if (!rs->originalDB_ID.getLogItem().empty()) {
		out <<rs->originalDB_ID.getLogItem();
	}
	out <<"})" <<std::endl;

	//Cache obstacles for display later.
	const map<centimeter_t, const RoadItem*>& obstacles = rs->obstacles;
	for(map<centimeter_t, const RoadItem*>::const_iterator obsIt = obstacles.begin(); obsIt!=obstacles.end(); ++obsIt) {
		//Save BusStops for later.
		{
		const BusStop* res = dynamic_cast<const BusStop*>(obsIt->second);
		if (res) {
			cachedBusStops.insert(res);
		}
		}

		//Save crossings for later.
		{
		const Crossing* res = dynamic_cast<const Crossing*>(obsIt->second);
		if (res) {
			cachedCrossings.insert(res);
		}
		}
	}
}

void sim_mob::PrintNetwork::LogLegacySegPolyline(const RoadSegment* const rs) const
{
	//No polyline?
	if (rs->polyline.empty()) { return; }

	out <<"(\"polyline\", 0, " <<&(rs->polyline) <<", {";
	out <<"\"parent-segment\":\"" <<rs <<"\",";
	out <<"\"points\":\"[";
	for (vector<Point2D>::const_iterator ptIt=rs->polyline.begin(); ptIt!=rs->polyline.end(); ptIt++) {
		out <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
	}
	out <<"]\",";
	out <<"})" <<std::endl;
}

void sim_mob::PrintNetwork::LogLegacySegLanes(const RoadSegment* const rs) const
{
	//Lane info is buffered into a complete stream, and then printed all at once.
	std::stringstream buff;
	buff <<"(\"lane\", 0, " <<&(rs->getLanes()) <<", {";
	buff <<"\"parent-segment\":\"" <<rs <<"\",";

	//Print each lane line
	for (size_t laneID=0; laneID<=rs->getLanes().size(); laneID++) {
		//TODO: We need to ensure that the lane_edge_polylines vector is *not* being
		//      modified by getLaneEdgePolyline() after it has finished loading from
		//      the DB/XML. This will require us to "estimate" lanes in the DB/XML code,
		//      rather than performing this check at runtime.
		//const vector<Point2D>& points = rs->getLaneEdgePolyline(laneID);     //<-- this is correct
		const vector<Point2D>& points = rs->laneEdgePolylines_cached[laneID];  //<-- this is a hack (for now)

		//Add the line's geometry
		buff <<"\"line-" <<laneID <<"\":\"[";
		for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
			buff <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
		}
		buff <<"]\",";

		//Tag if this is a sidewalk lane or not
		if (laneID<rs->getLanes().size() && rs->getLanes()[laneID]->is_pedestrian_lane()) {
			buff <<"\"line-" <<laneID <<"is-sidewalk\":\"true\",";
		}

	}
	buff <<"})" <<std::endl;

	//Flush the buffer to output.
	out <<buff.str();
}


void sim_mob::PrintNetwork::LogLegacyCrossing(const Crossing* const cr) const
{
	out <<"(\"crossing\", 0, " <<cr <<", {";
	out <<"\"near-1\":\"" <<cr->nearLine.first.getX() <<"," <<cr->nearLine.first.getY() <<"\",";
	out <<"\"near-2\":\"" <<cr->nearLine.second.getX() <<"," <<cr->nearLine.second.getY() <<"\",";
	out <<"\"far-1\":\"" <<cr->farLine.first.getX() <<"," <<cr->farLine.first.getY() <<"\",";
	out <<"\"far-2\":\"" <<cr->farLine.second.getX() <<"," <<cr->farLine.second.getY() <<"\",";
	out <<"})" <<std::endl;
}


void sim_mob::PrintNetwork::LogLegacyBusStop(const BusStop* const bs) const
{
	//Assumptions about the size of a Bus Stop
	const centimeter_t length = 400;
	const centimeter_t width  = 250;

	//Use the magnitude of the parent segment to set the Bus Stop's direction and extension.
	Point2D dir;
	{
		const Node* start = bs->getParentSegment()->getStart();
		const Node* end = bs->getParentSegment()->getEnd();
		dir = Point2D(start->location.getX()-end->location.getX(),start->location.getY()-end->location.getY());
	}

	//Get a vector that is at the "lower-left" point of a bus stop, facing "up" and "right"
	DynamicVector vec(bs->xPos, bs->yPos, bs->xPos+dir.getX(), bs->yPos+dir.getY());
	vec.scaleVectTo(length/2).translateVect().flipRight();
	vec.scaleVectTo(width/2).translateVect().flipRight();

	//Now, create a "near" vector and a "far" vector
	DynamicVector near(vec);
	near.scaleVectTo(length);
	DynamicVector far(vec);
	far.flipRight().scaleVectTo(width).translateVect();
	far.flipLeft().scaleVectTo(length);

	//Save it.
	out <<"(\"busstop\", 0, " <<bs <<", {";
	out <<"\"near-1\":\"" <<near.getX()    <<"," <<near.getY()    <<"\",";
	out <<"\"near-2\":\"" <<near.getEndX() <<"," <<near.getEndY() <<"\",";
	out <<"\"far-1\":\""  <<far.getX()     <<"," <<far.getY()     <<"\",";
	out <<"\"far-2\":\""  <<far.getEndX()  <<"," <<far.getEndY()  <<"\",";
	out <<"})" <<std::endl;

	//Old code; this just fakes the bus stop at a 40 degree angle.
	//Remove this code once we verify that BusStops are actually located more accurately with the new code.
	/*const double x = bs->xPos;
	const double y = bs->yPos;
	const int angle = 40;
	const double length = 400;
	const double width = 250;
	const double theta = atan(width / length);
	const double phi = M_PI * angle / 180;
	const double diagonal_half = (sqrt(length * length + width * width)) / 2;

	const double x1d = x + diagonal_half * cos(phi + theta);
	const double y1d = y + diagonal_half * sin(phi + theta);
	const double x2d = x + diagonal_half * cos(M_PI + phi - theta);
	const double y2d = y + diagonal_half * sin(M_PI + phi - theta);
	const double x3d = x + diagonal_half * cos(M_PI + phi + theta);
	const double y3d = y + diagonal_half * sin(M_PI + phi + theta);
	const double x4d = x + diagonal_half * cos(phi - theta);
	const double y4d = y + diagonal_half * sin(phi - theta);

	LogOut("(\"busstop\", 0, " <<bs <<", {");
	LogOut("\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",");
	LogOut("\"near-2\":\""<<x2d<<","<<y2d<<"\",");
	LogOut("\"far-1\":\""<<x3d<<","<<y3d<<"\",");
	LogOut("\"far-2\":\""<<x4d<<","<<y4d<<"\",");
	LogOut("})" <<std::endl);*/
}

void sim_mob::PrintNetwork::LogLegacyLaneConnectors(const LaneConnector* const lc) const
{
	//Retrieve relevant information
	const RoadSegment* fromSeg = lc->getLaneFrom()->getRoadSegment();
	unsigned int fromLane = std::distance(fromSeg->getLanes().begin(), std::find(fromSeg->getLanes().begin(), fromSeg->getLanes().end(),lc->getLaneFrom()));
	const RoadSegment* toSeg = lc->getLaneTo()->getRoadSegment();
	unsigned int toLane = std::distance(toSeg->getLanes().begin(), std::find(toSeg->getLanes().begin(), toSeg->getLanes().end(),lc->getLaneTo()));

	//Output
	out <<"(\"lane-connector\", 0, " <<lc <<", {";
	out <<"\"from-segment\":\"" <<fromSeg <<"\",";
	out <<"\"from-lane\":\"" <<fromLane <<"\",";
	out <<"\"to-segment\":\"" <<toSeg <<"\",";
	out <<"\"to-lane\":\"" <<toLane <<"\",";
	out <<"})" <<std::endl;
}










