#pragma once

#include <boost/noncopyable.hpp>

#include "MT_Config.hpp"

namespace sim_mob
{

namespace medium
{

class ExpandMidTermConfigFile: public boost::noncopyable
{
public:
	ExpandMidTermConfigFile(MT_Config& mtCfg, ConfigParams& cfg, std::set<sim_mob::Entity*>& active_agents);

private:
	void processConfig();

	//These functions are called by ProcessConfig()
	void checkGranularities();
	void setTicks();
	bool setTickFromBaseGran(unsigned int& res, unsigned int tickLenMs);

	void printSettings();

	void loadNetworkFromDatabase();

	void loadPublicTransitNetworkFromDatabase();

	void WarnMidroadSidewalks();

	void verifyIncidents();

	void setRestrictedRegionSupport();

	/**
	 * constructs confluxes around each multinode
	 * @param rdnw the road network
	 */
	static void ProcessConfluxes(const RoadNetwork& rdnw);

	/**
	 * creates a list of SegmentStats for a given segment depending on the stops
	 * in the segment. The list splitSegmentStats will contain SegmentStats objects
	 * containing bus stops (and quite possibly a last SegmentStats with no bus stop)
	 * @param rdSeg the road segment for which stats must be created
	 * @param splitSegmentStats vector of SegmentStats* to be filled up
	 */
	static void CreateSegmentStats(const RoadSegment* rdSeg, Conflux* conflux, std::list<SegmentStats*>& splitSegmentStats);

	/**
	 * Creates lane groups for every SegmentStats in each link.
	 * Lane groups are elicited based on the lane connections (turnings) of the last segment of the link.
	 */
	static void CreateLaneGroups();


	MT_Config& mtCfg;

	//The config file we are currently loading
	ConfigParams& cfg;

	//Our active/pending agent lists.
	std::set<Entity*>& active_agents;
};
} // namespace medium
} // namespace sim_mob
