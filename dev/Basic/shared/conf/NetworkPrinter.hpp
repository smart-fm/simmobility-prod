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
	void PrintSimulationProperties() const;

	/**
	 * Prints the nodes
	 * @param nodes map of id vs nodes
	 */
	void PrintNodes(const std::map<unsigned int, Node *> &nodes) const;

	/**
	 * Prints the links
	 * @param links map of id vs links
	 */
	void PrintLinks(const std::map<unsigned int, Link *> &links) const;

	/**
	 * Prints the segments, their poly-lines and lanes
	 * @param segments map of id vs segments
	 */
	void PrintSegments(const std::map<unsigned int, RoadSegment *> &segments) const;

	/**
	 * Prints the lane connectors
	 * @param lanes map of is vs lanes
	 */
	void PrintLaneConnectors(const std::map<unsigned int, Lane *> &lanes) const;

	/**
	 * Prints the turning groups 
	 * @param turningGroups map of id vs turning-groups
	 */
	void PrintTurningGroups(const std::map<unsigned int, TurningGroup *>& turningGroups) const;

	/**
	 * Prints the turning paths
	 * @param turnings map of id vs turning-paths
	 */
	void PrintTurnings(const std::map<unsigned int, TurningPath *> &turnings) const;

	/**
	 * Prints the turning conflicts 
	 * @param conflicts map of id vs turning-conflicts
	 */
	void PrintConflicts(const std::map<unsigned int, TurningConflict *> &conflicts) const;

	/**
	 * Prints the traffic signals
	 * @param signals map of id vs traffic signals
	 */
	void PrintSignals(const std::map<unsigned int, Signal *> &signals) const;

	/**
	 * Prints the bus stops
	 * @param stops map of id vs bus stops
	 */
	void PrintBusStops(const std::map<unsigned int, BusStop *> &stops) const;

	/**
	 * This method prints the stream to the output file and to the GUI, if Interactive mode is on.
     * @param str the stream to be written
     */
	void PrintToFileAndGui(const std::stringstream& str) const;

public:
	NetworkPrinter(ConfigParams& cfg, const std::string& outFileName);

protected:
	/**
	 * Prints the road network to the output file
	 * @param network the road network
	 */
	void PrintNetwork(const RoadNetwork *network) const;
};

}
