#pragma once

#include <set>
#include <boost/thread/mutex.hpp>

namespace sim_mob
{

/**
 * Travel time manager for edges in PT graph
 *
 * \author Zhang Huai Peng
 */
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
    void updateEdgeTravelTime(const unsigned int edgeId,const unsigned int startTime,const unsigned int endTime,const std::string& travelMode);
    /**
     * Export the edge travel time to a file.
     */
    void exportEdgeTravelTime() const;
    /**
     * load pt edge travel time from DB
     */
    void loadPT_EdgeTravelTime();
    /**
     * get pt edge travel time
     * @param edgeId is edge id
     * @param currentTime is current time
     * @param waitTime is waiting time for this edge
     * @param walkTime is walking time for this edge
     * @param dayTransitTime is day transit time for this edge
     * @param linkTravelTime is link travel time for this edge
     * @return true if finding corresponding record
     */
    bool getEdgeTravelTime(const unsigned int edgeId, unsigned int currentTime, double& waitTime, double& walkTime,
			double& dayTransitTime, double& linkTravelTime);

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
        double countforLinkTime;
        double countforWaitTime;
        EdgeTimeSlot() : edgeId(0), timeInterval(0),waitTime(0.0), walkTime(0.0),
        		dayTransitTime(0.0), linkTravelTime(0.0), countforLinkTime(0.0), countforWaitTime(0.0){}
    };
    typedef std::map<unsigned int, EdgeTimeSlot> EdgeTimeSlotMap;
    /**store the all travel time on the edge*/
    std::map<int, EdgeTimeSlotMap> storeEdgeTimes;
    /**load the all travel time on the edge*/
    std::map<int, EdgeTimeSlotMap> loadEdgeTimes;
private:
	/**
	 * add the travel time of vehicles passing through a PT edge
	 *
	 * @param edgeId is edge id
	 * @param startTime time of entry into edge
	 * @param endTime time of exit from edge
	 * @param waitTime waiting time for the edge
	 * @param walkTime walking time for the edge
	 * @param dayTransitTime transit time for the edge
	 * @param linkTravelTime link travel time for the edge
	 */
	void loadOneEdgeTravelTime(const unsigned int edgeId,
			const std::string& startTime, const std::string endTime,
			const double waitTime, const double walkTime,
			const double dayTransitTime, const double linkTravelTime);

};

}

