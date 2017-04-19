#pragma once

#include <fstream>
#include <mutex>
#include <vector>

#include "config/MT_Config.hpp"
#include "entities/misc/TripChain.hpp"

namespace sim_mob
{

namespace medium
{

/**
 * Singleton class to collect and print the trip chain details from withinday
 */

class TripChainOutput
{
public:
	/**
	 * Function to get singleton instance of TripChainOutput
	 *
	 * @return TripChainOutput singleton reference
	 */
	static TripChainOutput& getInstance();

	/**
	 * Print the trip chain created in withinday model
	 *
	 * @param tripChain - TripChain to be printed
	 */
	void printTripChain(const std::vector<TripChainItem*>& tripChain);
private:
	TripChainOutput();
	~TripChainOutput();

	/**
	 * Helper function to get the stop id of waypoint (node/busstop/mrt)
	 * @param wayPoint stop for which the id is to be retrieved
	 * @return stop id
	 */
	std::string getStopId(const WayPoint& wayPoint);

	/// Singleton instance
	static TripChainOutput* instance;
	/// Mutex to multiple creation of singleton instances
	static std::mutex instanceMutex;
	/// File where trips and activities are printed
	std::ofstream tripActivityFile;
	/// Mutex for trips and activities file, since the request to print will be from multiple threads
	std::mutex tripActivityMutex;

	/// File where subtrips are printed
	std::ofstream subTripFile;
	/// Mutex for subtrips file, since the request to print will be from multiple threads
	std::mutex subTripMutex;

	/// Reference to MT_Config instance
	const MT_Config& mtCfg;
};

}

}
