#pragma once
#include "Common.hpp"
#include "entities/Agent.hpp"

namespace sim_mob
{
class PathSetManager;
/**
 * ProcessTT is a small helper class to process Real Time Travel Time at RoadSegment Level.
 * PathSetManager receives Real Time Travel Time and delegates
 * the processing task to this class.
 * This class aggregates the data received within different
 * time ranges and writes them to a file.
 */
class ProcessTT
{
	/**
	 *	container to stor road segment travel times at different time intervals
	 */

	sim_mob::TravelTime rdSegTravelTimesMap;
public:
	/**
	 * time interval value used for processing data.
	 * This value is based on its counterpart in pathset manager.
	 */

	unsigned int &intervalMS;

	/**
	* current time interval, with respect to simulation time
	* this is used to avoid continuous calculation of the current
	* time interval.
	* Note: Updating this happens once in one of the barriers, currently
	* Aura Manager barrier(void sim_mob::WorkGroupManager::waitAllGroups_AuraManager())
	*/
	unsigned int &curIntervalMS;
	static int dbg_ProcessTT_cnt;
	static void initTimeInterval();
	static void updateCurrTimeInterval();
	ProcessTT(unsigned int &intervalMS, unsigned int &curIntervalMS);
	~ProcessTT();
	/*
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
	double getTT(const std::string mode,const  sim_mob::RoadSegment *rs) const;
	friend class sim_mob::PathSetManager;

	/**
	 * a helper class that maintains the latest processed travel time information.
	 */
	class EnRouteTT
	 {
	 protected:
//	 	ProcessTT &parent;
	 	sim_mob::TravelTime &rdSegTravelTimesMap;
	 public:
	 	EnRouteTT(ProcessTT &parent):/*parent(parent),*/rdSegTravelTimesMap(parent.rdSegTravelTimesMap){}
	 	/**
	 	 * get the desired travel time based on the implementation
	 	 * @param rs the roadsegment for which TT is retrieved
	 	 */
	 	virtual double getTT(const std::string mode, const sim_mob::RoadSegment *rs)const  = 0;
	 	/**
	 	 * do the implementation-specific internal updates based on the information given
	 	 * @param mode travel mode
	 	 * @param  timeInterval the time interval for which the travel time has been produced.
	 	 * @param rs the road segment for which the travel time has been produced.
	 	 * @param ttInfo actual travel time information
	 	 */
//	 	virtual void updateStats(std::string &mode, sim_mob::TT::TI timeInterval, sim_mob::RoadSegment* rs, sim_mob::TT::STC::iterator ttInfo) = 0;
	 };

	/**
	 * instance of EnRouteTT helper class
	 */
	boost::shared_ptr<EnRouteTT> enRouteTT;
};


class LastTT : public ProcessTT::EnRouteTT
{
	friend class sim_mob::ProcessTT;
private:
public:
	LastTT(ProcessTT &parent):EnRouteTT(parent){}
	/**
	 * get the travel time from latest time interval having a record
	 * for this read segment
	 */
	double getTT(const std::string mode, const sim_mob::RoadSegment *rs) const;
	virtual ~LastTT(){}
};
}//namespace
