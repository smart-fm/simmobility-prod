//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>

#include "logging/Log.hpp"
#include "SOCI_Converters.hpp"
#include "soci.h"
#include "soci-postgresql.h"
#include "RoadNetwork.hpp"

using namespace std;

namespace simmobility_network
{

  class NetworkLoader
  {
  private:
    
    //Pointer to the singleton instance
    static NetworkLoader *networkLoader;
    
    //Represents the road network that is loaded
    RoadNetwork *roadNetwork;
    
    //The database connection session
    soci::session sql;
    
    NetworkLoader();
    
    virtual ~NetworkLoader();
    
    //Returns the required stored procedure from the map of stored procedures
    string getStoredProcedure(map<string, string> const &storedProcs, string const &procedureName, bool mandatory);
    
    //Loads the Lanes
    void LoadLanes(const std::string& storedProc);
    
    //Loads the lane connectors
    void LoadLaneConnectors(const std::string& storedProc);
    
    //Loads the lane poly-lines
    void LoadLanePolyLines(const std::string& storedProc);
    
    //Loads the Links
    void LoadLinks(const std::string& storedProc);
    
    //Loads the Nodes
    void LoadNodes(const std::string& storedProc);
    
    //Load the road segments
    void LoadRoadSegments(const std::string& storedProc);
    
    //Loads the road segment poly-lines
    void LoadSegmentPolyLines(const std::string& storedProc);
    
    //Loads the turning conflicts
    void LoadTurningConflicts(const std::string& storedProc);
    
    //Loads the turning groups
    void LoadTurningGroups(const std::string& storedProc);
    
    //Loads the turning paths
    void LoadTurningPaths(const std::string& storedProc);
    
    //Loads the poly-lines associated with the turnings
    void LoadTurningPolyLines(const std::string& storedProc);
    
  public:    
    
    //Returns a pointer to the singleton instance of the class
    static NetworkLoader* getInstance();
    
    //Destroys the singleton instance
    static void destroyInstance();
    
    //Returns a pointer to the road network
    RoadNetwork* getRoadNetwork() const;
    
    //Loads the components of the network from the database
    void LoadNetwork(const string& connectionStr, const map<string, string>& storedProcs);

  } ;
}
