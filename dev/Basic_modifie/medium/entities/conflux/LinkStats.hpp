#pragma once

#include <mutex>
#include <set>
#include <string>

namespace sim_mob
{
class Link;
namespace medium
{
class Person_MT;

/**
 * Class to hold and output link level statistics during supply simulation.
 * Conflux class holds an instance of this class for each link that it manages.
 * NOTE: This class was written with the intention of tracking stats for every advance interval
 *       and producing outputs every update interval
 *
 * \author Harish Loganathan
 */
class LinkStats
{
private:
	/** id of link for which stats are collected */
	unsigned int linkId;

	/** link length in kilometers */
	double linkLengthKm;

	/** count of cars that exited this link */
	unsigned int carCount;
	/** count of buses that exited this link */
	unsigned int busCount;
	/** count of motor-bikes that exited this link */
	unsigned int motorcycleCount;
	/** count of taxis that exited this link */
	unsigned int taxiCount;
	/** count of other vehicle types that exited this link */
	unsigned int otherVehiclesCount;

	/** count of (all) vehicles that entered this link */
	unsigned int entryCount;
	/** count of (all) vehicles that exited this link */
	unsigned int exitCount;

	/**
	 * total length of all lanes in all segments in link in kilometers
	 * this is the sum of (segment length * number of lanes) for each road segment of this link represented by this linkStats
	 */
	double totalLinkLaneLength;

	/** instantaneous traffic density in PCUs / link-kilometer  */
	double density;

	/** set of all persons (with vehicles) in the link */
	std::set<const Person_MT*> linkEntities;

	/** instance specific mutex for LinkStats */
	std::mutex linkStatsMutex;

	/**
	 * resets all stats collected so far for this link
	 */
	void resetStats();

public:
	LinkStats(const Link* link);

	/**
	 * copy constructor is explicitly provided to exclude linkStatsMutex from the copy.
	 */
	LinkStats(const LinkStats& srcStats);

	~LinkStats() {}

	/**
	 * record addition of a person entity to link
	 * @param entity person entity who enters a link
	 */
	void addEntity(const Person_MT* entitiy);

	/**
	 * record removal of a person entity from link.
	 * vehicle type specific counts are taken here when the vehicle exits the link
	 * @param entity person entity who exits a link
	 */
	void removeEntitiy(const Person_MT* entity);

	/**
	 * constructs an output string with all stats collected so far and resets
	 * @param updateNumber update interval number at which the output was requested
	 * @return constructed csv string with all stats. csv-format: lnk,interval,lnkId,length,density,entry,exit,car,taxi,motorcycle,bus,other
	 */
	std::string writeOutLinkStats(unsigned int updateNumber);

	/**
	 * computes and sets link density
	 * @param total length of all vehicles on the link
	 */
	void computeLinkDensity(double vehicleLength);
};
} // end medium
} // end sim_mob
