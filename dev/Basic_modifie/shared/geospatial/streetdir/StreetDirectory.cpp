//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "StreetDirectory.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "A_StarShortestPathImpl.hpp"
#include "A_StarPublicTransitShortestPathImpl.hpp"
#include "A_StarShortestTravelTimePathImpl.hpp"

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
	if (!spImpl) {
		spImpl = new A_StarShortestPathImpl(network);
	}
	if (!ptImpl && ConfigManager::GetInstance().FullConfig().isPublicTransitEnabled()) {
		ptImpl = new A_StarPublicTransitShortestPathImpl(PT_NetworkCreater::getInstance().PT_NetworkEdgeMap,PT_NetworkCreater::getInstance().PT_NetworkVertexMap);
	}
	if(!sttpImpl && ConfigManager::GetInstance().FullConfig().PathSetMode()){
		sttpImpl = new A_StarShortestTravelTimePathImpl(network);
	}
}

StreetDirectory::ShortestPathImpl* StreetDirectory::getDistanceImpl() const
{
	return spImpl;
}

StreetDirectory::ShortestPathImpl* StreetDirectory::getTravelTimeImpl() const
{
	return sttpImpl;
}

StreetDirectory::PublicTransitShortestPathImpl* StreetDirectory::getPublicTransitShortestPathImpl() const
{
	return ptImpl;
}

}
