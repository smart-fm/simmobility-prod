//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NetworkPrinter.hpp"

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
#include "geospatial/network/NetworkLoader.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;

NetworkPrinter::NetworkPrinter(ConfigParams& cfg, const std::string& outFileName) : cfg(cfg), out(outFileName.c_str())
{
	//Print the header, and simulation-wide properties.
	PrintSimulationProperties();
	
	const RoadNetwork *network = RoadNetwork::getInstance();
	
	PrintNetwork(network);
}

void NetworkPrinter::PrintNetwork(const RoadNetwork *network) const
{
	//Avoid any output calculations if output is disabled.
	if (cfg.OutputDisabled())
	{
		return;
	}

	PrintNodes(network->getMapOfIdvsNodes());

	PrintLinks(network->getMapOfIdVsLinks());

	PrintSegments(network->getMapOfIdVsRoadSegments());
	
	PrintLaneConnectors(network->getMapOfIdVsLanes());
	
	PrintTurningGroups(network->getMapOfIdvsTurningGroups());
	
	PrintTurnings(network->getMapOfIdvsTurningPaths());

	PrintConflicts(network->getMapOfIdvsTurningConflicts());
	
	PrintSignals();
	
	PrintBusStops();

	//Tell the GUI this is done.
	if (cfg.InteractiveMode())
	{
		string end = "END";
		cfg.getCommDataMgr().sendRoadNetworkData(end);
	}

	//Print the StreetDirectory graphs.
	//StreetDirectory::Instance().printDrivingGraph(out);
	//StreetDirectory::Instance().printWalkingGraph(out);

	//Required for the visualizer
	out << "ROADNETWORK_DONE" << std::endl;
}

void NetworkPrinter::PrintSimulationProperties() const
{
	//Initial message
	out << "Printing node network" << std::endl;
	out << "NOTE: All IDs in this section are consistent for THIS simulation run, but will change if you run the simulation again." << std::endl;

	//Print some properties of the simulation itself
	out << "(\"simulation\", 0, 0, {";
	out << "\"frame-time-ms\":\"" << cfg.baseGranMS() << "\",";
	out << "})";
}

void NetworkPrinter::PrintNodes(const map<unsigned int, Node *> &nodes) const
{
	std::stringstream out;
	
	for (map<unsigned int, Node *>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		out << "\n(\"node\", 0, " << it->second->getNodeId() << ", {";
		out << "\"location\":\"[";
		out << "(" << it->second->getLocation()->getX() << "," << it->second->getLocation()->getY() << "),";
		out << "]\",";
		out << "\"type\":\"" << it->second->getNodeType() << "\",";
		out << "})";
	}
	
	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintLinks(const map<unsigned int, Link *> &links) const
{
	std::stringstream out;
	
	for (map<unsigned int, Link *>::const_iterator it = links.begin(); it != links.end(); ++it)
	{
		out << "\n(\"link\", 0, " << it->second->getLinkId() << ", {";
		out << "\"name\":\"" << it->second->getRoadName() << "\",";
		out << "\"from\":\"" << it->second->getFromNodeId() << "\",";
		out << "\"to\":\"" << it->second->getToNodeId() << "\",";
		out << "\"type\":\"" << it->second->getLinkType() << "\",";
		out << "\"category\":\"" << it->second->getLinkCategory() << "\",";
		out << "\"fwd-path\":\"[";
		
		//The associated segments
		vector<RoadSegment *> path = it->second->getRoadSegments();
		for (vector<RoadSegment*>::const_iterator segIt = path.begin(); segIt != path.end(); ++segIt)
		{
			out << (*segIt)->getRoadSegmentId() << ",";
		}
		out << "]\",";
		out << "})";
	}
	
	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintSegments(const map<unsigned int, RoadSegment *> &segments) const
{
	std::stringstream out;

	for (map<unsigned int, RoadSegment *>::const_iterator it = segments.begin(); it != segments.end(); ++it)
	{
		out << "\n(\"segment\", 0, " << it->second->getRoadSegmentId() << ", {";
		out << "\"link\":\"" << it->second->getLinkId() << "\",";
		out << "\"seq-number\":\"" << it->second->getSequenceNumber() << "\",";
		out << "\"max-speed\":\"" << it->second->getMaxSpeed() << "\",";
		out << "\"num-lanes\":\"" << it->second->getLanes().size() << "\",";
		out << "\"points\":\"[";
		const PolyLine *polyLine = it->second->getPolyLine();
		for (vector<PolyPoint>::const_iterator itPts = polyLine->getPoints().begin(); itPts != polyLine->getPoints().end(); ++itPts)
		{
			out << "(" << itPts->getX() << "," << itPts->getY() << "),";
		}
		out << "]\",";
		out << "})";

		//The associated lanes
		const vector<Lane *> &lanes = it->second->getLanes();		
		for (size_t index = 0; index < lanes.size(); ++index)
		{
			out << "\n(\"lane\", 0, " << lanes[index]->getLaneId() << ", {";
			out << "\"index\":\"" << lanes[index]->getLaneIndex();
			out << "\"points\":\"[";
			PolyLine *polyLine = lanes[index]->getPolyLine();
			for (vector<PolyPoint>::const_iterator itPts = polyLine->getPoints().begin(); itPts != polyLine->getPoints().end(); ++itPts)
			{
				out << "(" << itPts->getX() << "," << itPts->getY() << "),";
			}
			out << "]\",";

			if (lanes[index]->isPedestrianLane())
			{
				out << "\"lane-" << lanes[index]->getLaneIndex() << "is-pedestrian-lane\":\"true\",";
			}
		}
		out << "})";
	}

	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintLaneConnectors(const map<unsigned int, Lane *> &lanes) const
{
	std::stringstream out;
	
	for(map<unsigned int, Lane *>::const_iterator it = lanes.begin(); it != lanes.end(); ++it)
	{
		const vector<LaneConnector *> &connectors = it->second->getLaneConnectors();
		for (vector<LaneConnector *>::const_iterator itConnectors = connectors.begin(); itConnectors != connectors.end(); ++itConnectors)
		{
			out << "\n(\"lane-connector\", 0, " << (*itConnectors)->getLaneConnectionId() << ", {";
			out << "\"from-segment\":\"" << (*itConnectors)->getFromRoadSegmentId() << "\",";
			out << "\"from-lane\":\"" << (*itConnectors)->getFromLaneId() << "\",";
			out << "\"to-segment\":\"" << (*itConnectors)->getToRoadSegmentId() << "\",";
			out << "\"to-lane\":\"" << (*itConnectors)->getToLaneId() << "\",";
			out << "})";
		}
	}

	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintTurningGroups(const std::map<unsigned int, TurningGroup *>& turningGroups) const
{
	std::stringstream out;

	for (map<unsigned int, TurningGroup *>::const_iterator it = turningGroups.begin(); it != turningGroups.end(); ++it)
	{
		const TurningGroup *group = it->second;
		out << "\n(\"turning-group\", 0, " << group->getTurningGroupId() << ", {";
		out << "\"node\":\"" << group->getNodeId() << "\",";
		out << "\"from\":\"" << group->getFromLinkId() << "\",";
		out << "\"to\":\"" << group->getToLinkId() << "\",";
		out << "})";
	}

	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintTurnings(const map<unsigned int, TurningPath*>& turnings) const
{
	std::stringstream out;

	for (map<unsigned int, TurningPath *>::const_iterator it = turnings.begin(); it != turnings.end(); ++it)
	{
		const TurningPath *turningPath = it->second;
		out << "\n(\"turning-path\", 0, " << turningPath << ", {";
		out << "\"group\":\"" << turningPath->getTurningGroupId() << "\",";
		out << "\"from-segment\":\"" << turningPath->getFromLane()->getRoadSegmentId() << "\",";
		out << "\"to-segment\":\"" << turningPath->getToLane()->getRoadSegmentId() << "\",";
		out << "\"from-lane\":\"" << turningPath->getFromLaneId() << "\",";
		out << "\"to-lane\":\"" << turningPath->getToLaneId() << "\",";
		out << "\"points\":\"[";
		const PolyLine *polyLine = turningPath->getPolyLine();
		for (vector<PolyPoint>::const_iterator itPts = polyLine->getPoints().begin(); itPts != polyLine->getPoints().end(); ++itPts)
		{
			out << "(" << itPts->getX() << "," << itPts->getY() << "),";
		}
		out << "]\",";
		out << "})";
	}

	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintConflicts(const std::map<unsigned int, TurningConflict* >& conflicts) const
{
	std::stringstream out;

	for (std::map<unsigned int, TurningConflict* >::const_iterator it = conflicts.begin(); it != conflicts.end(); ++it)
	{
		const TurningConflict *conflict = it->second;
		out << "\n(\"conflict\", 0, " << conflict->getConflictId() << ", {";
		out << "\"turning-1\":\"" << conflict->getFirstTurningId() << "\",";
		out << std::setprecision(8) << "\"conflict-dist-1\":\"" << conflict->getFirstConflictDistance() << "\",";
		out << "\"turning-2\":\"" << conflict->getSecondTurningId() << "\",";
		out << std::setprecision(8) << "\"conflict-dist-2\":\"" << conflict->getSecondConflictDistance() << "\",";
		out << "})";
	}

	PrintToFileAndGui(out);
}

void NetworkPrinter::PrintSignals() const
{
}

void NetworkPrinter::PrintBusStops() const
{
}

void NetworkPrinter::PrintToFileAndGui(const std::stringstream& str) const
{
	//Print to file.
	out << str.str() << std::endl;

	//Print to GUI (if it's active).
	if (cfg.InteractiveMode())
	{
		cfg.getCommDataMgr().sendRoadNetworkData(str.str());
	}
}