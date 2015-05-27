//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "NetworkLoader.hpp"

using namespace simmobility_network;

NetworkLoader* NetworkLoader::networkLoader = NULL;

NetworkLoader::NetworkLoader()
{
	roadNetwork = new RoadNetwork();
}

NetworkLoader::~NetworkLoader()
{
	if(roadNetwork)
	{
		delete roadNetwork;
		roadNetwork = NULL;
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

void NetworkLoader::destroyInstance()
{
	if(networkLoader)
	{
		delete networkLoader;
		networkLoader = NULL;
	}
}

string NetworkLoader::getStoredProcedure(const map<string,string>& storedProcs, const string& procedureName, bool mandatory = true)
{
	//Look for the stored procedure in the map
	map<string, string>::const_iterator itProcedure = storedProcs.find(procedureName);

	//Stored procedure found, return it
	if (itProcedure != storedProcs.end())
	{
		return itProcedure->second;
	} 
	//Stored procedure not found, return empty string if not mandatory
	else if(!mandatory)
	{
		return "";
	}
	//Mandatory stored procedure not found, throw exception
	else
	{
		throw std::runtime_error("Stored-procedure '" + procedureName + "' not found in the configuration file");
	}
}

void NetworkLoader::LoadLanes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Lane> lanes = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Lane>::const_iterator itLanes = lanes.begin(); itLanes != lanes.end(); ++itLanes)
	{
		//Create new lane and add it to the segment to which it belongs
		Lane *lane = new Lane(*itLanes);
		roadNetwork->addLane(lane);
	}
}

void NetworkLoader::LoadLanePolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<simmobility_network::Point> points = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Point>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		Point *point = new Point(*itPoints);
		roadNetwork->addLanePolyLine(point);
	}
}

void NetworkLoader::LoadLinks(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Link> links = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Link>::const_iterator itLinks = links.begin(); itLinks != links.end(); ++itLinks)
	{
		//Create new node and add it in the map of nodes
		Link* link = new Link(*itLinks);
		roadNetwork->addLink(link);
	}
}

void NetworkLoader::LoadNodes(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<Node> nodes = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Node>::const_iterator itNodes = nodes.begin(); itNodes != nodes.end(); ++itNodes)
	{
		//Create new node and add it in the map of nodes
		Node* node = new Node(*itNodes);
		roadNetwork->addNode(node);
	}
}

void NetworkLoader::LoadRoadSegments(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<RoadSegment> segments = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<RoadSegment>::const_iterator itSegments = segments.begin(); itSegments != segments.end(); ++itSegments)
	{
		//Create new road segment and add it to the link to which it belongs
		RoadSegment *segment = new RoadSegment(*itSegments);
		roadNetwork->addRoadSegment(segment);
	}
}

void NetworkLoader::LoadSegmentPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<simmobility_network::Point> points = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Point>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		Point *point = new Point(*itPoints);
		roadNetwork->addSegmentPolyLine(point);
	}
}

void NetworkLoader::LoadTurningConflicts(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningConflict> turningConflicts = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<TurningConflict>::const_iterator itTurningConflicts = turningConflicts.begin(); itTurningConflicts != turningConflicts.end(); ++itTurningConflicts)
	{
		//Create new turning conflict and add it to the turning paths to which it belongs
		TurningConflict* turningConflict = new TurningConflict(*itTurningConflicts);		
		roadNetwork->addTurningConflict(turningConflict);
	}
}

void NetworkLoader::LoadTurningGroups(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningGroup> turningGroups = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<TurningGroup>::const_iterator itTurningGroups = turningGroups.begin(); itTurningGroups != turningGroups.end(); ++itTurningGroups)
	{
		//Create new turning group and add it in the map of turning groups
		TurningGroup* turningGroup = new TurningGroup(*itTurningGroups);		
		roadNetwork->addTurningGroup(turningGroup);
	}
}

void NetworkLoader::LoadTurningPaths(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<TurningPath> turningPaths = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<TurningPath>::const_iterator itTurningPaths = turningPaths.begin(); itTurningPaths != turningPaths.end(); ++itTurningPaths)
	{
		//Create new turning path and add it in the map of turning paths
		TurningPath* turningPath = new TurningPath(*itTurningPaths);		
		roadNetwork->addTurningPath(turningPath);
	}
}

void NetworkLoader::LoadTurningPolyLines(const std::string& storedProc)
{
	//SQL statement
	soci::rowset<simmobility_network::Point> points = (sql.prepare << "select * from " + storedProc);
	
	for(soci::rowset<Point>::const_iterator itPoints = points.begin(); itPoints != points.end(); ++itPoints)
	{
		//Create new point and add it to the poly-line, to which it belongs
		Point *point = new Point(*itPoints);
		roadNetwork->addTurningPolyLine(point);
	}
}

void NetworkLoader::LoadNetwork(const string& connectionStr, const map<string,string>& storedProcs)
{
	try
	{
		sim_mob::Print() << "Connecting to the database...\n";

		//Open the connection to the database
		sql.open(soci::postgresql, connectionStr);
		
		//Load the components of the network

		LoadNodes(getStoredProcedure(storedProcs, "nodes"));
		
		LoadLinks(getStoredProcedure(storedProcs, "links"));
		
		LoadTurningGroups(getStoredProcedure(storedProcs, "turning_groups"));
		
		LoadTurningPaths(getStoredProcedure(storedProcs, "turning_paths"));
		
		LoadTurningPolyLines(getStoredProcedure(storedProcs, "turning_polylines"));
		
		LoadTurningConflicts(getStoredProcedure(storedProcs, "turning_conflicts"));
		
		LoadRoadSegments(getStoredProcedure(storedProcs, "road_segments"));
		
		LoadSegmentPolyLines(getStoredProcedure(storedProcs, "segment_polylines"));
		
		LoadLanes(getStoredProcedure(storedProcs, "lanes"));
		
		LoadLanePolyLines(getStoredProcedure(storedProcs, "lane-polylines"));

		//Close the connection
		sql.close();

		sim_mob::Print() << "SimMobility network loaded!\n";
	}
	catch (soci::soci_error const &err)
	{
		sim_mob::Print() << "Exception occurred while loading the network!\n" << err.what() << std::endl;
	}
	catch(runtime_error const &err)
	{
		sim_mob::Print() << "Exception occurred while loading the network!\n" << err.what() << std::endl;
	}
}