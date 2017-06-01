//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NetworkLoader.hpp"

#include <stdexcept>
#include "logging/Log.hpp"
#include "SOCI_Converters.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/vehicle/VehicleBase.hpp"

using namespace sim_mob;

NetworkLoader* NetworkLoader::networkLoader = NULL;

namespace
{
const unsigned int TWIN_STOP_ID_START = 9900000;

/**Returns the required stored procedure from the map of stored procedures*/
string getStoredProcedure(const map<string, string>& storedProcs, const string& procedureName, bool mandatory = true)
{
	//Look for the stored procedure in the map
	map<string, string>::const_iterator itProcedure = storedProcs.find(procedureName);

	//Stored procedure found, return it
	if (itProcedure != storedProcs.end())
	{
		return itProcedure->second;
	}
	//Stored procedure not found, return empty string if not mandatory
	else if (!mandatory)
	{
		return "";
	}
	//Mandatory stored procedure not found, throw exception
	else
	{
		throw std::runtime_error("Stored-procedure '" + procedureName + "' not found in the configuration file");
	}
}
}

NetworkLoader::NetworkLoader() : roadNetwork(RoadNetwork::getWritableInstance()), isNetworkLoaded(false)
{
}

NetworkLoader::~NetworkLoader()
{
	safe_delete_item(roadNetwork);
}

void NetworkLoader::loadLanes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Lane> lanes = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<Lane>::const_iterator itLanes = lanes.begin(); itLanes != lanes.end(); ++itLanes)
	{
		//Create new lane and add it to the segment to which it belongs
		Lane *lane = new Lane(*itLanes);

		try
		{
			roadNetwork->addLane(lane);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long lanesLoaded = roadNetwork->getMapOfIdVsLanes().size();

	if(lanesLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 lanes!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Lanes\t\t\t\t\t|\t" << lanesLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadLaneConnectors(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<LaneConnector> connectors = (sql.prepare << "select * from " + storedProc);
	unsigned long connectorsLoaded = 0;

	for (soci::rowset<LaneConnector>::const_iterator itConnectors = connectors.begin(); itConnectors != connectors.end(); ++itConnectors)
	{
		//Create new lane connector and add it to the lane to which it belongs
		LaneConnector *connector = new LaneConnector(*itConnectors);

		try
		{
			roadNetwork->addLaneConnector(connector);
			connectorsLoaded++;
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	if(connectorsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 lane connectors!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Lane connectors\t\t\t|\t" << connectorsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadLanePolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);
	unsigned int prevLineId = 0, linesLoaded = 0;

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);

		try
		{
			roadNetwork->addLanePolyLine(point);

			if(prevLineId != point.getPolyLineId())
			{
				prevLineId = point.getPolyLineId();
				linesLoaded++;
			}
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	if(linesLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 links!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Lane poly-lines\t\t\t|\t" << linesLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadLinks(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Link> links = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<Link>::const_iterator itLinks = links.begin(); itLinks != links.end(); ++itLinks)
	{
		//Create new node and add it in the map of nodes
		Link* link = new Link(*itLinks);

		try
		{
			roadNetwork->addLink(link);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long linksLoaded = roadNetwork->getMapOfIdVsLinks().size();

	if(linksLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 links!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Links\t\t\t\t\t|\t" << linksLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadNodes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Node> nodes = (sql.prepare << "select * from " + storedProc);
	std::set<sim_mob::Node*> nodesSet;
	for (soci::rowset<Node>::const_iterator itNodes = nodes.begin(); itNodes != nodes.end(); ++itNodes)
	{
		//Create new node and add it in the map of nodes
		Node* node = new Node(*itNodes);
		roadNetwork->addNode(node);
		nodesSet.insert(node);
	}

	Node::allNodesMap.update(nodesSet);

	//Sanity check
	unsigned long nodesLoaded = roadNetwork->getMapOfIdvsNodes().size();

	if(nodesLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 nodes!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Nodes\t\t\t\t\t|\t" << nodesLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadRoadSegments(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<RoadSegment> segments = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<RoadSegment>::const_iterator itSegments = segments.begin(); itSegments != segments.end(); ++itSegments)
	{
		//Create new road segment and add it to the link to which it belongs
		RoadSegment *segment = new RoadSegment(*itSegments);

		try
		{
			roadNetwork->addRoadSegment(segment);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long segmentsLoaded = roadNetwork->getMapOfIdVsRoadSegments().size();

	if(segmentsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 road segments!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Segments\t\t\t\t|\t" << segmentsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadSegmentPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);
	unsigned int prevLineId = 0, linesLoaded = 0;

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);

		try
		{
			roadNetwork->addSegmentPolyLine(point);

			if(point.getPolyLineId() != prevLineId)
			{
				prevLineId = point.getPolyLineId();
				linesLoaded++;
			}
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	if(linesLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 road segment poly-lines!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Segment poly-lines\t\t|\t" << linesLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadTurningConflicts(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningConflict> turningConflicts = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningConflict>::const_iterator itTurningConflicts = turningConflicts.begin(); itTurningConflicts != turningConflicts.end(); ++itTurningConflicts)
	{
		//Create new turning conflict and add it to the turning paths to which it belongs
		TurningConflict* turningConflict = new TurningConflict(*itTurningConflicts);

		try
		{
			roadNetwork->addTurningConflict(turningConflict);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

#ifndef NDEBUG
	unsigned int conflictsLoaded = roadNetwork->getMapOfIdvsTurningConflicts().size();
	Print() << "Turning conflicts\t\t|\t" << conflictsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadTurningGroups(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningGroup> turningGroups = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningGroup>::const_iterator itTurningGroups = turningGroups.begin(); itTurningGroups != turningGroups.end(); ++itTurningGroups)
	{
		//Create new turning group and add it in the map of turning groups
		TurningGroup* turningGroup = new TurningGroup(*itTurningGroups);

		try
		{
			roadNetwork->addTurningGroup(turningGroup);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long turningsLoaded = roadNetwork->getMapOfIdvsTurningGroups().size();

	if(turningsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 turning groups!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Turning groups\t\t\t|\t" << turningsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadTurningPaths(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningPath> turningPaths = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<TurningPath>::const_iterator itTurningPaths = turningPaths.begin(); itTurningPaths != turningPaths.end(); ++itTurningPaths)
	{
		//Create new turning path and add it in the map of turning paths
		TurningPath* turningPath = new TurningPath(*itTurningPaths);

		try
		{
			roadNetwork->addTurningPath(turningPath);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long turningPathsLoaded = roadNetwork->getMapOfIdvsTurningPaths().size();

	if(turningPathsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 turning paths!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Turning paths\t\t\t|\t" << turningPathsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadTurningPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<PolyPoint> points = (sql.prepare << "select * from " + storedProc);
	unsigned int prevLineId = 0, linesLoaded = 0;

	for (soci::rowset<PolyPoint>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		PolyPoint point(*itPoints);

		try
		{
			roadNetwork->addTurningPolyLine(point);

			if(point.getPolyLineId() != prevLineId)
			{
				prevLineId = point.getPolyLineId();
				linesLoaded++;
			}
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	if(linesLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 turning path poly-lines!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Turning path poly-lines\t|\t" << linesLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadTaxiStands(const std::string& storedProc)
{
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();

	if(storedProc.empty())
	{
		Print() << "Optional data: Taxi stands not loaded. Stored procedure not provided\n";
		Warn() << "Stored procedure to load taxi stands not specified in the configuration file."
		       << "\nTaxi Stands not loaded..." << std::endl;
		return;
	}

	//SQL statement
	soci::rowset<sim_mob::TaxiStand> stands = (sql.prepare << "select * from " + storedProc);
	std::set<sim_mob::TaxiStand*> standSet;
	for (soci::rowset<TaxiStand>::const_iterator itStand = stands.begin(); itStand != stands.end(); ++itStand)
	{
		try
		{
			//Create new taxi stand and add it to road network
			TaxiStand* stand = new TaxiStand(*itStand);
			roadNetwork->addTaxiStand(stand);
			standSet.insert(stand);
			TaxiStand::allTaxiStandMap.update(standSet);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long taxiStandsLoaded = roadNetwork->getMapOfIdvsTaxiStands().size();

	if(taxiStandsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 taxi stands!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Taxi stands\t\t\t\t|\t" << taxiStandsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadSurveillanceStns(const string &storedProc)
{
	if(!storedProc.empty())
	{
		//SQL statement
		soci::rowset<soci::row> surveillanceStns = (sql.prepare << "select * from " + storedProc);

		unsigned int id, type, code, segmentId, trafficLight;
		double zone, offset;

		for(soci::rowset<soci::row>::const_iterator itStn = surveillanceStns.begin(); itStn != surveillanceStns.end(); ++itStn)
		{
			id = (*itStn).get<unsigned int>(0);
			type = (*itStn).get<unsigned int>(1);
			code = (*itStn).get<unsigned int>(2);
			zone = (*itStn).get<double>(3);
			offset = (*itStn).get<double>(4);
			segmentId = (*itStn).get<unsigned int>(5);
			trafficLight = (*itStn).get<unsigned int>(6);

			//Create a new surveillance station and add it to the network
			SurveillanceStation *station = new SurveillanceStation(id, type, code, zone, offset, segmentId, trafficLight);

			try
			{
				roadNetwork->addSurveillenceStn(station);
			}
			catch(runtime_error &ex)
			{
				std::stringstream msg;
				msg << ex.what() << "\nStored procedure: " << storedProc;
				throw std::runtime_error(msg.str());
			}
		}

		//Sanity check
		unsigned long survStnsLoaded = SurveillanceStation::surveillanceStations.size();

		if(survStnsLoaded == 0)
		{
			std::stringstream msg;
			msg << storedProc << " returned 0 traffic sensors!";
			throw runtime_error(msg.str());
		}

#ifndef NDEBUG
		Print() << "Traffic sensors\t\t\t|\t" << survStnsLoaded << "\t\t| " << storedProc << endl;
#endif
	}
	else
	{
		Print() << "Optional data: Traffic sensors not loaded. Stored procedure not provided\n";
	}
}

void NetworkLoader::loadBusStops(const std::string& storedProc)
{
	sim_mob::ConfigParams& config = sim_mob::ConfigManager::GetInstanceRW().FullConfig();
	if(!config.busController.enabled)
	{
		Print() << "Optional data: Bus stops not loaded. Bus controller is disabled.\n";
		Warn() << "\nBus controller is not enabled in the config file " << std::endl;
		return;
	}

	if (storedProc.empty())
	{
		Print() << "Optional data: Bus stops not loaded. Stored procedure not provided\n";
		Warn() << "Stored procedure to load bus stops not specified in the configuration file."
				<< "\nBus Stops not loaded..." << std::endl;
		return;
	}

	//SQL statement
	soci::rowset<sim_mob::BusStop> stops = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<BusStop>::const_iterator itStop = stops.begin(); itStop != stops.end(); ++itStop)
	{
		if (!sim_mob::ConfigManager::GetInstance().FullConfig().isGenerateBusRoutes() && itStop->getStopName().find("Virtual Bus Stop") != std::string::npos)
		{
			continue;
		}
		
		if (!itStop->getStopStatus().compare("NOP"))
		{
			continue;
		}

		//Create new bus stop and add it to road network
		BusStop* stop = new BusStop(*itStop);

		//hackish data validation to evade errors
		if(stop->getLength() < sim_mob::BUS_LENGTH)
		{
			stop->setLength(sim_mob::BUS_LENGTH);
		}

		try
		{
			roadNetwork->addBusStop(stop);

			if(stop->getReverseSectionId() != 0) // this condition is true only for bus interchange stops
			{
				//Create twin bus stop for this interchange stop and add it to road network
				BusStop* twinStop = new BusStop();
				unsigned int twinStopId = TWIN_STOP_ID_START + stop->getStopId(); // expected result is 990<orig. stop id>
				std::string twinStopCode = "twin_" + stop->getStopCode(); //"twin_<orig. stop code>
				twinStop->setVirtualStop();
				twinStop->setStopId(twinStopId);
				twinStop->setStopCode(twinStopCode);
				twinStop->setRoadSegmentId(stop->getReverseSectionId());
				twinStop->setStopName(stop->getStopName());
				twinStop->setLength(stop->getLength());
				twinStop->setOffset(stop->getOffset());
				twinStop->setReverseSectionId(stop->getRoadSegmentId());
				twinStop->setTerminalNodeId(stop->getTerminalNodeId());
				twinStop->setStopLocation(stop->getStopLocation());
				roadNetwork->addBusStop(twinStop);

				//source and sink must be determined after adding the stop since the parentSegment will be available only now
				if(stop->getParentSegment()->getParentLink()->getFromNode()->getNodeId() == stop->getTerminalNodeId())
				{
					stop->setTerminusType(sim_mob::SOURCE_TERMINUS);
					twinStop->setTerminusType(sim_mob::SINK_TERMINUS);
				}
				else if(stop->getParentSegment()->getParentLink()->getToNode()->getNodeId() == stop->getTerminalNodeId())
				{
					twinStop->setTerminusType(sim_mob::SOURCE_TERMINUS);
					stop->setTerminusType(sim_mob::SINK_TERMINUS);
				}
				else
				{
					throw std::runtime_error("Invalid assignment of terminal node for interchange bus stop");
				}
				stop->setTwinStop(twinStop);
				twinStop->setTwinStop(stop);

				//yet another hackish fix to make up for data errors
				//All bus interchanges are made long enough to accept at least 10 buses at a time
				stop->setLength(std::max(stop->getLength(), 10 * sim_mob::BUS_LENGTH));
				twinStop->setLength(std::max(twinStop->getLength(), 10 * sim_mob::BUS_LENGTH));
			}
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long busStopsLoaded = roadNetwork->getMapOfIdvsBusStops().size();

	if(busStopsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 bus stops!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Bus stops\t\t\t\t|\t" << busStopsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadParkingSlots(const std::string& storedProc)
{
	if (storedProc.empty())
	{
		Print() << "Optional data: Parking slots not loaded. Stored procedure not provided\n";
		Warn() << "Stored procedure to load parking slots not specified in the configuration file."
		       << "\nParking slots not loaded..." << std::endl;
		return;
	}

	//SQL statement
	soci::rowset<ParkingSlot> pkSlots = (sql.prepare << "select * from " + storedProc);

	for (soci::rowset<ParkingSlot>::const_iterator itPkSlots = pkSlots.begin(); itPkSlots != pkSlots.end(); ++itPkSlots)
	{
		//Create new parking slot and add it to the netowrk
		ParkingSlot *parkingSlot = new ParkingSlot(*itPkSlots);

		try
		{
			roadNetwork->addParking(parkingSlot);
		}
		catch(runtime_error &ex)
		{
			std::stringstream msg;
			msg << ex.what() << "\nStored procedure: " << storedProc;
			throw std::runtime_error(msg.str());
		}
	}

	//Sanity check
	unsigned long parkingSlotsLoaded = roadNetwork->getMapOfIdVsParkingSlots().size();

	if(parkingSlotsLoaded == 0)
	{
		std::stringstream msg;
		msg << storedProc << " returned 0 parking slots!";
		throw runtime_error(msg.str());
	}

#ifndef NDEBUG
	Print() << "Parking slots\t\t\t|\t" << parkingSlotsLoaded << "\t\t| " << storedProc << endl;
#endif
}

void NetworkLoader::loadNetwork(const string& connectionStr, const map<string, string>& storedProcs)
{
	try
	{
		//Open the connection to the database
		sql.open(soci::postgresql, connectionStr);

		//Load the components of the network

#ifndef NDEBUG
		Print() << "Network element\t\t\t|\t#Loaded\t| Stored procedure\n";
		Print() << "------------------------------------------------------\n";
#endif

		loadNodes(getStoredProcedure(storedProcs, "nodes"));

		loadLinks(getStoredProcedure(storedProcs, "links"));

		loadRoadSegments(getStoredProcedure(storedProcs, "road_segments"));

		loadSegmentPolyLines(getStoredProcedure(storedProcs, "segment_polylines"));

		loadLanes(getStoredProcedure(storedProcs, "lanes"));

		loadLanePolyLines(getStoredProcedure(storedProcs, "lane_polylines"));

		loadLaneConnectors(getStoredProcedure(storedProcs, "lane_connectors"));

		loadTurningGroups(getStoredProcedure(storedProcs, "turning_groups"));

		loadTurningPaths(getStoredProcedure(storedProcs, "turning_paths"));

		loadTurningPolyLines(getStoredProcedure(storedProcs, "turning_polylines"));

		loadTurningConflicts(getStoredProcedure(storedProcs, "turning_conflicts"));

		loadSurveillanceStns(getStoredProcedure(storedProcs, "traffic_sensors", false));

		loadBusStops(getStoredProcedure(storedProcs, "bus_stops", false));

		loadParkingSlots(getStoredProcedure(storedProcs, "parking_slots", false));

		loadTaxiStands(getStoredProcedure(storedProcs, "taxi_stands", false));

		//Close the connection
		sql.close();

		isNetworkLoaded = true;

		Print() << "\nSimMobility Road Network loaded from database\n";
	}
	catch (soci::soci_error const &err)
	{
		Print() << "Exception occurred while loading the network!\n" << err.what() << std::endl;
		exit(-1);
	}
	catch (runtime_error const &err)
	{
		Print() << "Exception occurred while loading the network!\n" << err.what() << std::endl;
		exit(-1);
	}
}

void NetworkLoader::processNetwork()
{
	//Calculate the lengths of all the links
	const std::map<unsigned int, Link *> &mapOfLinks = roadNetwork->getMapOfIdVsLinks();
	std::map<unsigned int, Link *>::const_iterator itLinks = mapOfLinks.begin();

	while(itLinks != mapOfLinks.end())
	{
		itLinks->second->calculateLength();
		++itLinks;
	}
}

NetworkLoader* NetworkLoader::getInstance()
{
	if(networkLoader == NULL)
	{
		networkLoader = new NetworkLoader();
	}

	return networkLoader;
}

void NetworkLoader::deleteInstance()
{
	safe_delete_item(networkLoader);
}
