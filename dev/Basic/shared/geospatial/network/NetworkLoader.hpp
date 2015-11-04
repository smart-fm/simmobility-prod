//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>

using namespace std;

namespace sim_mob
{

class RoadNetwork;
/**
 * class for loading the network for simulation
 * \author Neeraj D
 * \author Harish L
 */
class NetworkLoader
{
private:
	/**Pointer to the singleton instance*/
	static NetworkLoader *networkLoader;

	/**Represents the road network that is loaded*/
	RoadNetwork *roadNetwork;

	/**The database connection session*/
	soci::session sql;

	/**Indicates whether the road network has been loaded successfully*/
	bool isNetworkLoaded;

	/**Private constructor as the class is a singleton*/
	NetworkLoader();

	/**
	 * Loads the lanes using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadLanes(const std::string& storedProc);

	/**
	 * Loads the lane connectors using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadLaneConnectors(const std::string& storedProc);

	/**
	 * Loads the lane poly-lines using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadLanePolyLines(const std::string& storedProc);

	/**
	 * Loads the Links using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadLinks(const std::string& storedProc);

	/**
	 * Loads the Nodes using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadNodes(const std::string& storedProc);

	/**
	 * Load the road segments using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadRoadSegments(const std::string& storedProc);

	/**
	 * Loads the road segment poly-lines using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadSegmentPolyLines(const std::string& storedProc);

	/**
	 * Loads the turning conflicts using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadTurningConflicts(const std::string& storedProc);

	/**
	 * Loads the turning groups using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadTurningGroups(const std::string& storedProc);

	/**
	 * Loads the turning paths using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadTurningPaths(const std::string& storedProc);

	/**
	 * Loads the poly-lines associated with the turnings using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadTurningPolyLines(const std::string& storedProc);

	/**
	 * Loads bus stops associated with parent road segment using the given stored procedure
	 *
	 * @param storedProc - the stored procedure to be executed in order to retrieve the data
	 */
	void loadBusStops(const std::string& storedProc);

public:
	virtual ~NetworkLoader();

	static NetworkLoader* getInstance();
	static void deleteInstance();

	/**
	 * Connects to the database using the given connection string and then loads the components of the
	 * network from the database using the stored procedures specified in the given map of stored procedures
	 *
	 * @param connectionStr - the database connection string
	 * @param storedProcs - the map of stored procedures
	 */
	void loadNetwork(const string& connectionStr, const map<string, string>& storedProcs);

	/**
	 * This method does post processing on the road network
     */
	void processNetwork();
};
}
