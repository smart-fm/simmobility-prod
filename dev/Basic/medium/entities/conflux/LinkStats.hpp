#pragma once

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

	std::set<Person_MT*> onLinkEntities;

	void resetStats();

public:
	LinkStats(const Link* link);

	void addEntity(Person_MT* entitiy);

	void removeEntitiy(Person_MT* entity);

	std::string outputLinkStats() const;

	void computeLinkDensity(double vehicleLength);
};
} // end medium
} // end sim_mob
