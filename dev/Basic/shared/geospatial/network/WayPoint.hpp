//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "Link.hpp"
#include "Node.hpp"
#include "RoadSegment.hpp"
#include "TurningGroup.hpp"
#include "PT_Stop.hpp"
#include "Platform.hpp"

namespace sim_mob
{

struct WayPoint
{
	enum
	{
		/**The way point is invalid. None of the pointers are valid*/
		INVALID,

		/**The way point is a node. node points to a Node object*/
		NODE,

		/**The way point is a lane. lane points to a Lane object*/
		LANE,

		/**The way point is a road-segment. roadSegment points to a RoadSegment object*/
		ROAD_SEGMENT,

		/**The way point is a link. link points to a Link object*/
		LINK,

		/**The way point is a turning path. turningPath points to a TurningPath object*/
		TURNING_PATH,

		/**The way point is a turning group. turningGroup points to a TurningGroup object*/
		TURNING_GROUP,

		/**The way point is a platform, platform points to a Platform object*/
		MRT_PLATFORM,

		/**The way point is a bus stop. busStop points to a BusStop object*/
		BUS_STOP,

		/**The way point is an MRT stop. trainStop points to an TrainStop object*/
		TRAIN_STOP
	} type;

	union
	{
		const Node *node;
		const Lane *lane;
		const RoadSegment *roadSegment;
		const Link *link;
		const TurningPath *turningPath;
		const TurningGroup *turningGroup;
		const Platform* platform;
		const BusStop *busStop;
		const TrainStop *trainStop;
	};

	WayPoint() :
	type(INVALID), node(NULL)
	{
	}

	explicit WayPoint(const Node *node) :
	type(NODE), node(node)
	{
	}

	explicit WayPoint(const Lane *lane) :
	type(LANE), lane(lane)
	{
	}

	explicit WayPoint(const RoadSegment *segment) :
	type(ROAD_SEGMENT), roadSegment(segment)
	{
	}

	explicit WayPoint(const Link *link) :
	type(LINK), link(link)
	{
	}

	explicit WayPoint(const TurningPath *turningPath) :
	type(TURNING_PATH), turningPath(turningPath)
	{
	}

	explicit WayPoint(const TurningGroup *turningGroup) :
	type(TURNING_GROUP), turningGroup(turningGroup)
	{
	}

	explicit WayPoint(const Platform* platform) :
	type(MRT_PLATFORM), platform(platform)
	{
	}

	explicit WayPoint(const BusStop *busStop) :
	type(BUS_STOP), busStop(busStop)
	{
	}

	explicit WayPoint(const TrainStop *trainStop) :
	type(TRAIN_STOP), trainStop(trainStop)
	{
	}

	WayPoint(const WayPoint& orig)
	{
		type = orig.type;

		switch (type)
		{
		case INVALID:
			roadSegment = NULL;
			break;

		case NODE:
			node = orig.node;
			break;

		case LANE:
			lane = orig.lane;
			break;

		case ROAD_SEGMENT:
			roadSegment = orig.roadSegment;
			break;

		case LINK:
			link = orig.link;
			break;

		case TURNING_PATH:
			turningPath = orig.turningPath;
			break;

		case TURNING_GROUP:
			turningGroup = orig.turningGroup;
			break;

		case BUS_STOP:
			busStop = orig.busStop;
			break;

		case MRT_PLATFORM:
			platform = orig.platform;
			break;

		case TRAIN_STOP:
			trainStop = orig.trainStop;
			break;
		}
	}

	bool operator==(const WayPoint& rhs) const
	{
		return (type == rhs.type &&
				node == rhs.node &&
				lane == rhs.lane &&
				roadSegment == rhs.roadSegment &&
				link == rhs.link &&
				turningPath == rhs.turningPath &&
				turningGroup == rhs.turningGroup &&
				busStop == rhs.busStop &&
				trainStop == rhs.trainStop &&
				platform == rhs.platform);
	}

	bool operator!=(const WayPoint& rhs) const
	{
		return !(*this == rhs);
	}

	WayPoint& operator=(const WayPoint& rhs)
	{
		type = rhs.type;

		switch (type)
		{
		case INVALID:
			node = NULL;
			roadSegment = NULL;
			break;

		case NODE:
			node = rhs.node;
			break;

		case LANE:
			lane = rhs.lane;
			break;

		case ROAD_SEGMENT:
			roadSegment = rhs.roadSegment;
			break;

		case LINK:
			link = rhs.link;
			break;

		case TURNING_PATH:
			turningPath = rhs.turningPath;
			break;

		case TURNING_GROUP:
			turningGroup = rhs.turningGroup;
			break;

		case BUS_STOP:
			busStop = rhs.busStop;
			break;

		case TRAIN_STOP:
			trainStop = rhs.trainStop;
			break;

		case MRT_PLATFORM:
			platform = rhs.platform;
			break;
		}

		return *this;
	}

};
}

