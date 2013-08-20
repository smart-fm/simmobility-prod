/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "PrintNetwork.hpp"

#include "conf/simpleconf.hpp"
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
#include "network/CommunicationDataManager.hpp"
#include "util/DynamicVector.hpp"
#include "metrics/Length.hpp"



using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


sim_mob::PrintNetwork::PrintNetwork(const ConfigParams& cfg, const std::string& outFileName) : cfg(cfg), out(outFileName.c_str())
{
	//Print the network legacy format to our log file.
	LogNetworkLegacyFormat();
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

	//Tell the GUI this is done.
	if (cfg.InteractiveMode()) {
		string end = "END";
		ConfigParams::GetInstance().getCommDataMgr().sendRoadNetworkData(end);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph(out);
	StreetDirectory::instance().printWalkingGraph(out);

	//Required for the visualizer
	out <<"ROADNETWORK_DONE" <<std::endl;
}



void sim_mob::PrintNetwork::LogLegacySimulationProps() const
{
	//Initial message
	out <<"Printing node network" <<std::endl;
	out <<"NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." <<std::endl;

	//Print some properties of the simulation itself
	out <<"(\"simulation\", 0, 0, {";
	out <<"\"frame-time-ms\":\"" <<ConfigParams::GetInstance().baseGranMS <<"\",";
	out <<"})" <<std::endl;
}


void sim_mob::PrintNetwork::LogLegacySignalProps() const
{
	//Save signal information.
	for (std::vector<Signal*>::const_iterator it = sim_mob::Signal::all_signals_.begin(); it!= sim_mob::Signal::all_signals_.end(); it++) {
		//Print the Signal representation.
		out <<(*it)->toString() <<std::endl;
	}
}


void sim_mob::PrintNetwork::LogLegacyUniNodeProps(set<const RoadSegment*>& cachedSegments) const
{
	//Print all Uni-Nodes, caching RoadSegments as you go.
	const set<UniNode*>& nodes = cfg.getNetwork().getUniNodes();
	for (set<UniNode*>::const_iterator it=nodes.begin(); it!=nodes.end(); it++) {
		std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
		out <<"(\"uni-node\", 0, " <<*it <<", {";
		out <<"\"xPos\":\"" <<(*it)->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<(*it)->location.getY() <<"\",";
		if (!(*it)->originalDB_ID.getLogItem().empty()) {
			out <<(*it)->originalDB_ID.getLogItem();
		}
		out <<"})";
		PrintToFileAndGui(out);

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
	const vector<MultiNode*>& nodes = cfg.getNetwork().getNodes();
	for (vector<MultiNode*>::const_iterator it=nodes.begin(); it!=nodes.end(); it++) {
		const MultiNode* node = *it;
		std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
		out <<"(\"multi-node\", 0, " <<node <<", {";
		out <<"\"xPos\":\"" <<node->location.getX() <<"\",";
		out <<"\"yPos\":\"" <<node->location.getY() <<"\",";
		if (!node->originalDB_ID.getLogItem().empty()) {
			out <<node->originalDB_ID.getLogItem();
		}
		out <<"})";
		PrintToFileAndGui(out);

		//Cache all segments
		const std::set<sim_mob::RoadSegment*>& segs = node->getRoadSegments();
		for (set<RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}

		//Now cache all lane connectors at this node
		for (set<RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			if (node->hasOutgoingLanes(*i2)) {
				const set<LaneConnector*>& lcs = node->getOutgoingLanes(*i2);
				for (set<LaneConnector*>::iterator i3=lcs.begin(); i3!=lcs.end(); i3++) {
					//Cache the connector
					cachedConnectors.insert(*i3);
				}
			}
		}
	}
}


void sim_mob::PrintNetwork::LogLegacyLinks() const
{
	const vector<Link*>& links = cfg.getNetwork().getLinks();
	for (vector<Link*>::const_iterator it=links.begin(); it!=links.end(); it++) {
		std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
		out <<"(\"link\", 0, " <<*it <<", {";
		out <<"\"road-name\":\"" <<(*it)->roadName <<"\",";
		out <<"\"start-node\":\"" <<(*it)->getStart() <<"\",";
		out <<"\"end-node\":\"" <<(*it)->getEnd() <<"\",";
		out <<"\"fwd-path\":\"[";
		vector<RoadSegment*>& path = (*it)->getPath();
		for (vector<RoadSegment*>::const_iterator segIt=path.begin(); segIt!=path.end(); segIt++) {
			out <<*segIt <<",";
		}
		out <<"]\",";
		out <<"})";
		PrintToFileAndGui(out);
	}
}




void sim_mob::PrintNetwork::LogLegacySegment(const RoadSegment* const rs, set<const Crossing*>& cachedCrossings, set<const BusStop*>& cachedBusStops) const
{
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
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
	out <<"})";
	PrintToFileAndGui(out);

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

	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out <<"(\"polyline\", 0, " <<&(rs->polyline) <<", {";
	out <<"\"parent-segment\":\"" <<rs <<"\",";
	out <<"\"points\":\"[";
	for (vector<Point2D>::const_iterator ptIt=rs->polyline.begin(); ptIt!=rs->polyline.end(); ptIt++) {
		out <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
	}
	out <<"]\",";
	out <<"})";

	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::LogLegacySegLanes(const RoadSegment* const rs) const
{
	//Header.
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out <<"(\"lane\", 0, " <<&(rs->getLanes()) <<", {";
	out <<"\"parent-segment\":\"" <<rs <<"\",";

	//Print each lane line.
	for (size_t laneID=0; laneID <= rs->getLanes().size(); laneID++) {
		const vector<Point2D>& points =rs->laneEdgePolylines_cached.at(laneID);
		out <<"\"lane-" <<laneID /*(*it)->getLanes()[laneID]*/<<"\":\"[";
		for (vector<Point2D>::const_iterator ptIt=points.begin(); ptIt!=points.end(); ptIt++) {
			out <<"(" <<ptIt->getX() <<"," <<ptIt->getY() <<"),";
		}
		out <<"]\",";

		if (laneID<rs->getLanes().size() && rs->getLanes().at(laneID)->is_pedestrian_lane()) {
			out <<"\"line-" <<laneID <<"is-sidewalk\":\"true\",";
		}
	}

	out <<"})";

	PrintToFileAndGui(out);
}


void sim_mob::PrintNetwork::LogLegacyCrossing(const Crossing* const cr) const
{
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out <<"(\"crossing\", 0, " <<cr <<", {";
	out <<"\"near-1\":\"" <<cr->nearLine.first.getX() <<"," <<cr->nearLine.first.getY() <<"\",";
	out <<"\"near-2\":\"" <<cr->nearLine.second.getX() <<"," <<cr->nearLine.second.getY() <<"\",";
	out <<"\"far-1\":\"" <<cr->farLine.first.getX() <<"," <<cr->farLine.first.getY() <<"\",";
	out <<"\"far-2\":\"" <<cr->farLine.second.getX() <<"," <<cr->farLine.second.getY() <<"\",";
	out <<"})";

	PrintToFileAndGui(out);
}


void sim_mob::PrintNetwork::LogLegacyBusStop(const BusStop* const bs) const
{
	//Avoid null BusStops (shouldn't happen)
	if (!bs) {
		return;
	}

	//Assumptions about the size of a Bus Stop
	const centimeter_t length = 400;
	const centimeter_t width  = 250;

	Point2D dir;
	{
		const Node* start = nullptr;
		if(bs->getParentSegment()) {
			start = bs->getParentSegment()->getStart();
		}

		const Node* end = nullptr;
		if(bs->getParentSegment()) {
			end = bs->getParentSegment()->getEnd();
		}

		if(start && end) {
			dir = Point2D(start->location.getX()-end->location.getX(),start->location.getY()-end->location.getY());
		} else {
			return;
		}
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
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out <<"(\"busstop\", 0, " <<bs <<", {";
	out <<"\"near-1\":\"" <<near.getX()    <<"," <<near.getY()    <<"\",";
	out <<"\"near-2\":\"" <<near.getEndX() <<"," <<near.getEndY() <<"\",";
	out <<"\"far-1\":\""  <<far.getX()     <<"," <<far.getY()     <<"\",";
	out <<"\"far-2\":\""  <<far.getEndX()  <<"," <<far.getEndY()  <<"\",";
	out <<"})";
	PrintToFileAndGui(out);

	//Old code; this just fakes the bus stop at a 40 degree angle.
	//TODO: Remove this code once we verify that the above code is better.
	/*out <<"(\"busstop\", 0, " <<*it <<", {";
	double x = (*it)->xPos;
	double y = (*it)->yPos;
	int angle = 40;
	double length = 400;
	double width = 250;
	double theta = atan(width / length);
	double phi = M_PI * angle / 180;
	double diagonal_half = (sqrt(length * length + width * width)) / 2;

	//TODO: It looks like this can be easily replaced with DynamicVectors. Should be both faster and simpler.
	double x1d = x + diagonal_half * cos(phi + theta);
	double y1d = y + diagonal_half * sin(phi + theta);
	double x2d = x + diagonal_half * cos(M_PI + phi - theta);
	double y2d = y + diagonal_half * sin(M_PI + phi - theta);
	double x3d = x + diagonal_half * cos(M_PI + phi + theta);
	double y3d = y + diagonal_half * sin(M_PI + phi + theta);
	double x4d = x + diagonal_half * cos(phi - theta);
	double y4d = y + diagonal_half * sin(phi - theta);

	out <<"\"near-1\":\""<<std::setprecision(8)<<x<<","<<y<<"\",";
	out <<"\"near-2\":\""<<x2d<<","<<y2d<<"\",";
	out <<"\"far-1\":\""<<x3d<<","<<y3d<<"\",";
	out <<"\"far-2\":\""<<x4d<<","<<y4d<<"\",";
	out <<"})" <<endl;*/
}

void sim_mob::PrintNetwork::LogLegacyLaneConnectors(const LaneConnector* const lc) const
{
	//Retrieve relevant information
	const RoadSegment* fromSeg = lc->getLaneFrom()->getRoadSegment();
	unsigned int fromLane = std::distance(fromSeg->getLanes().begin(), std::find(fromSeg->getLanes().begin(), fromSeg->getLanes().end(),lc->getLaneFrom()));
	const RoadSegment* toSeg = lc->getLaneTo()->getRoadSegment();
	unsigned int toLane = std::distance(toSeg->getLanes().begin(), std::find(toSeg->getLanes().begin(), toSeg->getLanes().end(),lc->getLaneTo()));

	//Output
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out <<"(\"lane-connector\", 0, " <<lc <<", {";
	out <<"\"from-segment\":\"" <<fromSeg <<"\",";
	out <<"\"from-lane\":\"" <<fromLane <<"\",";
	out <<"\"to-segment\":\"" <<toSeg <<"\",";
	out <<"\"to-lane\":\"" <<toLane <<"\",";
	out <<"})";

	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::PrintToFileAndGui(const std::stringstream& str) const
{
	//Print to file.
	out <<str.str() <<std::endl;

	//Print to GUI (if it's active).
	if (cfg.InteractiveMode()) {
		cfg.getCommDataMgr().sendRoadNetworkData(str.str());
	}
}







