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

namespace aimsun
{
//Forward Declaration
class sim_mob::aimsun::Node;
class SubTrip;

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
protected:
	// sim_mob::Entity* parentEntity; // Keeping only ID for now. Entity objects will have to be created when Person table has data.
	unsigned int sequenceNumber;
public:
	sim_mob::TripChainItemType itemType;
	sim_mob::DailyTime startTime;
	int entityID;

	std::string tmp_startTime;

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
};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class Activity : public aimsun::TripChainItem {
public:
	std::string description;
	sim_mob::aimsun::Node* location;
	sim_mob::Location_Type locationType;
	bool isPrimary;
	bool isFlexible;
	sim_mob::DailyTime activityStartTime;
	sim_mob::DailyTime activityEndTime;

	int tmp_activityID;
	int tmp_locationID;
	std::string  tmp_locationType;
	std::string tmp_activityStartTime;
	std::string tmp_activityEndTime;

};

/**
 * \author Seth N. Hetu
 * \author Harish
 */
class Trip : public aimsun::TripChainItem
{
public:
	sim_mob::aimsun::Node* fromLocation;
	sim_mob::Location_Type fromLocationType;
    sim_mob::aimsun::Node* toLocation;
    sim_mob::Location_Type toLocationType;
    int tripID;

    //Temporaries for SOCI conversion
	int tmp_subTripID;
	int tmp_fromLocationNodeID;
	std::string tmp_fromlocationType;
	int  tmp_toLocationNodeID;
	std::string tmp_tolocationType;
	std::string tmp_startTime;


    std::vector<SubTrip*> getSubTrips() const {
        return subTrips;
    }

    void setSubTrips(std::vector<SubTrip*> subTrips) {
        this->subTrips = subTrips;
    }

private:
    std::vector<SubTrip*> subTrips;
};

/**
 * \author Harish
 */
class SubTrip : public aimsun::Trip {
public:
	Trip* parentTrip;
	std::string mode;
};

}
