//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PersonLoader.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <map>
#include <sstream>
#include <stdint.h>
#include <utility>
#include <vector>
#include "soci/soci.h"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "logging/Log.hpp"
#include "misc/TripChain.hpp"
#include "Person.hpp"
#include "util/DailyTime.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;

sim_mob::PeriodicPersonLoader::PeriodicPersonLoader(std::set<sim_mob::Entity*>& activeAgents, StartTimePriorityQueue& pendinAgents) :
		activeAgents(activeAgents), pendingAgents(pendinAgents), dataLoadInterval(0), nextLoadStart(0.0), elapsedTimeSinceLastLoad(0),
		storedProcName(std::string())
{
}

sim_mob::PeriodicPersonLoader::~PeriodicPersonLoader()
{
}

void sim_mob::PeriodicPersonLoader::addOrStashPerson(Person* p)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (p->getStartTime()==0)
	{
		p->load(p->getConfigProperties());
		p->clearConfigProperties();
		activeAgents.insert(p);
	}
	else
	{
		//Start later.
		pendingAgents.push(p);
	}
}

bool sim_mob::PeriodicPersonLoader::checkTimeForNextLoad()
{
	elapsedTimeSinceLastLoad += ConfigManager::GetInstance().FullConfig().baseGranSecond();
	if(elapsedTimeSinceLastLoad >= dataLoadInterval)
	{
		elapsedTimeSinceLastLoad = 0;
		return true;
	}
	return false;
}


boost::shared_ptr<sim_mob::RestrictedRegion> sim_mob::RestrictedRegion::instance;
sim_mob::RestrictedRegion::RestrictedRegion() : Impl(new TagSearch(*this)),zoneSegmentsStr(""),zoneNodesStr(""),inStr(""), outStr("")
{}

sim_mob::RestrictedRegion::~RestrictedRegion()
{
	safe_delete_item(Impl);
}

void sim_mob::RestrictedRegion::populate()
{
	if(!populated.check()) { return; } //skip if already populated

	sim_mob::aimsun::Loader::getCBD_Border(in,out);
	sim_mob::aimsun::Loader::getCBD_Segments(zoneSegments);

	sim_mob::aimsun::Loader::getCBD_Nodes(zoneNodes);

	typedef std::map<unsigned int, const Node*>::value_type Pair;

	//String representations & Tagging
	std::stringstream outStrm("");
	BOOST_FOREACH(const sim_mob::RoadSegment*rs,zoneSegments)
	{
		outStrm << rs->getId() << ",";
		rs->CBD = true;
	}
	zoneSegmentsStr = outStrm.str();

	outStrm.str(std::string());
	BOOST_FOREACH(Pair node, zoneNodes)
	{
		outStrm << node.first << ",";
		node.second->CBD = true;
	}
	zoneNodesStr = outStrm.str();

	outStrm.str(std::string());
	BOOST_FOREACH(SegPair item, in)
	{
		outStrm << item.first->getId() << ":" << item.second->getId() << ",";
	}
	inStr = outStrm.str();

	outStrm.str(std::string());
	BOOST_FOREACH(SegPair item, out)
	{
		outStrm << item.first->getId() << ":" << item.second->getId() << ",";
	}
	outStr = outStrm.str();
}

bool sim_mob::RestrictedRegion::isInRestrictedZone(const sim_mob::Node* target) const
{
	return Impl->isInRestrictedZone(target);
}

bool sim_mob::RestrictedRegion::isInRestrictedZone(const sim_mob::WayPoint& target) const
{
	switch(target.type_)
	{
		case WayPoint::NODE:
		{
			return isInRestrictedZone(target.node_);
		}
		case WayPoint::ROAD_SEGMENT:
		{
			return isInRestrictedSegmentZone(target.roadSegment_);
		}
		default:
		{
			throw std::runtime_error("Invalid Waypoint type supplied\n");
		}
	}
}

bool sim_mob::RestrictedRegion::isInRestrictedZone(const std::vector<WayPoint>& target) const
{
	BOOST_FOREACH(WayPoint wp, target)
	{
		if(isInRestrictedZone(wp))
		{
			return true;
		}
	}
	return false;
}

bool sim_mob::RestrictedRegion::isInRestrictedSegmentZone(const std::vector<WayPoint> & target) const
{
	BOOST_FOREACH(WayPoint wp, target)
	{
		if(wp.type_ != WayPoint::ROAD_SEGMENT)
		{
			throw std::runtime_error("Invalid WayPoint type Supplied\n");
		}
		if(Impl->isInRestrictedSegmentZone(wp.roadSegment_))
		{
			return true;
		}
	}
	return false;
}

bool sim_mob::RestrictedRegion::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return Impl->isInRestrictedSegmentZone(target);
}

bool sim_mob::RestrictedRegion::isEnteringRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg)
{
	return in.find(std::make_pair(curSeg,nxtSeg)) != in.end();
}

bool sim_mob::RestrictedRegion::isExitingRestrictedZone(const sim_mob::RoadSegment* curSeg ,const sim_mob::RoadSegment* nxtSeg)
{
	return out.find(std::make_pair(curSeg,nxtSeg)) != out.end();
}

//Search implementation
sim_mob::RestrictedRegion::StrSearch::StrSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedZone(const Node* target) const
{
	return (instance.zoneNodesStr.find(boost::lexical_cast<string>(target->getID())) != std::string::npos ? true : false);
}

bool sim_mob::RestrictedRegion::StrSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return (instance.zoneSegmentsStr.find(boost::lexical_cast<string>(target->getId())) != std::string::npos ? true : false);
}

sim_mob::RestrictedRegion::ObjSearch::ObjSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::ObjSearch::isInRestrictedZone(const Node* target) const
{
	std::map<unsigned int, const Node*>::const_iterator it(instance.zoneNodes.find(target->getID()));
	return (instance.zoneNodes.end() == it ? false : true);
}

bool sim_mob::RestrictedRegion::ObjSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	std::set<const sim_mob::RoadSegment*>::iterator itDbg;
	if ((itDbg = instance.zoneSegments.find(target)) != instance.zoneSegments.end())
	{
		return true;
	}
	return false;
}

sim_mob::RestrictedRegion::TagSearch::TagSearch(sim_mob::RestrictedRegion & instance):Search(instance){}
bool sim_mob::RestrictedRegion::TagSearch::isInRestrictedZone(const Node* target) const
{
	return target->CBD;
}

bool sim_mob::RestrictedRegion::TagSearch::isInRestrictedSegmentZone(const sim_mob::RoadSegment * target) const
{
	return target->CBD;
}


