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

//Forward declarations
class Node;
class SubTrip;

/**
 * Base class for elements in a trip chain.
 * \author Harish L
 */
class TripChainItem {
public:
	//Type of location of this trip chain item.
	enum LocationType{
		LT_BUILDING, LT_NODE, LT_LINK, LT_PUBLIC_TRANSIT_STOP
	};

	//Type of this trip chain item.
	enum ItemType {
		IT_TRIP, IT_ACTIVITY
	};

	ItemType itemType;
	sim_mob::DailyTime startTime;
	int entityID;

	unsigned int getSequenceNumber() const {
		return sequenceNumber;
	}

	void setSequenceNumber(unsigned int sequenceNumber) {
		this->sequenceNumber = sequenceNumber;
	}
	
	static LocationType getLocationType(std::string locType);

	static ItemType getItemType(std::string itemType){
		if(itemType == "Activity") {
			return IT_ACTIVITY;
		} else if(itemType == "Trip") {
			return IT_TRIP;
		} else {
			throw std::runtime_error("Unknown trip chain item type.");
		}
	}

    virtual ~TripChainItem() {}

protected:
	// sim_mob::Entity* parentEntity; // Keeping only ID for now. Entity objects will have to be created when Person table has data.//sim_mob::Entity* parentEntity;
	unsigned int sequenceNumber;

};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class Activity : public sim_mob::TripChainItem {
public:
	std::string description;
	sim_mob::Node* location;
	TripChainItem::LocationType locationType;
	bool isPrimary;
	bool isFlexible;

	sim_mob::DailyTime activityEndTime;

	sim_mob::DailyTime& activityStartTime() {
		return startTime;
	}

	virtual ~Activity() {}
};


/**
 * \author Seth N. Hetu
 * \author Harish
 */
class Trip : public sim_mob::TripChainItem
{
public:
    sim_mob::Node* fromLocation;
    TripChainItem::LocationType fromLocationType;
    sim_mob::Node* toLocation;
    TripChainItem::LocationType toLocationType;
    int tripID;

    std::vector<SubTrip*> getSubTrips() const
    {
        return subTrips;
    }

    void setSubTrips(const std::vector<SubTrip*>& subTrips)
    {
        this->subTrips = subTrips;
    }

    void addSubTrip(sim_mob::SubTrip* aSubTrip) {
    	subTrips.push_back(aSubTrip);
    }

    virtual ~Trip() {}

private:
    std::vector<SubTrip*> subTrips;
};

/**
 * \author Harish
 */
class SubTrip : public sim_mob::Trip {
public:
	sim_mob::Trip* parentTrip;
	std::string mode;

	virtual ~SubTrip() {}
};

}
