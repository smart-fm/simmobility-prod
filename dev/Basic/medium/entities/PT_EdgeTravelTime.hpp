/*
 * PT_EdgeTravelTime.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: zhang huai peng
 */

#pragma once

#include <set>
#include <boost/thread/mutex.hpp>

namespace sim_mob
{
namespace medium
{

class PT_EdgeTravelTime {
public:
	PT_EdgeTravelTime();
	virtual ~PT_EdgeTravelTime();

public:
	/**
	 * return global instance to help access this object
	 * @return the unique instance of this class
	 */
    static PT_EdgeTravelTime* getInstance();

    /**
     * Update the travel time of vehicles passing through a PT edge
     *
     * @param edgeId is edge id
     * @param startTime time of entry into edge
     * @param endTime time of exit from edge
     * @param travelMode travel mode
     */
    void updateEdgeTravelTime(const unsigned int edgeId,const unsigned int startTime,const unsigned int endTime,const double waitTime,const std::string& travelMode);

    /**
     * Export the edge travel time to a file.
     */
    void exportEdgeTravelTime() const;

private:
    /**global instance to store the pointer to this class*/
    static PT_EdgeTravelTime* instance;
    /**locker for this global instance*/
    boost::mutex instanceMutex;
    /**
     * structure to store edge time
     */
    struct EdgeTimeSlot{
    	unsigned int edgeId;
    	unsigned int timeInterval;
        double waitTime;
        double walkTime;
        double dayTransitTime;
        double linkTravelTime;
        double count;
        EdgeTimeSlot() : edgeId(0), timeInterval(0),waitTime(0.0), walkTime(0.0),
        		dayTransitTime(0.0), linkTravelTime(0.0), count(0.0){}
    };
    typedef std::map<unsigned int, EdgeTimeSlot> EdgeTimeSlotMap;
    /**store the all travel time on the edge*/
    std::map<int, EdgeTimeSlotMap> edgeTimes;
};

}

}

