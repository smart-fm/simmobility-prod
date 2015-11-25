/*
 * ScreenLineCounter.hpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#pragma once

#include <set>
#include <boost/thread/mutex.hpp>

namespace sim_mob
{
namespace medium
{

class ScreenLineCounter {
public:
    static ScreenLineCounter* getInstance();

    /**
     * Update the count of vehicles passing through a screen line segment
     *
     * @param segId Road Segment id
     * @param entryTimeSec time of entry (in seconds) into road segment
     * @param travelMode vehicle mode
     */
    void updateScreenLineCount(unsigned int segId, double entryTimeSec, const std::string& travelMode);

    /**
     * Export the screen line count to a file.
     */
    void exportScreenLineCount() const;

private:
    struct VehicleCount
    {
    	unsigned int count;
    	VehicleCount() : count(0)
    	{
    	}
    };

	/** time interval */
	typedef unsigned int TimeInterval;

	/** the heart of the final container holding accumulated mode-wise vehicle counts */
	typedef std::map<std::string, VehicleCount> CountMap;

	/** map of road segment id --> CountMap */
	typedef std::map<unsigned int, CountMap> RoadSegmentCountMap;

	/**
	 * final container for collecting in simulation data:
	 * map[time interval][road segment id][travel mode]-->[number-of-vehicles]
	 */
	typedef std::map<TimeInterval, RoadSegmentCountMap> ScreenLineCountCollector;

    ScreenLineCounter();
    virtual ~ScreenLineCounter();

    /**
     * Loads the screenline segments from database
     */
    void loadScreenLines();

    /**
     * Get the Time Interval based on user configuration
     */
    unsigned int getTimeInterval(const double time) const;

    /**
     * container to store road segment travel times at different time intervals
     */
    ScreenLineCountCollector screenlineMap;

    /**
     * List of Screen Lines segment ids
     */
    std::set<unsigned int> screenLineSegments;

    static ScreenLineCounter* instance;
    boost::mutex instanceMutex;
};

}
}
