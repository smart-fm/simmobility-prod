//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PrintNetwork.hpp"

#include "conf/ConfigParams.hpp"
#include "entities/signal/Signal.hpp"
#include "logging/Log.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/BusStop.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "network/CommunicationDataManager.hpp"
#include "util/DynamicVector.hpp"
#include "metrics/Length.hpp"
#include "geospatial/network/RoadNetwork.hpp"




using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;

sim_mob::PrintNetwork::PrintNetwork(ConfigParams& cfg, const std::string& outFileName) : cfg(cfg), out(outFileName.c_str())
{
	//Print the network legacy format to our log file.
	LogNetworkLegacyFormat();
}

void sim_mob::PrintNetwork::LogIncidents() const
{
	std::vector<IncidentParams>& incidents = cfg.getIncidents();
	const unsigned int baseGranMS = cfg.system.simulation.simStartTime.getValue();
	double baseFrameTick = cfg.system.simulation.baseGranMS;

	if (incidents.size() == 0)
	{
		return;
	}

	out << "Printing incident" << std::endl;
	out << "{\"Incident\" : ";
	for (std::vector<IncidentParams>::iterator incIt = incidents.begin(); incIt != incidents.end(); incIt++)
	{
		out << "{";
		out << "\"id\":\"" << incIt->incidentId << "\",";
		out << "\"visibility\":\"" << incIt->visibilityDistance << "\",";
		out << "\"segment_aimsun_id\":\"" << incIt->segmentId << "\",";
		out << "\"position\":\"" << incIt->position << "\",";
		out << "\"severity\":\"" << incIt->severity << "\",";
		out << "\"cap_factor\":\"" << incIt->capFactor << "\",";
		out << "\"start_time\":\"" << (incIt->startTime - baseGranMS) / baseFrameTick << "\",";
		out << "\"duration\":\"" << incIt->duration / baseFrameTick << "\",";
		out << "\"length\":\"" << incIt->length << "\",";
		out << "\"compliance\":\"" << incIt->compliance << "\",";

		for (std::vector<IncidentParams::LaneParams>::iterator laneIt = incIt->laneParams.begin(); laneIt != incIt->laneParams.end(); laneIt++)
		{
			if (laneIt->speedLimit == 0)
			{
				out << "\"speed_limit\":\"" << (*laneIt).speedLimit << "\",";
				out << "\"xLaneStartPos\":\"" << static_cast<int> (laneIt->xLaneStartPos) << "\",";
				out << "\"yLaneStartPos\":\"" << static_cast<int> (laneIt->yLaneStartPos) << "\",";
				out << "\"xLaneEndPos\":\"" << static_cast<int> (laneIt->xLaneEndPos) << "\",";
				out << "\"yLaneEndPos\":\"" << static_cast<int> (laneIt->yLaneEndPos) << "\"";
				break;
			}
		}

		std::ostringstream oss;
		for (std::vector<IncidentParams::LaneParams>::iterator laneIt = incIt->laneParams.begin(); laneIt != incIt->laneParams.end(); laneIt++)
		{
			if (laneIt->speedLimit == 0)
			{
				if (oss.str().size() > 0)
				{
					oss << " ";
				}
				oss << laneIt->laneId;
			}
		}

		out << "\"lane\":\"" << oss.str() << "\",";
		out << "\"accessibility\":\"" << incIt->accessibility << "\"}";
	}
	out << "}" << std::endl;
}

void sim_mob::PrintNetwork::LogNetworkLegacyFormat() const
{
	//Avoid any output calculations if output is disabled.
	if (cfg.OutputDisabled())
	{
		return;
	}

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
	LogLegacyNodeProps(cachedSegments);
	//LogLegacyMultiNodeProps(cachedSegments, cachedConnectors);

	//Print Links.
	LogLegacyLinks();

	//Print segments; build up a list of crossings/bus stops as you go.
	set<const BusStop*> cachedBusStops;
	for (set<const RoadSegment*>::const_iterator it = cachedSegments.begin(); it != cachedSegments.end(); it++)
	{
		LogLegacySegment(*it, cachedBusStops);
		LogLegacySegPolyline(*it);
		LogLegacySegLanes(*it);
	}

	/*for (set<const BusStop*>::iterator it = cachedBusStops.begin(); it != cachedBusStops.end(); it++)
	{
		LogLegacyBusStop(*it);
	}*/
	
	for (set<LaneConnector*>::const_iterator it = cachedConnectors.begin(); it != cachedConnectors.end(); it++)
	{
		LogLegacyLaneConnectors(*it);
	}
	
	const std::map<unsigned int, TurningPath *> turningSectionMap = RoadNetwork::getInstance()->getMapOfIdvsTurningPaths();
	LogTurnings(turningSectionMap);

	const std::map<unsigned int, TurningConflict* > conflicts = RoadNetwork::getInstance()->getMapOfIdvsTurningConflicts();
	LogConflicts(conflicts);

	//Tell the GUI this is done.
	if (cfg.InteractiveMode())
	{
		string end = "END";
		cfg.getCommDataMgr().sendRoadNetworkData(end);
	}

	//Print the StreetDirectory graphs.
	StreetDirectory::instance().printDrivingGraph(out);
	StreetDirectory::instance().printWalkingGraph(out);

	//Required for the visualizer
	out << "ROADNETWORK_DONE" << std::endl;

	LogIncidents();

}

void sim_mob::PrintNetwork::LogLegacySimulationProps() const
{
	//Initial message
	out << "Printing node network" << std::endl;
	out << "NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." << std::endl;

	//Print some properties of the simulation itself
	out << "(\"simulation\", 0, 0, {";
	out << "\"frame-time-ms\":\"" << cfg.baseGranMS() << "\",";
	out << "})" << std::endl;
}

void sim_mob::PrintNetwork::LogLegacySignalProps() const
{
	//Save signal information.
	for (std::vector<Signal*>::const_iterator it = sim_mob::Signal::all_signals_.begin(); it != sim_mob::Signal::all_signals_.end(); it++)
	{
		//Print the Signal representation.
		out << (*it)->toString() << std::endl;
	}
}

void sim_mob::PrintNetwork::LogLegacyNodeProps(set<const RoadSegment*>& cachedSegments) const
{
	//Print all Uni-Nodes, caching RoadSegments as you go.
	const map<unsigned int, Node *>& nodes = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	for (map<unsigned int, Node *>::const_iterator it = nodes.begin(); it != nodes.end(); it++)
	{
		std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
		out << "(\"node\", 0, " << it->second->getNodeId() << ", {";
		out << "\"xPos\":\"" << it->second->getLocation()->getX() << "\",";
		out << "\"yPos\":\"" << it->second->getLocation()->getY() << "\",";
		out << "\"node-type\":\"" << it->second->getNodeType() << "\",";
		out << it->second->getNodeId();
		out << "})";
		PrintToFileAndGui(out);

		//Cache all segments at this Node.
		/*vector<const RoadSegment*> segs = (*it)->getRoadSegments();
		for (vector<const RoadSegment*>::const_iterator i2=segs.begin(); i2!=segs.end(); ++i2) {
			cachedSegments.insert(*i2);
		}*/
	}
}

/*
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
		out <<"\"node-type\":\"" <<node->type <<"\",";
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
 */

void sim_mob::PrintNetwork::LogLegacyLinks() const
{
	const map<unsigned int, Link *> links = RoadNetwork::getInstance()->getMapOfIdVsLinks();
	for (map<unsigned int, Link *>::const_iterator it = links.begin(); it != links.end(); it++)
	{
		std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
		out << "(\"link\", 0, " << it->second->getLinkId() << ", {";
		out << "\"road-name\":\"" << it->second->getRoadName() << "\",";
		out << "\"start-node\":\"" << it->second->getFromNodeId() << "\",";
		out << "\"end-node\":\"" << it->second->getToNodeId() << "\",";
		out << "\"fwd-path\":\"[";
		vector<RoadSegment *> path = it->second->getRoadSegments();
		for (vector<RoadSegment*>::const_iterator segIt = path.begin(); segIt != path.end(); segIt++)
		{
			out << (*segIt)->getRoadSegmentId() << ",";
		}
		out << "]\",";
		out << "})";
		PrintToFileAndGui(out);
	}
}

void sim_mob::PrintNetwork::LogLegacySegment(const RoadSegment * const rs, set<const BusStop*>& cachedBusStops) const
{
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"road-segment\", 0, " << rs << ", {";
	out << "\"parent-link\":\"" << rs->getLinkId() << "\",";
	out << "\"max-speed\":\"" << rs->getMaxSpeed() << "\",";
	out << "\"lanes\":\"" << rs->getLanes().size() << "\",";
	out << "\"segment-type\":\"" << rs->getParentLink()->getLinkType() << "\",";
	out << rs->getRoadSegmentId();
	out << "})";
	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::LogLegacySegPolyline(const RoadSegment * const rs) const
{
	//No polyline?
	if (rs->getPolyLine()->getPoints().empty())
	{
		return;
	}

	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"polyline\", 0, " << rs->getPolyLine()->getPolyLineId() << ", {";
	out << "\"parent-segment\":\"" << rs << "\",";
	out << "\"points\":\"[";
	for (vector<PolyPoint>::const_iterator ptIt = rs->getPolyLine()->getPoints().begin(); ptIt != rs->getPolyLine()->getPoints().end(); ptIt++)
	{
		out << "(" << ptIt->getX() << "," << ptIt->getY() << "),";
	}
	out << "]\",";
	out << "})";

	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::LogLegacySegLanes(const RoadSegment * const rs) const
{
	//Header.
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"lane\", 0, " << &(rs->getLanes()) << ", {";
	out << "\"parent-segment\":\"" << rs->getRoadSegmentId() << "\",";

	//Print each lane line.
	for (size_t laneID = 0; laneID <= rs->getLanes().size(); laneID++)
	{
		const vector<PolyPoint> points = rs->getLane(laneID)->getPolyLine()->getPoints();
		out << "\"lane-" << laneID /*(*it)->getLanes()[laneID]*/ << "\":\"[";
		for (vector<PolyPoint>::const_iterator ptIt = points.begin(); ptIt != points.end(); ptIt++)
		{
			out << "(" << ptIt->getX() << "," << ptIt->getY() << "),";
		}
		out << "]\",";

		if (rs->getLane(laneID)->isPedestrianLane())
		{
			out << "\"line-" << laneID << "is-pedestrian-lane\":\"true\",";
		}
	}

	out << "})";

	PrintToFileAndGui(out);
}

/*void sim_mob::PrintNetwork::LogLegacyCrossing(const Crossing * const cr) const
{
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"crossing\", 0, " << cr << ", {";
	out << "\"near-1\":\"" << cr->nearLine.first.getX() << "," << cr->nearLine.first.getY() << "\",";
	out << "\"near-2\":\"" << cr->nearLine.second.getX() << "," << cr->nearLine.second.getY() << "\",";
	out << "\"far-1\":\"" << cr->farLine.first.getX() << "," << cr->farLine.first.getY() << "\",";
	out << "\"far-2\":\"" << cr->farLine.second.getX() << "," << cr->farLine.second.getY() << "\",";
	out << "})";

	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::LogLegacyBusStop(const BusStop * const bs) const
{
	//Avoid null BusStops (shouldn't happen)
	if (!bs)
	{
		return;
	}

	//Assumptions about the size of a Bus Stop
	const centimeter_t length = 400;
	const centimeter_t width = 250;

	Point dir;
	{
		const Node* start = nullptr;
		if (bs->getParentSegment())
		{
			start = bs->getParentSegment()->getStart();
		}

		const Node* end = nullptr;
		if (bs->getParentSegment())
		{
			end = bs->getParentSegment()->getEnd();
		}

		if (start && end)
		{
			dir = Point(start->location.getX() - end->location.getX(), start->location.getY() - end->location.getY());
		}
		else
		{
			return;
		}
	}

	//Get a vector that is at the "lower-left" point of a bus stop, facing "up" and "right"
	DynamicVector vec(bs->xPos, bs->yPos, bs->xPos + dir.getX(), bs->yPos + dir.getY());
	vec.scaleVectTo(length / 2).translateVect().flipRight();
	vec.scaleVectTo(width / 2).translateVect().flipRight();

	//Now, create a "near" vector and a "far" vector
	DynamicVector near(vec);
	near.scaleVectTo(length);
	DynamicVector far(vec);
	far.flipRight().scaleVectTo(width).translateVect();
	far.flipLeft().scaleVectTo(length);

	//Save it.
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"busstop\", 0, " << bs << ", {";
	out << "\"aimsun-id\":\"" << bs->busstopno_ << "\",";
	out << "\"near-1\":\"" << static_cast<int> (near.getX()) << "," << static_cast<int> (near.getY()) << "\",";
	out << "\"near-2\":\"" << static_cast<int> (near.getEndX()) << "," << static_cast<int> (near.getEndY()) << "\",";
	out << "\"far-1\":\"" << static_cast<int> (far.getX()) << "," << static_cast<int> (far.getY()) << "\",";
	out << "\"far-2\":\"" << static_cast<int> (far.getEndX()) << "," << static_cast<int> (far.getEndY()) << "\",";
	out << "})";
	PrintToFileAndGui(out);
}
*/

void sim_mob::PrintNetwork::LogLegacyLaneConnectors(const LaneConnector * const lc) const
{
	//Output
	std::stringstream out; //Shadow PrintNetwork::out to prevent accidental stream modification.
	out << "(\"lane-connector\", 0, " << lc->getLaneConnectionId() << ", {";
	out << "\"from-segment\":\"" << lc->getFromRoadSegmentId() << "\",";
	out << "\"from-lane\":\"" << lc->getFromLaneId() << "\",";
	out << "\"to-segment\":\"" << lc->getToRoadSegmentId() << "\",";
	out << "\"to-lane\":\"" << lc->getToLaneId() << "\",";
	out << "})";

	PrintToFileAndGui(out);
}

void sim_mob::PrintNetwork::LogTurnings(const std::map<unsigned int, TurningPath*>& turnings) const
{
	std::map<unsigned int, TurningPath*>::const_iterator it;
	for (it = turnings.begin(); it != turnings.end(); ++it)
	{
		const TurningPath* ts = it->second;
		std::stringstream out;
		out << "(\"turning-path\", 0, " << ts << ", {";
		out << "\"id\":\"" << ts->getTurningPathId() << "\",";
		out << std::setprecision(10) << "\"from_xpos\":\"" << ts->getPolyLine()->getFirstPoint().getX() * 100.0 << "\",";
		out << std::setprecision(8) << "\"from_ypos\":\"" << ts->getPolyLine()->getFirstPoint().getY() * 100.0 << "\",";
		out << std::setprecision(8) << "\"to_xpos\":\"" << ts->getPolyLine()->getLastPoint().getX() * 100.0 << "\",";
		out << std::setprecision(8) << "\"to_ypos\":\"" << ts->getPolyLine()->getLastPoint().getY() * 100.0 << "\",";
		out << std::setprecision(8) << "\"from_road_section\":\"" << ts->getFromLane()->getRoadSegmentId() << "\",";
		out << std::setprecision(8) << "\"to_road_section\":\"" << ts->getToLane()->getRoadSegmentId() << "\",";
		out << std::setprecision(8) << "\"from_lane_index\":\"" << ts->getFromLaneId() << "\",";
		out << std::setprecision(8) << "\"to_lane_index\":\"" << ts->getToLaneId() << "\",";
		out << "})";

		PrintToFileAndGui(out);
	}
}

void sim_mob::PrintNetwork::LogConflicts(const std::map<unsigned int, TurningConflict* >& conflicts) const
{
	std::map<unsigned int, TurningConflict* >::const_iterator it;
	for (it = conflicts.begin(); it != conflicts.end(); ++it)
	{
		const sim_mob::TurningConflict* cf = it->second;
		std::stringstream out;
		out << "(\"turning-conflict\", 0, " << cf << ", {";
		out << "\"id\":\"" << cf->getConflictId() << "\",";
		out << "\"first-turning\":\"" << cf->getFirstTurningId() << "\",";
		out << std::setprecision(8) << "\"first-cd\":\"" << cf->getFirstConflictDistance() << "\",";
		out << "\"second-turning\":\"" << cf->getSecondTurningId() << "\",";
		out << std::setprecision(8) << "\"second-cd\":\"" << cf->getSecondConflictDistance() << "\",";
		out << "})";

		PrintToFileAndGui(out);
	}
}

void sim_mob::PrintNetwork::PrintToFileAndGui(const std::stringstream& str) const
{
	//Print to file.
	out << str.str() << std::endl;

	//Print to GUI (if it's active).
	if (cfg.InteractiveMode())
	{
		cfg.getCommDataMgr().sendRoadNetworkData(str.str());
	}
}