/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>


namespace sim_mob {

class Config;
class RoadSegment;
class BusStop;
class Crossing;
class LaneConnector;


/**
 * Class used to print output after loading a config file.
 * Typically used like a verb:
 *     Config cfg = //load cfg somehow
 *     PrintNetwork print(cfg);
 */
class PrintNetwork : private boost::noncopyable {
public:
	///Print the network output for a given Config file.
	PrintNetwork(const Config& cfg);

protected:
	///Print the network to "LogOut", using the old network format.
	void LogNetworkLegacyFormat() const;

private:
	//These functions are called by LogNetworkLegacyFormat()
	void LogLegacySimulationProps() const;
	void LogLegacySignalProps() const;
	void LogLegacyUniNodeProps(std::set<const sim_mob::RoadSegment*>& cachedSegments) const;
	void LogLegacyMultiNodeProps(std::set<const sim_mob::RoadSegment*>& cachedSegments, std::set<sim_mob::LaneConnector*>& cachedConnectors) const;
	void LogLegacyLinks() const;
	void LogLegacySegment(const sim_mob::RoadSegment* const rs, std::set<const sim_mob::Crossing*>& cachedCrossings, std::set<const sim_mob::BusStop*>& cachedBusStops) const;
	void LogLegacySegPolyline(const sim_mob::RoadSegment* const rs) const;
	void LogLegacySegLanes(const sim_mob::RoadSegment* const rs) const;
	void LogLegacyCrossing(const sim_mob::Crossing* const cr) const;
	void LogLegacyBusStop(const sim_mob::BusStop* const bs) const;
	void LogLegacyLaneConnectors(const sim_mob::LaneConnector* const lc) const;

private:
	//The config file we are currently printing.
	const Config& cfg;

	//Where we are printing it.
	mutable std::ofstream out; //The const exists for the config file; the ostream is obviously mutable.
};

}
