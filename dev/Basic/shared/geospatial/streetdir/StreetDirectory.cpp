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
StreetDirectory::VertexDesc StreetDirectory::DrivingVertex(const Node& node) const
{
	if (!spImpl)
	{
		return StreetDirectory::VertexDesc(false);
	}
	return spImpl->DrivingVertex(node);
}

StreetDirectory::VertexDesc StreetDirectory::DrivingTimeVertex(const Node& node, TimeRange timeRange, int randomGraphId) const
{
	if (!sttpImpl)
	{
		return StreetDirectory::VertexDesc(false);
	}

//	if (timeRange == MorningPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexMorningPeak(node);
//	}
//	else if (timeRange == EveningPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexEveningPeak(node);
//	}
//	else if (timeRange == OffPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexNormalTime(node);
//	}
//	else
	if (timeRange == Default)
	{
		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexDefault(node);
	}
	else if (timeRange == HighwayBiasDistance)
	{
		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexHighwayBiasDistance(node);
	}
//	else if (timeRange == HighwayBiasMorningPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexHighwayBiasMorningPeak(node);
//	}
//	else if (timeRange == HighwayBiasEveningPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexHighwayBiasEveningPeak(node);
//	}
//	else if (timeRange == HighwayBiasOffPeak)
//	{
//		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexHighwayBiasNormalTIme(node);
//	}
	else if (timeRange == HighwayBiasDefault)
	{
		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexHighwayBiasDefault(node);
	}
	else if (timeRange == Random)
	{
		return ((A_StarShortestTravelTimePathImpl *) sttpImpl)->DrivingVertexRandom(node, randomGraphId);
	}

	return StreetDirectory::VertexDesc(false);
}

std::vector<sim_mob::WayPoint> StreetDirectory::SearchShortestDrivingTimePath(const sim_mob::Node& from,
		const sim_mob::Node& to, const std::vector<const sim_mob::Link*>& blacklist,TimeRange timeRange,unsigned int randomGraphIdx) const
{
	if (!sttpImpl) {
		return std::vector<sim_mob::WayPoint>();
	}
	VertexDesc source = DrivingTimeVertex(from, timeRange, randomGraphIdx);
	VertexDesc sink = DrivingTimeVertex(to, timeRange, randomGraphIdx);
	std::vector<sim_mob::WayPoint> res =
			((A_StarShortestTravelTimePathImpl *) sttpImpl)->GetShortestDrivingPath(source, sink, blacklist, timeRange, randomGraphIdx);
	return res;
}

std::vector<WayPoint> StreetDirectory::SearchShortestDrivingPath(const Node &from, const Node &to, const std::vector<const Link*>& blackList ) const
{
	std::vector<WayPoint> res;
	if (!spImpl)
	{
		return res;
	}
	VertexDesc source = DrivingVertex(from);
	VertexDesc sink = DrivingVertex(to);
	res = spImpl->GetShortestDrivingPath(source, sink, blackList);
	return res;
}
}
