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

enum Location_Type{
	building, node, link, publicTansitStop
};

enum TripChainItemType {
	trip, activity
};

/**
 * Base class for elements in a trip chain.
 * \author Harish L
 */
class TripChainItem {

public:
	TripChainItemType itemType;
	sim_mob::DailyTime startTime;
	int entityID;

	unsigned int getSequenceNumber() const {
		return sequenceNumber;
	}

	void setSequenceNumber(unsigned int sequenceNumber) {
		this->sequenceNumber = sequenceNumber;
	}
	
	static Location_Type getLocationType(std::string locType) {
		if(locType.compare("building") == 0) return building;
		else if(locType.compare("node") == 0) return node;
		else if(locType.compare("link") == 0) return link;
		else if(locType.compare("stop") == 0) return publicTansitStop;
		return node;
	}

	static TripChainItemType getItemType(std::string itemType){
		if(itemType.compare("Activity") == 0) return activity;
		else return trip;
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
	Location_Type locationType;
	bool isPrimary;
	bool isFlexible;
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;

	virtual ~Activity() {}
};

//Forward Declaration
class SubTrip;

/**
 * \author Seth N. Hetu
 * \author Harish
 */
class Trip : public sim_mob::TripChainItem
{
public:
    sim_mob::Node* fromLocation;
    Location_Type fromLocationType;
    sim_mob::Node* toLocation;
    Location_Type toLocationType;
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
