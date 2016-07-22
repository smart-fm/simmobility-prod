//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LinkStats.hpp"

#include <stddef.h>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <vector>
#include "entities/roles/Role.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "entities/Person_MT.hpp"
#include "entities/Vehicle.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
const double METERS_IN_UNIT_KM = 1000.0;
}

LinkStats::LinkStats(const Link* link) : linkId(link->getLinkId()), carCount(0), busCount(0), motorcycleCount(0), taxiCount(0),
		otherVehiclesCount(0), entryCount(0), exitCount(0), density(0), totalLinkLaneLength(0),
		linkLengthKm(link->getLength()/METERS_IN_UNIT_KM)
{
	const std::vector<RoadSegment*>& lnkSegments = link->getRoadSegments();
	for(const RoadSegment* seg : lnkSegments)
	{
		totalLinkLaneLength = totalLinkLaneLength + (seg->getLength() * seg->getNoOfLanes());
	}
	totalLinkLaneLength = totalLinkLaneLength / METERS_IN_UNIT_KM; //convert to KM
}

LinkStats::LinkStats(const LinkStats& srcStats) : linkId(srcStats.linkId), carCount(srcStats.carCount), busCount(srcStats.busCount),
		motorcycleCount(srcStats.motorcycleCount), taxiCount(srcStats.taxiCount), otherVehiclesCount(srcStats.otherVehiclesCount),
		entryCount(srcStats.entryCount), exitCount(srcStats.exitCount), density(srcStats.density),
		totalLinkLaneLength(srcStats.totalLinkLaneLength), linkLengthKm(srcStats.linkLengthKm), linkStatsMutex()
{
}

void LinkStats::resetStats()
{
	linkStatsMutex.lock();
	carCount = 0;
	busCount = 0;
	motorcycleCount = 0;
	taxiCount = 0;
	otherVehiclesCount = 0;
	entryCount = 0;
	exitCount = 0;
	density = 0;
	linkStatsMutex.unlock();
}

void LinkStats::addEntity(const Person_MT* person)
{
	if(!person || !person->getRole() || !person->getRole()->getResource())
	{
		throw std::runtime_error("invalid person/role for link stats addition");
	}

	{	//guarded code section
		linkStatsMutex.lock();
		std::set<const Person_MT*>::const_iterator personIt = linkEntities.find(person);
		if(personIt == linkEntities.end())
		{
			linkEntities.insert(person);
			++entryCount;
		}
		//else - we don't count an already counted person again
		linkStatsMutex.unlock();
	}

}

void LinkStats::removeEntitiy(const Person_MT* person)
{
	if(!person)
	{
		throw std::runtime_error("invalid person for link stats addition");
	}

	{	//guarded code section
		linkStatsMutex.lock();
		size_t eraseCount = linkEntities.erase(person);
		if(eraseCount > 0)
		{
			++exitCount;
			Vehicle::VehicleType vehicleType = person->getRole()->getResource()->getVehicleType();
			switch(vehicleType)
			{
			case Vehicle::CAR:
			{
				++carCount;
				break;
			}
			case Vehicle::BUS:
			{
				++busCount;
				break;
			}
			case Vehicle::TAXI:
			{
				++taxiCount;
				break;
			}
			case Vehicle::BIKE:
			{
				++motorcycleCount;
				break;
			}
			default:
			{
				++otherVehiclesCount;
				break;
			}
			}
		}
		else
		{
			throw std::runtime_error("attempt to remove a person who was never added from LinkStats");
		}
		linkStatsMutex.unlock();
	}
}

std::string LinkStats::writeOutLinkStats(unsigned int updateNumber)
{
	char buf[200];
	sprintf(buf, "lnk,%u,%u,%.3f,%.3f,%u,%u,%u,%u,%u,%u,%u\n",
			updateNumber,
			linkId,
			linkLengthKm,
			density,
			entryCount,
			exitCount,
			carCount,
			taxiCount,
			motorcycleCount,
			busCount,
			otherVehiclesCount
			);
	resetStats();
	return std::string(buf);
}

void LinkStats::computeLinkDensity(double vehicleLength)
{
	double totalPCUs = vehicleLength / PASSENGER_CAR_UNIT;
	{	//guarded code section
		linkStatsMutex.lock();
		density = totalPCUs / totalLinkLaneLength;
		linkStatsMutex.unlock();
	}
}
