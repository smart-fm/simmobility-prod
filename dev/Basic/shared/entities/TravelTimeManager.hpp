#pragma once
#include <boost/thread/shared_mutex.hpp>
#include "path/Common.hpp"

namespace sim_mob
{
/**
 * Structure to record travel time of agents through a link
 */
struct LinkTravelStats
{
	const Link* link_;
	double entryTime;

	LinkTravelStats(const Link* link, unsigned int linkEntryTime)
	: link_(link), entryTime(linkEntryTime)
	{
	}
};

/**
 * Structure to record travel time of agents through a road segment
 */
struct RdSegTravelStat
{
	const RoadSegment* rs;
	double entryTime;
	double travelTime;
	std::string travelMode;
	bool started;
	bool finalized;

	RdSegTravelStat(const RoadSegment* rdSeg, std::string travelMode = "")
	: rs(rdSeg), entryTime(0.0), travelTime(0.0), started(false), finalized(false), travelMode(travelMode)
	{
	}

	/**
	 * Records the entry time for the road segment.
	 *
	 * @param rdSeg The road segment which the agent has entered
	 * @param rdSegEntryTime The time of entry on the road segment
	 */
	void start(const RoadSegment* rdSeg, double &rdSegEntryTime)
	{
		if (started)
		{
			throw std::runtime_error("Starting a travel time which was started before");
		}
		rs = rdSeg;
		entryTime = rdSegEntryTime;
		started = true;
	}

	/**
	 * Records the exit time for the road segment.
	 *
	 * @param rdSeg The road segment which the agent has exited
	 * @param rdSegExitTime The time of exit on the road segment
	 * @param travelMode_ The mode of travel being used by the agent
	 */
	void finalize(const RoadSegment* rdSeg, double &rdSegExitTime, const std::string &travelMode_)
	{
		if (!started)
		{
			throw std::runtime_error("Finalizing a travel time which never started");
		}
		if (finalized)
		{
			throw std::runtime_error("Finalizing a travel time which is already finalized");
		}
		if (rdSeg != rs)
		{
			throw std::runtime_error("Finalizing a wrong travel time");
		}
		if (rs == nullptr)
		{
			throw std::runtime_error("empty road segment supplied for travel time calculations.");
		}
		finalized = true;
		travelTime = rdSegExitTime - entryTime;
		/*
		 * we set the travel_mode in the end coz in our team, the probability of programmers "calling start() method of this structure before changing a role"
		 * is more than the probability of "calling finalize 'after' changing the role". So we set the travel mode(which is dependent on the role type) at the end.
		 * You may change it later if you wish so.
		 */
		travelMode = travelMode_;
	}

	/**Reset the member variables to their un-initialized values*/
	void reset()
	{
		*this = RdSegTravelStat(nullptr);
	}
};

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
	 * simulation time interval in milliseconds
	 */
	unsigned int intervalMS;

	/**
	 * accumulates Travel Time data
	 * @param stats travel time record
	 * @person the recording person
	 */
	void addTravelTime(const RdSegTravelStat & stats);

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
	double getInSimulationLinkTT(const std::string mode, const sim_mob::Link *lnk) const;

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
		double getInSimulationLinkTT(const std::string mode, const sim_mob::Link* lnk) const;
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
