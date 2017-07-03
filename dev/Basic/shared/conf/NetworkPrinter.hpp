//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "geospatial/network/RoadNetwork.hpp"

namespace sim_mob
{

class ConfigParams;

/**
 * This class is used for printing the road network to an output file
 */
class NetworkPrinter : private boost::noncopyable
{
private:
	/**The configuration parameters*/
	ConfigParams &cfg;

	/**The output stream to which we are writing the file*/
	mutable std::ofstream out;

	/**Prints the simulation wide properties and the header*/
	void printSimulationProperties() const;

	/**
	 * Prints the nodes
	 * @param nodes map of id vs nodes
	 */
	void printNodes(const std::map<unsigned int, Node *> &nodes) const;

	/**
	 * Prints the links
	 * @param links map of id vs links
	 */
	void printLinks(const std::map<unsigned int, Link *> &links) const;

	/**
	 * Prints the segments, their poly-lines and lanes
	 * @param segments map of id vs segments
	 */
	void printSegments(const std::map<unsigned int, RoadSegment *> &segments) const;

	/**
	 * Prints the lane connectors
	 * @param lanes map of is vs lanes
	 */
	void printLaneConnectors(const std::map<unsigned int, Lane *> &lanes) const;

	/**
	 * Prints the turning groups 
	 * @param turningGroups map of id vs turning-groups
	 */
	void printTurningGroups(const std::map<unsigned int, TurningGroup *>& turningGroups) const;

	/**
	 * Prints the turning paths
	 * @param turnings map of id vs turning-paths
	 */
	void printTurnings(const std::map<unsigned int, TurningPath *> &turnings) const;

	/**
	 * Prints the turning conflicts 
	 * @param conflicts map of id vs turning-conflicts
	 */
	void printConflicts(const std::map<unsigned int, TurningConflict *> &conflicts) const;	

	/**
	 * Prints the bus stops
	 * @param stops map of id vs bus stops
	 */
	void printBusStops(const std::map<unsigned int, BusStop *> &stops) const;

	/**
	 * Prints the parking slots
	 * @param parkingSlots map of id vs the parking slots
	 */
	void printParkingSlots(const std::map<unsigned int, ParkingSlot *> &parkingSlots) const;

	/**
 	* Prints the parking Info (For Oncall & MRT)
 	* @param parkingDetails map of id vs the parking Detail (Info)
 	*/
	void printParkingDetails(const std::map<unsigned int, ParkingDetail *> &parkingDetails) const;

	/**
	 * This method prints the stream to the output file.
     * @param str the stream to be written
     */
	void printToFile(const std::stringstream& str) const;

public:
	NetworkPrinter(ConfigParams& cfg, const std::string& outFileName);

	/**
	 * Prints the road network to the output file
	 * @param network the road network
	 */
	void printNetwork(const RoadNetwork *network) const;
	
	/**
	 * This method prints the pre-generated information about traffic signals
	 * @param signalsInfo information about signals to be printed
	 */
	void printSignals(const std::string &signalsInfo) const;
};

}
