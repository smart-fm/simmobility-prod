#pragma once

#include <sstream>

#include "geospatial/network/WayPoint.hpp"
#include "geospatial/network/Node.hpp"

namespace sim_mob
{
enum PT_EdgeType
{
	UNKNOWN_EDGE,
	WALK_EDGE,
	BUS_EDGE,
	TRAIN_EDGE,
	SMS_EDGE
};
}


/**
 * A structure to store Origin and Destination in one pair
 * additional operator overload for assignment and comparisons
 */
class OD
{
private:
	std::string odIdStr;

public:
	sim_mob::WayPoint origin;
	sim_mob::WayPoint destination;

	OD(){}

	OD(const sim_mob::WayPoint &origin, const sim_mob::WayPoint &destination):
		origin(origin), destination(destination)
	{
		std::stringstream str("");
		str << origin.node->getNodeId() << "," << destination.node->getNodeId();
		odIdStr = str.str();
	}

	OD(const sim_mob::Node * origin, const sim_mob::Node * destination):
		origin(sim_mob::WayPoint(origin)), destination(sim_mob::WayPoint(destination))
	{
		std::stringstream str("");
		str << origin->getNodeId() << "," << destination->getNodeId();
		odIdStr = str.str();
	}

	const std::string& getOD_Str() const
	{
		return odIdStr;
	}

	bool operator==(const OD & rhs) const
	{
		return (origin == rhs.origin && destination == rhs.destination);
	}

	bool operator!=(const OD & rhs) const
	{
		return !(*this == rhs);
	}

	OD & operator=(const OD & rhs)
	{
		origin = rhs.origin;
		destination = rhs.destination;
		return *this;
	}

	bool operator<(const OD & rhs) const
	{
		// just an almost dummy operator< to preserve uniquness
		return odIdStr < rhs.odIdStr;
	}
};
