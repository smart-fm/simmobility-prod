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

enum location_type{
	building, node, link, publicTansitStop
};

/**
 * Base class for elements in a trip chain.
 * \author Harish L
 */
class aimsun::TripChainItem {
protected:
	sim_mob::Entity* parentEntity;
	unsigned int sequenceNumber;
public:
	sim_mob::DailyTime startTime;
	int tmp_personID;
	std::string tmp_itemType;
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

	static location_type getLocationType(std::string locType) {
		if(locType.compare("building") == 0){
			return building;
		} else if(locType.compare("node") == 0){
			return node;
		} else if(locType.compare("link") == 0){
			return link;
		} else if(locType.compare("stop") == 0){
			return publicTansitStop;
		} else return "";
	}

};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class aimsun::Activity : aimsun::TripChainItem {
public:
	std::string description;
	sim_mob::Node* location;
	location_type locationType;
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
 * \author Harish
 */
class aimsun::SubTrip : aimsun::Trip {
public:
	Trip* parentTrip;
	std::string mode;
};

/**
 * \author Seth N. Hetu
 * \author Harish
 */
class aimsun::Trip : aimsun::TripChainItem
{
public:
    sim_mob::Node* fromLocation;
    location_type fromLocationType;
    sim_mob::Node* toLocation;
    location_type toLocationType;
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

}
