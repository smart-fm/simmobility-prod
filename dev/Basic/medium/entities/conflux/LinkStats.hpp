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
 * class to hold and output link level statistics during supply simulation
 *
 * \author Harish Loganathan
 */
class LinkStats
{
private:
	unsigned int linkId;
	double linkLengthKm;
	unsigned int carCount;
	unsigned int busCount;
	unsigned int motorcycleCount;
	unsigned int taxiCount;
	unsigned int otherVehiclesCount;
	unsigned int entryCount;
	unsigned int exitCount;

	/**
	 * total length of all links in lane in kilometers
	 * this is the sum of (segment length * number of lanes) for each road segment of this link represented by this linkStats
	 */
	double totalLinkLaneLength;

	double density;

	std::set<const Person_MT*> linkEntities;

	std::mutex linkStatsMutex;

	void resetStats();

public:
	LinkStats(const Link* link);
	LinkStats(const LinkStats& srcStats);
	~LinkStats() {}

	void addEntity(const Person_MT* entitiy);

	void removeEntitiy(const Person_MT* entity);

	std::string writeOutLinkStats(unsigned int updateNumber);

	void computeLinkDensity(double vehicleLength);
};
} // end medium
} // end sim_mob
