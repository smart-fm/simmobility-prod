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
#include "geospatial/streetdir/StreetDirectory.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif
namespace geo
{
//Forward Declaration
class Trip_t_pimpl;
class SubTrip_t;
}
namespace sim_mob {

//Forward declarations
class Node;
class SubTrip;
class Person;



/**
 * Base class for elements in a trip chain.
 * \author Harish L
 */
class TripChainItem {
public:
	/**
	 * Type of location of this trip chain item.
	 *
	 * \note
	 * If you make changes in the following enum, you have to manually make required modifications
	 * in the xml reader too.
	 */
	enum LocationType {
		LT_BUILDING, LT_NODE, LT_LINK, LT_PUBLIC_TRANSIT_STOP
	};

	/**
	 * Type of this trip chain item.
	 *
	 * \note
	 * If you make changes in the following enum, you have to manually make required modifications
	 * in the xml reader too.
	 */
	enum ItemType {
		IT_TRIP, IT_ACTIVITY, IT_BUSTRIP, IT_FMODSIM
	};

	std::string personID;//replaces entityID
	ItemType itemType;
	unsigned int sequenceNumber;
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;
	int requestTime;

	//TripChainItem();
	TripChainItem(std::string entId= "", std::string type="Trip",
				DailyTime start=DailyTime(), DailyTime end=DailyTime(),
				unsigned int seqNumber=0, int requestTime=-1);
	virtual ~TripChainItem() {}

	static LocationType getLocationType(std::string locType);
	static ItemType getItemType(std::string itemType);
	//initialization within person's constructor with respect to tripchain
	virtual bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *) { return false; }
	virtual  const std::string getMode(const sim_mob::SubTrip *subTrip) const { return "<ERROR>"; };//can't make it pur virtual coz the class will turn to abstract and we will face problem in XML reader
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
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);
	 const std::string getMode(const sim_mob::SubTrip *subTrip) const  { return "Activity";}
};

/**
 * \author Seth N. Hetu
 * \author Harish
 * \author zhang huai peng
 */
class Trip: public sim_mob::TripChainItem {

	friend class ::geo::Trip_t_pimpl;
	friend class ::geo::SubTrip_t;

public:
	std::string tripID;
	WayPoint fromLocation;
	TripChainItem::LocationType fromLocationType;
	WayPoint toLocation;
	TripChainItem::LocationType toLocationType;

	Trip(std::string entId = "", std::string type="Trip", unsigned int seqNumber=0, int requestTime=-1,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(),
			std::string tripId = "", Node* from=nullptr, std::string fromLocType="node",
			Node* to=nullptr, std::string toLocType="node");
	virtual ~Trip() {}

	void addSubTrip(const sim_mob::SubTrip& aSubTrip);

	const std::vector<sim_mob::SubTrip>& getSubTrips() const {
		return subTrips;
	}

	std::vector<sim_mob::SubTrip>& getSubTripsRW() {
		return subTrips;
	}

	void setSubTrips(const std::vector<sim_mob::SubTrip>& subTrips) {
		this->subTrips = subTrips;
	}
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);
	const std::string getMode(const sim_mob::SubTrip *subTrip) const;

private:
	std::vector<sim_mob::SubTrip> subTrips;
};

class FMODSchedule{
public:
	struct STOP
	{
		int stop_id;
		double dwell_time;
		std::string arrival_time;
		std::string depature_time;
		std::vector< int > boardingpassengers;
		std::vector< int > alightingpassengers;
	};
	std::vector<STOP> stop_schdules;
	std::vector<Node*> routes;
	std::vector<const Person*> insidepassengers;
};
/**
 * \author Harish
 * \author zhang huai peng
 */
class SubTrip: public sim_mob::Trip {
public:
	//sim_mob::Trip* parentTrip;
	std::string mode;
	bool isPrimaryMode;
	std::string ptLineId; //Public transit (bus or train) line identifier.

	FMODSchedule* schedule;

	SubTrip(std::string entId="", std::string type="Trip", unsigned int seqNumber=0,int requestTime=-1,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), Node* from=nullptr,
			std::string fromLocType="node", Node* to=nullptr, std::string toLocType="node",
			/*Trip* parent=nullptr,*/ std::string mode="", bool isPrimary=true, std::string ptLineId="");
	const std::string getMode() const ;
//	{
//		std::cout << "Mode for subtrip " << this << " from " << this->fromLocation->getID() << " to " << this->toLocation->getID() << " is " << mode << std::endl;
//		return mode;
//	}//this is not implementation of a vrtual function
	virtual ~SubTrip() {}
};

/**
 * \author zhang huai peng
 *//*
class FMODTrip : public sim_mob::Trip {

public:
	struct STOP
	{
		std::string stop_id;
		std::string arrival_time;
		std::string depature_time;
		std::vector< std::string > boardingpassengers;
		std::vector< std::string > alightingpassengers;
	};
	std::vector<STOP> stop_schdules;
	struct PASSENGER
	{
		std::string client_id;
		int price;
	};
	std::vector<PASSENGER> passengers;
	struct ROUTE
	{
		std::string id;
		int type;
	};
	std::vector<ROUTE> routes;

	FMODTrip(std::string entId="", std::string type="Trip", unsigned int seqNumber=0,int requestTime=-1,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), Node* from=nullptr,
			std::string fromLocType="node", Node* to=nullptr, std::string toLocType="node");

	virtual ~FMODTrip() {}
};
*/

//Non-member comparison functions
bool operator==(const SubTrip& s1, const SubTrip& s2);
bool operator!=(const SubTrip& s1, const SubTrip& s2);



}
