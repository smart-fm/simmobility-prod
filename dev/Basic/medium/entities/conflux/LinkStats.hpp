#pragma once

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
	const Link* link;
	unsigned int carCount;
	unsigned int busCount;
	unsigned int motorcycleCount;
	unsigned int taxiCount;
	unsigned int totalCount;
	unsigned int entryCount;
	unsigned int exitCount;
	double density;

	std::set<Person_MT*> onLinkEntities;

public:
	LinkStats(const Link* link) : link(link), carCount(0), busCount(0), motorcycleCount(0), taxiCount(0), totalCount(0),
		entryCount(0), exitCount(0), density(0) {}

	void addEntity(Person_MT* entitiy);

	void removeEntitiy(Person_MT* entity);

	std::string outputLinkStats() const;
};
} // end medium
} // end sim_mob
