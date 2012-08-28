/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <map>
#include <string>
#include <set>
#include <string>

#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/Node.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif
namespace geo
{
//Forward Declaration
class Trip_t_pimpl;
}
namespace sim_mob {

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
	enum LocationType {
		LT_BUILDING, LT_NODE, LT_LINK, LT_PUBLIC_TRANSIT_STOP
	};

	//Type of this trip chain item.
	enum ItemType {
		IT_TRIP, IT_ACTIVITY
	};

	int entityID;
	ItemType itemType;
	unsigned int sequenceNumber;
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;

	//TripChainItem();
	TripChainItem(int entId=0, std::string type="Trip",
				DailyTime start=DailyTime(), DailyTime end=DailyTime(),
				unsigned int seqNumber=0);
	virtual ~TripChainItem() {}

	static LocationType getLocationType(std::string locType);
	static ItemType getItemType(std::string itemType);
};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class Activity: public sim_mob::TripChainItem {
public:
	//NOTE: I've gone with Harish's implementation here. Please double-check. ~Seth
	std::string description;
	sim_mob::Node* location;
	TripChainItem::LocationType locationType;
	bool isPrimary;
	bool isFlexible;
	bool isMandatory;

	Activity(std::string locType="node");
};

/**
 * \author Seth N. Hetu
 * \author Harish
 */
class Trip: public sim_mob::TripChainItem {
	friend class ::geo::Trip_t_pimpl;
public:
	int tripID;
	sim_mob::Node* fromLocation;
	TripChainItem::LocationType fromLocationType;
	sim_mob::Node* toLocation;
	TripChainItem::LocationType toLocationType;

	Trip(int entId=0, std::string type="Trip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(),
			int tripId=0, Node* from=nullptr, std::string fromLocType="node",
			Node* to=nullptr, std::string toLocType="node");
	virtual ~Trip() {}

	void addSubTrip(const sim_mob::SubTrip& aSubTrip);

	const std::vector<SubTrip>& getSubTrips() const {
		return subTrips;
	}

	/*void setSubTrips(const std::vector<SubTrip>& subTrips) {
		this->subTrips = subTrips;
	}*/

private:
	std::vector<SubTrip> subTrips;
};


/**
 * \author Harish
 */
class SubTrip: public sim_mob::Trip {
public:
	//sim_mob::Trip* parentTrip;
	std::string mode;
	bool isPrimaryMode;
	std::string ptLineId; //Public transit (bus or train) line identifier.

	SubTrip(int entId=0, std::string type="Trip", unsigned int seqNumber=0,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), Node* from=nullptr,
			std::string fromLocType="node", Node* to=nullptr, std::string toLocType="node",
			/*Trip* parent=nullptr,*/ std::string mode="", bool isPrimary=true, std::string ptLineId="");
	virtual ~SubTrip() {}
};


//Non-member comparison functions
bool operator==(const SubTrip& s1, const SubTrip& s2);
bool operator!=(const SubTrip& s1, const SubTrip& s2);



}
