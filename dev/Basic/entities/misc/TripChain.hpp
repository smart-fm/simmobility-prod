/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <set>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

namespace sim_mob
{

enum location_type{
	building, node, link, bus_stop
};

/**
 * Base class for elements in a trip chain.
 * \author Harish L
 */
class sim_mob::TripChainItem {
protected:
	sim_mob::Entity* parentEntity;
	unsigned int sequenceNumber;
public:
	sim_mob::DailyTime startTime;

	sim_mob::Entity* getParentEntity() const {
		return parentEntity;
	}

	void setParentEntity(sim_mob::Entity* parentEntity) {
		this->parentEntity = parentEntity;
	}

	unsigned int getSequenceNumber() const {
		return sequenceNumber;
	}

	void setSequenceNumber(unsigned int sequenceNumber) {
		this->sequenceNumber = sequenceNumber;
	}
	
};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class sim_mob::Activity : sim_mob::TripChainItem {
public:
	std::string description;
	sim_mob::Node* location;
	location_type locationType;
	bool isPrimary;
	bool isFlexible;
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;
};

/**
 * \author Harish
 */
class sim_mob::SubTrip : sim_mob::Trip {
public:
	Trip* parentTrip;
	std::string mode;
};

/**
 * \author Seth N. Hetu
 * \author Harish
 */
class sim_mob::Trip : sim_mob::TripChainItem
{
public:
    sim_mob::Node* fromLocation;
    location_type fromLocationType;
    sim_mob::Node* toLocation;
    location_type toLocationType;
    int tripID;

    std::vector<SubTrip*> getSubTrips() const
    {
        return subTrips;
    }

    void setSubTrips(std::vector<SubTrip*> subTrips)
    {
        this->subTrips = subTrips;
    }

    void addSubTrip(sim_mob::SubTrip& aSubTrip) {
    	subTrips.push_back(&aSubTrip);
    }

private:
    std::vector<SubTrip*> subTrips;
};

}
