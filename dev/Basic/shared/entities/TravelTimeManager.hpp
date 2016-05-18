#pragma once
#include <boost/thread/shared_mutex.hpp>
#include <map>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <string>
#include "util/DailyTime.hpp"
#include "path/Common.hpp"

namespace sim_mob
{

/**
 * Simple helper struct to store info needed to track average travel time for a link.
 * Stores the sum of all travel times experienced by agents on a link along with the count of those agents.
 * This total travel time and count is supposed to be maintained for a defined interval of time.
 *
 * \author Harish Loganathan
 */
struct TimeAndCount
{
	/**	total travel time in seconds*/
	double totalTravelTime;

	/**	total count of contributions to travel time */
	unsigned int travelTimeCnt;

	TimeAndCount() : totalTravelTime(0.0), travelTimeCnt(0)
	{
	}

	double getTravelTime() const
	{
		if(travelTimeCnt>0)
		{
			return totalTravelTime/travelTimeCnt;
		}
		return 0.0;
	}
};

typedef std::map<const RoadSegment*, TimeAndCount> RSToTimeCountMap;
typedef std::map<std::string, RSToTimeCountMap> ModeToRSCountMap;
typedef std::map<unsigned int, ModeToRSCountMap> SegmentTravelTimeMap;

struct SegmentTravelStats
{
	const RoadSegment* roadSegment;
	double entryTime;
	double travelTime;
	bool started;
	bool finalized;
	std::string travelMode;

	SegmentTravelStats(const RoadSegment* rs = nullptr):
		roadSegment(rs), entryTime(0.0), travelTime(0.0), started(false), finalized(false), travelMode("")
	{}

	/**
	 * Records the entry time for the Roadsegment
	 *
	 * @param rs The Roadsegment which was entered
	 * @param lnkEntryTime The time of entry into the link
	 */
	void start(const RoadSegment* rs, const double segEntryTime)
	{
		if (started)
		{
			throw std::runtime_error("Starting a segment travel time which was started before");
		}
		roadSegment = rs;
		entryTime = segEntryTime;
		started = true;
	}

	/**
	 * Records the exit time for the Roadsegment
	 *
	 * @param rdSeg The road segment which the agent has exited
	 * @param rdSegExitTime The time of exit on the road segment
	 * @param travelMode_ The mode of travel being used by the agent
	 */
	void finalize(const RoadSegment* rdSeg, const double rdSegExitTime, const std::string travelMode_)
	{
		//validations
		if (!started)
		{
			throw std::runtime_error("Finalizing a segment travel time which never started");
		}
		if (finalized)
		{
			throw std::runtime_error("Finalizing a segment travel time which is already finalized");
		}
		if (rdSeg == nullptr)
		{
			throw std::runtime_error("empty road segment supplied for travel time calculations.");
		}
		if (travelMode_.empty())
		{
			throw std::runtime_error("Finalizing a wrong travel time");
		}
		if(rdSegExitTime < entryTime)
		{
			throw std::runtime_error("link exit time is before entry time");
		}

		travelTime = rdSegExitTime - entryTime;
		travelMode = travelMode_;
		finalized = true;
	}

	/**Reset the member variables to their un-initialized values*/
	void reset()
	{
		*this = SegmentTravelStats(nullptr);
	}
};

/**
 * Structure to record travel time of agents through a link
 */
struct LinkTravelStats
{
	const Link* link;
	const Link* downstreamLink;
	double entryTime;
	double travelTime;
	bool started;
	bool finalized;

	LinkTravelStats(const Link* link = nullptr) :
			link(link), downstreamLink(nullptr), entryTime(0.0), travelTime(0.0), started(false), finalized(false)
	{
	}

	/**
	 * Records the entry time for the link
	 *
	 * @param lnk The link which was entered
	 * @param lnkEntryTime The time of entry into the link
	 */
	void start(const Link* lnk, double& lnkEntryTime)
	{
		if (started)
		{
			throw std::runtime_error("Starting a travel time which was started before");
		}
		link = lnk;
		entryTime = lnkEntryTime;
		started = true;
	}

	/**
	 * Records the exit time for the link
	 *
	 * @param rdSeg The road segment which the agent has exited
	 * @param rdSegExitTime The time of exit on the road segment
	 * @param travelMode_ The mode of travel being used by the agent
	 */
	void finalize(const Link* lnk, double& lnkExitTime, const Link* nextLink)
	{
		//validations
		if (!started)
		{
			throw std::runtime_error("Finalizing a travel time which never started");
		}
		if (finalized)
		{
			throw std::runtime_error("Finalizing a travel time which is already finalized");
		}
		if (link == nullptr)
		{
			throw std::runtime_error("empty road segment supplied for travel time calculations.");
		}
		if (lnk != link)
		{
			throw std::runtime_error("Finalizing a wrong travel time");
		}
		if(lnkExitTime < entryTime)
		{
			throw std::runtime_error("link exit time is before entry time");
		}

		travelTime = lnkExitTime - entryTime;
		downstreamLink = nextLink;
		finalized = true;
	}

	/**Reset the member variables to their un-initialized values*/
	void reset()
	{
		*this = LinkTravelStats(nullptr);
	}
};

/**
 * Class to hold default, historical and in-simulation travel times for a link
 *
 * \author Harish Loganathan
 */
class LinkTravelTime
{
private:
	/**	time interval */
	typedef unsigned int TimeInterval;

	/** typedef for downstream link id -> travel time of this link; allows different travel times to be stored for this link for each immediate downstream link */
	typedef std::map<unsigned int, double> DownStreamLinkSpecificTT_Map;

	/** typedef of downstream link id -> (travel time and count) for this link; this is used for storing in-simulation link travel times*/
	typedef std::map<unsigned int, TimeAndCount> DownStreamLinkSpecificTimeAndCount_Map;

	/**	map of [time interval][downstream Link] --> [average travel time for Link] */
	typedef std::map<TimeInterval, DownStreamLinkSpecificTT_Map> TravelTimeStore;

	/** map of [time interval][downstream Link] --> [TimeAndCount for link] */
	typedef std::map<TimeInterval, DownStreamLinkSpecificTimeAndCount_Map> TimeAndCountStore;

	/** link id */
	unsigned int linkId;

	/**	travel time in seconds */
	double defaultTravelTime;

	/** time specific turning based historical TT map */
	TravelTimeStore historicalTT_Map;

	/** time specific turning based in-simulation TT map */
	TimeAndCountStore currentSimulationTT_Map;

	/** mutex for adding travel times in currentSimulationTT_Map */
	boost::shared_mutex ttMapMutex;

public:
	LinkTravelTime();
	virtual ~LinkTravelTime();

	/**
	 * custom assignment operator - becasue implicit assignment operator would not be auto-constructed for this class
	 * this would be used while loading default travel times
	 */
	LinkTravelTime& operator=(const LinkTravelTime& rhs);

	double getDefaultTravelTime() const
	{
		return defaultTravelTime;
	}

	void setDefaultTravelTime(double defaultTravelTime)
	{
		this->defaultTravelTime = defaultTravelTime;
	}

	unsigned int getLinkId() const
	{
		return linkId;
	}

	void setLinkId(unsigned int linkId)
	{
		this->linkId = linkId;
	}

	/**
	 * adds travel time of this link for specific next downstream link
	 * @param dt time of day at which the added travel time is applicable
	 * @param downstreamLinkId id of downstream link
	 * @param travelTime travel time in seconds for this link when next link is downstreamLinkId
	 *
	 * NOTE: this function assumes that the supplied downstream link is connected to current link. It does not check connectivity.
	 */
	void addHistoricalTravelTime(const DailyTime& dt, unsigned int downstreamLinkId, double travelTime);

	/**
	 * accumulates in-simulation Travel Time data
	 * @param stats travel time record
	 */
	void addInSimulationTravelTime(const LinkTravelStats& stats);

	/**
	 * fetches tt in seconds for provided downstream link and time interval index
	 * @param downstreamLinkId id of downstream link
	 * @param dt time of day for which travel time is to be fetched
	 * @return tt found from downstreamLinkTT_Map if available; -1 otherwise
	 */
	double getHistoricalLinkTT(unsigned int downstreamLinkId, const DailyTime& dt) const;
	
	/**
	 * fetches tt in seconds for provided downstream link and interval index
	 * @param downstreamLinkId id of the downstream link
	 * @param dt time of day for which travel time is to be fetched
	 * @return tt found from currentSimulationTT_Map if available; -1 otherwise
	 */
	double getInSimulationLinkTT(unsigned int downstreamLinkId, const DailyTime& dt) const;

	/**
	 * fetches tt in seconds for provided time interval index
	 * this function averages the traveltime for each downstream link and returns the average travel time
	 * @param timeInterval index of time interval for which tt is requested
	 * @return tt found from downstreamLinkTT_Map if available; -1 otherwise
	 */
	double getHistoricalLinkTT(const DailyTime& dt) const;

	/**
	 * Writes the aggregated data into the file
	 * @param fileName name of file to dump travel times
	 */
	void dumpTravelTimesToFile(const std::string fileName) const;
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
	 * loads default and historical simulation travel times for all links from database
	 */
	void loadTravelTimes();

	/**
	 * base method to get travel time of a link in a specific time of
	 * day from different sources. This method searches for link travel
	 * time in different sources in the below order:
	 * in-simulation, previous simulations, default.
	 * the method returns the first value found.
	 * @param lnk input Link
	 * @param startTime start of the time range
	 * @param downstreamLink the next link which is to be taken after lnk
	 * @param useInSimulationTT indicates whether in simulation travel times are to be used
	 * @return travel time in seconds
	 */
	double getLinkTT(const sim_mob::Link* lnk, const sim_mob::DailyTime& startTime, const sim_mob::Link* downstreamLink = NULL,
					bool useInSimulationTT = false) const;

	/**
	 * fetches the default travel time for link
	 */
	double getDefaultLinkTT(const Link* lnk) const;

	/**
	 * returns the travel time experienced by other drivers in the current simulation
	 * @param mode mode of travel requested
	 * @param rs target road segment
	 * @return the travel time
	 */
	double getInSimulationLinkTT(const sim_mob::Link *lnk) const;

	/**
	 * simulation time interval in milliseconds
	 */
	unsigned int intervalMS;

	/**
	 * accumulated segment travel time data
	 * @param stats segment travel time record
	 */
	void addSegmentTravelTime(const SegmentTravelStats& stats);

	void dumpSegmentTravelTimeToFile(const std::string& fileName) const;


	/**
	 * accumulates Travel Time data
	 * @param stats travel time record
	 */
	void addTravelTime(const LinkTravelStats& stats);

	/**
	 * Writes the aggregated data into the file
	 * @param fileName name of file to dump travel times
	 */
	void dumpTravelTimesToFile(const std::string fileName) const;

	/**
	 * save Realtime Travel Time into Database
	 */
	bool storeCurrentSimulationTT();

	/**
	 * accumulated OD travel time data
	 * @param odPair Origin-Destination pair
	 * @param travelTime travel time
	 */
	void addODTravelTime(const std::pair<unsigned int, unsigned int>& odPair,
			const uint32_t startTime, const uint32_t travelTime);

	void dumpODTravelTimeToFile(const std::string& fileName) const;

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
		double getInSimulationLinkTT(const sim_mob::Link* lnk) const;
	};

	/**
	 * instance of EnRouteTT helper class
	 */
	EnRouteTT* enRouteTT;

private:
	TravelTimeManager();
	~TravelTimeManager();

	/**
	 * loads default travel times for all links from database into lnkTravelTimeMap
	 * @param sql soci::session object for db connection
	 */
	void loadLinkDefaultTravelTime(soci::session& sql);

	/**
	 * loads historical simulation travel times for all links from database into lnkTravelTimeMap
	 * @param sql soci::session object for db connection
	 */
	void loadLinkHistoricalTravelTime(soci::session& sql);

	unsigned int getODInterval(const unsigned int time);

	unsigned int getSegmentInterval(const unsigned int time);

	/**
	 * OD Travel Time interval in milliseconds
	 */
	unsigned int odIntervalMS;

	/**
	 * Segment travel time interval in milliseconds
	 */
	unsigned int segIntervalMS;

	/**
	 * a structure to keep history of average segment travel time records
	 */
	SegmentTravelTimeMap segmentTravelTimeMap;

	/**
	 * a structure to keep history of average travel time records
	 */
	std::map<unsigned int, sim_mob::LinkTravelTime> lnkTravelTimeMap;

	/**
	 * a structure to keep history of average od travel time records
	 */
	std::map<unsigned int, std::map<std::pair<unsigned int, unsigned int>, TimeAndCount> > odTravelTimeMap;

	/** historical travel time table name */
	std::string historicalTT_TableName;

	/** default travel time table name */
	std::string defaultTT_TableName;

	static sim_mob::TravelTimeManager* instance;
};
}//namespace
