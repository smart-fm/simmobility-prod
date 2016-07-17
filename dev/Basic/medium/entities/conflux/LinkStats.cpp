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
#include "entities/Vehicle.hpp"
#include "entities/Person_MT.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

void sim_mob::medium::LinkStats::addEntity(Person_MT* person)
{
	if(!person || !person->getRole() || !person->getRole()->getResource())
	{
		throw std::runtime_error("invalid person/role for link stats addition");
	}

	std::set<Person_MT*>::const_iterator personIt = onLinkEntities.find(person);
	if(personIt == onLinkEntities.end())
	{
		onLinkEntities.insert(person);
		++entryCount;
	}
	//else - we don't count an already counted person again
}

void sim_mob::medium::LinkStats::resetStats()
{
	carCount = 0;
	busCount = 0;
	motorcycleCount = 0;
	taxiCount = 0;
	otherVehiclesCount = 0;
	entryCount = 0;
	exitCount = 0;
	density = 0;
}

void sim_mob::medium::LinkStats::removeEntitiy(Person_MT* person)
{
	if(!person)
	{
		throw std::runtime_error("invalid person for link stats addition");
	}

	size_t eraseCount = onLinkEntities.erase(person);
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
}

sim_mob::medium::LinkStats::LinkStats(const Link* link) : linkId(link->getLinkId()), carCount(0), busCount(0), motorcycleCount(0), taxiCount(0),
		otherVehiclesCount(0), entryCount(0), exitCount(0), density(0), totalLinkLaneLength(0)
{
	const std::vector<RoadSegment*>& lnkSegments = link->getRoadSegments();
	for(const RoadSegment* seg : lnkSegments)
	{
		totalLinkLaneLength = totalLinkLaneLength + (seg->getLength() * seg->getNoOfLanes());
	}
	totalLinkLaneLength = totalLinkLaneLength / 1000.0; //convert to KM
}

std::string sim_mob::medium::LinkStats::outputLinkStats() const
{
	char buf[200];
	sprintf(buf, "%u,%.3f,%u,%u,%u,%u,%u,%u,%u",
			linkId,
			density,
			entryCount,
			exitCount,
			carCount,
			taxiCount,
			motorcycleCount,
			busCount,
			otherVehiclesCount
			);
	return std::string(buf);
}

void sim_mob::medium::LinkStats::computeLinkDensity(double vehicleLength)
{
	double totalPCUs = vehicleLength / PASSENGER_CAR_UNIT;
	density = totalPCUs / totalLinkLaneLength;
}
