#pragma once
#include "Common.hpp"
#include "entities/Agent.hpp"

namespace sim_mob
{
class PathSetManager;
/**
 * TravelTimeManager is a small helper class to process Real Time Travel Time at RoadSegment Level.
 * PathSetManager receives Real Time Travel Time and delegates
 * the processing task to this class.
 * This class aggregates the data received within different
 * time ranges and writes them to a file.
 */
class TravelTimeManager
{
public:
	/**
	 * gets the singleton instance of TravelTimeManager
	 */
	static sim_mob::TravelTimeManager* getInstance();

	/**
	 *	container to stor road segment travel times at different time intervals
	 */
	sim_mob::TravelTime ttMap;
	boost::shared_mutex ttMapMutex;

	/**
	 * time interval value used for processing data.
	 * This value is based on its counterpart in pathset manager.
	 */
	unsigned int intervalMS;

	/**
	 * accumulates Travel Time data
	 * @param stats travel time record
	 * @person the recording person
	 */
	void addTravelTime(const Agent::RdSegTravelStat & stats);

	/**
	 * Writes the aggregated data into the file
	 */
	void insertTravelTime2TmpTable(const std::string fileName);

	/**
	 * save Realtime Travel Time into Database
	 */
	bool storeRTT2DB();

	/**
	 * get corresponding TI (Time interval) give a time of day
	 * @param time a time within the day (usually segment entry time)
	 * @param interval travel time intreval specified for this simulation
	 * @return Time interval corresponding the give time
	 * Note: for uniformity purposes this methods works with milliseconds values
	 */
	static sim_mob::TT::TI getTimeInterval(const unsigned long timeMS, const unsigned int intervalMS);

	/**
	 * returns the travel time experienced by other drivers in the current simulation
	 * @param mode mode of travel requested
	 * @param rs target road segment
	 * @return the travel time
	 */
	double getInSimulationSegTT(const std::string mode,const  sim_mob::RoadSegment *rs) const;

	/**
	 * a helper class that maintains the latest processed travel time information.
	 */
	class EnRouteTT
	{
	private:
		TravelTimeManager &parent;

	public:
		EnRouteTT(TravelTimeManager &parent) : parent(parent) {}
		~EnRouteTT() {}

		/**
		 * get the desired travel time
		 * @param mode	travel mode
		 * @param rs	the roadsegment for which TT is retrieved
		 */
		double getInSimulationSegTT(const std::string mode, const sim_mob::RoadSegment* rs) const;
	};

	/**
	 * instance of EnRouteTT helper class
	 */
	EnRouteTT* enRouteTT;

private:
	TravelTimeManager();
	~TravelTimeManager();

	static sim_mob::TravelTimeManager* instance;
};
}//namespace
