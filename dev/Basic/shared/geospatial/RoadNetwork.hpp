//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <set>
#include <climits>
#include "geospatial/RoadRunnerRegion.hpp"
#include "TurningConflict.hpp"
#include "TurningPolyline.hpp"
#include "TurningSection.hpp"

namespace sim_mob
{

//Forward declarations
class Node;
class UniNode;
class MultiNode;
class Point2D;
class Link;
class Conflux;
class CoordinateTransform;
class TurningSection;
class TurningConflict;
class TurningPolyline;

namespace aimsun
{
//Forward declaration
class Loader;
}


/**
 * The side of the road on which cars drive.
 *
 * \author Seth N. Hetu
 *
 * For the USA, this is DRIVES_ON_RIGHT;
 * for Singapore it is DRIVES_ON_LEFT. This affects the context of "can_turn_right_on_red",
 * and may affect lane merging rules.
 */
enum DRIVING_SIDE {
	DRIVES_ON_LEFT,
	DRIVES_ON_RIGHT
};


/**
 * The main Road Network. (Currently, this class only contains the "drivingSide" variable)
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 */

/*Added  by vahid*/

class RoadNetwork {
	friend class sim_mob::aimsun::Loader;
public:
	RoadNetwork() { drivingSide=DRIVES_ON_LEFT; } //TEMP
	~RoadNetwork();

	DRIVING_SIDE drivingSide;


	/**
	 * Forces all lane-edge and lane polylines to generate.
	 * For legacy reasons, our Road Network doesn't always generate lane lines and lane-edge lines.
	 * This function iterates through each Segment in a RoadNetwork in order of increasing ID, and
	 * generates both the lane-edge and lane polylines. The ordered ID should ensure that each
	 * lane/lane edge is given the same LaneID for multiple runs of Sim Mobility.
	 *
	 * \todo
	 * This function needs to be migrated to the Database and XML loaders. In other words,
	 * Road Networks should *not* delay loading of lane edge polylines, and should not have any
	 * mutable properties at runtime (these should go into StreetDirectory decorator classes).
	 */
	static void ForceGenerateAllLaneEdgePolylines(sim_mob::RoadNetwork& rn);


	///Retrieve list of all Uni/MultiNodes (intersections & roundabouts) in this Road Network.
	///
	///\todo This needs to eventually have some structure; see the wiki for an example.
	std::vector<sim_mob::MultiNode*>& getNodes();
	const std::vector<sim_mob::MultiNode*>& getNodes() const;
	std::set<sim_mob::UniNode*>& getUniNodes();
	const std::set<sim_mob::UniNode*>& getUniNodes() const;

	///Retrieve a list of all Links (high-level paths between MultiNodes) in this Road Network.
	std::vector<sim_mob::Link*>& getLinks();
	const std::vector<sim_mob::Link*>& getLinks() const;

	///Find the closest Node.
	///If includeUniNodes is false, then only Intersections and Roundabouts are searched.
	///If no node is found within maxDistCM, the match fails and nullptr is returned.
	sim_mob::Node* locateNode(const sim_mob::Point2D& position, bool includeUniNodes=false, int maxDistCM=1e8) const;
	sim_mob::Node* locateNode(double xPos, double yPos, bool includeUniNodes=false, int maxDistCM=1e8) const;

	sim_mob::Node* getNodeById(int aimsunId);
	sim_mob::MultiNode* getMultiNodeById(std::string aimsunId);
	std::map<int,sim_mob::Node*> nodeMap;

	//Temporary; added for the XML loader
	void setLinks(const std::vector<sim_mob::Link*>& lnks);
	void setSegmentNodes(const std::set<sim_mob::UniNode*>& sn);
	void addNodes(const std::vector<sim_mob::MultiNode*>& vals);

	///Retrieve the first CoordinateTransform; throws an error if none exist.
	sim_mob::CoordinateTransform* getCoordTransform(bool required=true) const;

//private:
	//Temporary: Geometry will eventually make specifying nodes and links easier.
	std::vector<sim_mob::MultiNode*> nodes;
	std::vector<sim_mob::Link*> links;

	//Temporary: Not exposed publicly
	std::set<sim_mob::UniNode*> segmentnodes;

	//List of CoordinateTransforms this map contains. Only the first is guaranteed to be valid.
	std::vector<sim_mob::CoordinateTransform*> coordinateMap;

	//List of Road Runner Regions, if applicable.
	std::map<int, sim_mob::RoadRunnerRegion> roadRunnerRegions;

	///to store simmobility segment type table data, key = segment aimsun id,value=type
	///segment type 1:freeway 2:ramp 3:urban road
	std::map<std::string,int> segmentTypeMap;

	/// Map to store the Turning sections, Key = Turning id, Vaule = TurningSection
	std::map<std::string,sim_mob::TurningSection* > turningSectionMap;
    
	/// Getter for the map of Turning sections
	const  std::map<std::string,sim_mob::TurningSection* >& getTurnings() const  { return turningSectionMap;}

	/// Map to store Turning sections with 'from  segment' as the search key
	/// Key = from segment aimsun id, Value = Turning
	std::map<std::string,sim_mob::TurningSection* > turningSectionByFromSeg;
	
	/// Map to store Turning sections with 'to  segment' as the search key
	/// Key= to segment aimsun seg id, Value = Turning
	std::map<std::string,sim_mob::TurningSection* > turningSectionByToSeg;
    
	/// Map to store the Turning conflicts, Key = Turning id, Vaule = TurningConflict
	std::map<std::string,sim_mob::TurningConflict* > turningConflictMap;
	
	/// Getter for the map of Turning conflicts
	const std::map<std::string,sim_mob::TurningConflict* >& getConflicts() const { return turningConflictMap;}
	
	/// Stores the Turning section from the database to the road network object
	void storeTurningSection( sim_mob::TurningSection* turning);

	/// Map to store the Turning poly-lines, Key = Poly-line id, Value = Poly-line
	std::map<int,sim_mob::TurningPolyline* > turningPolylineMap;
        
	/// Stores the turning poly-lines from the database to the road network object
	void storeTurningPolyline(sim_mob::TurningPolyline* tp);

	/// Stores the turning poly-lines from the database to the road network object
	void storeTurningConflict( sim_mob::TurningConflict* t);
        
	/// Stores the turning poly-lines from the database to the road network object
	sim_mob::TurningSection* findTurningById(std::string id);
        
	/// Builds the map of segment id vs road segment
	void makeSegPool();
        
	///	Map for storing all segments <aimsun id ,seg>
	std::map<std::string,sim_mob::RoadSegment*> segPool;
    
	/// Returns the required segment
	sim_mob::RoadSegment* getSegById(std::string aimsunId);
        
	/**
	 * /brief get segment type by aimsun id
	 * /param id segment aimsun id
	 * /return type id, if not find in segmentTypeMap return 0, which is unknown
	 */
	int getSegmentType(std::string& id);
	
	///to store simmobility node type table data, key = ndoe aimsun id,value=type
	///node type 1:urban intersection with signal 2:urban intersection w/o signal 3:priority merge 4:non-priority merge
    std::map<std::string,int> nodeTypeMap;
	
	/**
	 * /brief get node type by aimsun id
	 * /param id segment aimsun id
	 * /return type id, if not find in nodeTypeMap return 0, which is unknown
	 */
	int getNodeType(std::string& id);
};





}
