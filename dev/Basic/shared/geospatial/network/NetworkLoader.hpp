//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include "soci.h"
#include "soci-postgresql.h"
#include "RoadNetwork.hpp"

using namespace std;

namespace simmobility_network
{
  /**
   * class for loading the network for simulation
   * \author Neeraj D
   * \author Harish L
   */
  class NetworkLoader
  {
  private:
    //Pointer to the singleton instance
    static NetworkLoader* networkLoader;
    
    //Represents the road network that is loaded
    RoadNetwork* roadNetwork;
    
    //The database connection session
    soci::session sql;
    
    //Loads the Lanes
    void loadLanes(const std::string& storedProc);
    
    //Loads the lane connectors
    void loadLaneConnectors(const std::string& storedProc);
    
    //Loads the lane poly-lines
    void loadLanePolyLines(const std::string& storedProc);
    
    //Loads the Links
    void loadLinks(const std::string& storedProc);
    
    //Loads the Nodes
    void loadNodes(const std::string& storedProc);
    
    //Load the road segments
    void loadRoadSegments(const std::string& storedProc);
    
    //Loads the road segment poly-lines
    void loadSegmentPolyLines(const std::string& storedProc);
    
    //Loads the turning conflicts
    void loadTurningConflicts(const std::string& storedProc);
    
    //Loads the turning groups
    void loadTurningGroups(const std::string& storedProc);
    
    //Loads the turning paths
    void loadTurningPaths(const std::string& storedProc);
    
    //Loads the poly-lines associated with the turnings
    void loadTurningPolyLines(const std::string& storedProc);
    
  public:    
    NetworkLoader();
    virtual ~NetworkLoader();

    //Returns a pointer to the road network
    RoadNetwork* getRoadNetwork() const;
    
    //Loads the components of the network from the database
    void loadNetwork(const string& connectionStr, const map<string, string>& storedProcs);
  };
}
