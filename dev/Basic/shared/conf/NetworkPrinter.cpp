//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NetworkPrinter.hpp"

#include "conf/ConfigParams.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/NetworkLoader.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "metrics/Length.hpp"
#include "network/CommunicationDataManager.hpp"
#include "util/DynamicVector.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;

NetworkPrinter::NetworkPrinter(ConfigParams& cfg, const std::string& outFileName) : cfg(cfg), out(outFileName.c_str())
{
	//Print the header, and simulation-wide properties.
	printSimulationProperties();
}

void NetworkPrinter::printSignals(const std::string& signalsInfo) const
{
	if (!cfg.OutputDisabled())
	{
		out << signalsInfo << std::endl;
	}
}

void NetworkPrinter::printNetwork(const RoadNetwork *network) const
{
	//Avoid any output calculations if output is disabled.
	if (cfg.OutputDisabled())
	{
		return;
	}

	printNodes(network->getMapOfIdvsNodes());

	printLinks(network->getMapOfIdVsLinks());

	printSegments(network->getMapOfIdVsRoadSegments());
	
	printLaneConnectors(network->getMapOfIdVsLanes());
	
	printTurningGroups(network->getMapOfIdvsTurningGroups());
	
	printTurnings(network->getMapOfIdvsTurningPaths());

	printConflicts(network->getMapOfIdvsTurningConflicts());	
	
	printBusStops(network->getMapOfIdvsBusStops());

	printTrafficSensors(network->getMapOfIdVsTrafficSensors());

	//Required for the visualiser
	out << "ROADNETWORK_DONE" << std::endl;
}

void NetworkPrinter::printSimulationProperties() const
{
	//Initial message
	out << "Printing node network" << std::endl;

	//Print some properties of the simulation itself
	out << "(\"simulation\", 0, 0, {";
	out << "\"frame-time-ms\":\"" << cfg.baseGranMS() << "\",";
	out << "})\n";
}

void NetworkPrinter::printNodes(const map<unsigned int, Node *> &nodes) const
{
	std::stringstream out;
	out << std::setprecision(8);
	
	for (map<unsigned int, Node *>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		out << "(\"node\", 0, " << it->second->getNodeId() << ", {";
		out << "\"location\":\"[";
		out << "(" << it->second->getLocation().getX() << "," << it->second->getLocation().getY() << "),";
		out << "]\",";
		out << "\"type\":\"" << it->second->getNodeType() << "\",";
		out << "})\n";
	}
	
	printToFile(out);
}

void NetworkPrinter::printLinks(const map<unsigned int, Link *> &links) const
{
	std::stringstream out;
	out << std::setprecision(8);
	
	for (map<unsigned int, Link *>::const_iterator it = links.begin(); it != links.end(); ++it)
	{
		out << "(\"link\", 0, " << it->second->getLinkId() << ", {";
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
		out << "})\n";
	}
	
	printToFile(out);
}

void NetworkPrinter::printSegments(const map<unsigned int, RoadSegment *> &segments) const
{
	std::stringstream out;
	out << std::setprecision(8);

	for (map<unsigned int, RoadSegment *>::const_iterator it = segments.begin(); it != segments.end(); ++it)
	{
		out << "(\"segment\", 0, " << it->second->getRoadSegmentId() << ", {";
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
		out << "})\n";

		//The associated lanes
		const vector<Lane *> &lanes = it->second->getLanes();		
		for (size_t index = 0; index < lanes.size(); ++index)
		{
			out << "(\"lane\", 0, " << lanes[index]->getLaneId() << ", {";
			out << "\"index\":\"" << lanes[index]->getLaneIndex() << "\",";
			out << "\"width\":\"" << lanes[index]->getWidth() << "\",";
			out << "\"points\":\"[";
			const PolyLine *polyLine = lanes[index]->getPolyLine();
			for (vector<PolyPoint>::const_iterator itPts = polyLine->getPoints().begin(); itPts != polyLine->getPoints().end(); ++itPts)
			{
				out << "(" << itPts->getX() << "," << itPts->getY() << "),";
			}
			out << "]\",";

			if (lanes[index]->isPedestrianLane())
			{
				out << "\"lane-" << lanes[index]->getLaneIndex() << "is-pedestrian-lane\":\"true\",";
			}
			out << "})\n";
		}
	}

	printToFile(out);
}

void NetworkPrinter::printLaneConnectors(const map<unsigned int, Lane *> &lanes) const
{
	std::stringstream out;
	out << std::setprecision(8);
	
	for(map<unsigned int, Lane *>::const_iterator it = lanes.begin(); it != lanes.end(); ++it)
	{
		const std::vector<LaneConnector *> &connectors = it->second->getLaneConnectors();
		for(std::vector<LaneConnector *>::const_iterator itConnectors = connectors.begin(); itConnectors != connectors.end(); ++itConnectors)
		{
			out << "(\"lane-connector\", 0, " << (*itConnectors)->getLaneConnectionId() << ", {";
			out << "\"from-segment\":\"" << (*itConnectors)->getFromRoadSegmentId() << "\",";
			out << "\"from-lane\":\"" << (*itConnectors)->getFromLaneId() << "\",";
			out << "\"to-segment\":\"" << (*itConnectors)->getToRoadSegmentId() << "\",";
			out << "\"to-lane\":\"" << (*itConnectors)->getToLaneId() << "\",";
			out << "})\n";
		}
	}

	printToFile(out);
}

void NetworkPrinter::printTurningGroups(const std::map<unsigned int, TurningGroup *>& turningGroups) const
{
	std::stringstream out;
	out << std::setprecision(8);

	for (map<unsigned int, TurningGroup *>::const_iterator it = turningGroups.begin(); it != turningGroups.end(); ++it)
	{
		const TurningGroup *group = it->second;
		out << "(\"turning-group\", 0, " << group->getTurningGroupId() << ", {";
		out << "\"node\":\"" << group->getNodeId() << "\",";
		out << "\"from\":\"" << group->getFromLinkId() << "\",";
		out << "\"to\":\"" << group->getToLinkId() << "\",";
		out << "})\n";
	}

	printToFile(out);
}

void NetworkPrinter::printTurnings(const map<unsigned int, TurningPath*>& turnings) const
{
	std::stringstream out;
	out << std::setprecision(8);

	for (map<unsigned int, TurningPath *>::const_iterator it = turnings.begin(); it != turnings.end(); ++it)
	{
		const TurningPath *turningPath = it->second;
		out << "(\"turning-path\", 0, " << turningPath->getTurningPathId() << ", {";
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
		out << "})\n";
	}

	printToFile(out);
}

void NetworkPrinter::printConflicts(const std::map<unsigned int, TurningConflict* >& conflicts) const
{
	std::stringstream out;
	out << std::setprecision(8);

	for (std::map<unsigned int, TurningConflict* >::const_iterator it = conflicts.begin(); it != conflicts.end(); ++it)
	{
		const TurningConflict *conflict = it->second;
		out << "(\"conflict\", 0, " << conflict->getConflictId() << ", {";
		out << "\"turning-1\":\"" << conflict->getFirstTurningId() << "\",";
		out << "\"conflict-dist-1\":\""<< std::setprecision(8) << conflict->getFirstConflictDistance() << "\",";
		out << "\"turning-2\":\"" << conflict->getSecondTurningId() << "\",";
		out << "\"conflict-dist-2\":\"" << std::setprecision(8) << conflict->getSecondConflictDistance() << "\",";
		out << "})\n";
	}

	printToFile(out);
}

void NetworkPrinter::printBusStops(const std::map<unsigned int, BusStop *> &stops) const
{
	std::stringstream out;
	out << std::setprecision(8);
	
	for(std::map<unsigned int, BusStop *>::const_iterator it = stops.begin(); it != stops.end(); ++it)
	{
		const BusStop *stop = it->second;
		out << "(\"bus-stop\", 0, " << stop->getStopId() << ", {";
		out << "\"code\":\"" << stop->getStopCode() << "\",";
		out << "\"name\":\"" << stop->getStopName() << "\",";
		out << "\"length\":\"" << stop->getLength() << "\",";
		out << "\"segment\":\"" << stop->getRoadSegmentId() << "\",";
		out << "\"location\":\"[";
		out << "(" << stop->getStopLocation().getX() << "," << stop->getStopLocation().getY() << "),";
		out << "]\",";		
		out << "})\n";
	}
	
	printToFile(out);
}

void NetworkPrinter::printTrafficSensors(const std::map<unsigned int, TrafficSensor *> &sensors) const
{
	std::stringstream out;
	out << std::setprecision(8);

	for(std::map<unsigned int, TrafficSensor *>::const_iterator it = sensors.begin(); it != sensors.end(); ++it)
	{
		const TrafficSensor *sensor = it->second;
		out << "(\"sensor\", 0, " << sensor->getSensorId() << ", {";
		out << "\"type\":\"" << sensor->getSensorType() << "\",";
		out << "\"task_code\":\"" << sensor->getTaskCode() << "\",";
		out << "\"zone-length\":\"" << sensor->getZoneLength() << "\",";
		out << "\"segment\":\"" << sensor->getSegmentId() << "\",";
		out << "\"offset\":\"" << sensor->getOffsetDistance() << "\",";
		out << "\"work-prob\":\"" << sensor->getWorkingProbability() << "\",";
		out << "\"lane\":\"" << sensor->getLaneId() << "\",";
		out << "\"signal-id\":\"" << sensor->getTrafficLightId() << "\",";
		out << "})\n";
	}

	printToFile(out);
}

void NetworkPrinter::printToFile(const std::stringstream& str) const
{
	//Print to file.
	out << str.str() << std::endl;
}
