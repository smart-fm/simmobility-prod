//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "StreetDirectory.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/BusStop.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "A_StarShortestPathImpl.hpp"

namespace sim_mob
{

StreetDirectory StreetDirectory::instance;

StreetDirectory::StreetDirectory() : spImpl(nullptr), sttpImpl(nullptr), ptImpl(nullptr)
{
}

StreetDirectory::~StreetDirectory()
{
	if (spImpl)
	{
		delete spImpl;
	}
	if (sttpImpl)
	{
		delete sttpImpl;
	}
	if (ptImpl)
	{
		delete ptImpl;
	}
}

void StreetDirectory::Init(const RoadNetwork& network)
{
	if (!spImpl)
	{
		spImpl = new A_StarShortestPathImpl(network);
	}
}

StreetDirectory::ShortestPathImpl* StreetDirectory::getDistanceImpl()
{
	return spImpl;
}

StreetDirectory::ShortestPathImpl* StreetDirectory::getTravelTimeImpl()
{
	return sttpImpl;
}

StreetDirectory::PublicTransitShortestPathImpl* StreetDirectory::getPublicTransitShortestPathImpl()
{
	return ptImpl;
}

StreetDirectory::VertexDesc StreetDirectory::DrivingTimeVertex(const Node& node, TimeRange timeRange,int randomGraphId) const
{
}

}
