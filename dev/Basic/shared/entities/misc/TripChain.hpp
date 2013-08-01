/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>
#include <string>

#include "geospatial/streetdir/WayPoint.hpp"
#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"

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


private:
	///Note: The personID was being used quite randomly; being set to -1, to agent.getId(), and to other
	//       bogus integer values. So I'm making it private, and requiring all modifications to use the
	//       setPersonID() public function. Please be careful! This kind of usage can easily corrupt memory. ~Seth
	std::string personID; //replaces entityID

public:
	ItemType itemType;
	unsigned int sequenceNumber;
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;
	int requestTime;

	//Get/set personID. Please make sure not to set the personID to an Integer!
	std::string getPersonID() const;
	void setPersonID(const std::string& val);
	void setPersonID(int val);

	//TripChainItem();
	TripChainItem(std::string entId= "", std::string type="Trip",
				DailyTime start=DailyTime(), DailyTime end=DailyTime(),
				unsigned int seqNumber=0, int requestTime=-1);
	virtual ~TripChainItem() {}

	static LocationType getLocationType(std::string locType);
	static ItemType getItemType(std::string itemType);

	//initialization within person's constructor with respect to tripchain
	virtual bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *) { return false; }
	virtual  const std::string getMode(const sim_mob::SubTrip *subTrip) const { return "<ERROR>"; };//can't make it pure virtual coz the class will turn to abstract and we will face problem in XML reader

	//Helper: Convert a location type string to an object of that type.
	//TODO: This SHOULD NOT be different for the database and for XML.
	static sim_mob::TripChainItem::LocationType  GetLocationTypeXML(std::string name);
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

	const std::vector<sim_mob::SubTrip>& getSubTrips() const;

	std::vector<sim_mob::SubTrip>& getSubTripsRW();

	void setSubTrips(const std::vector<sim_mob::SubTrip>& subTrips);
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);
	const std::string getMode(const sim_mob::SubTrip *subTrip) const;
private:
	std::vector<sim_mob::SubTrip> subTrips;
};


/**
 * \author Harish
 * \author zhang huai peng
 */
class SubTrip: public sim_mob::Trip {
public:
	virtual ~SubTrip() {}

	//sim_mob::Trip* parentTrip;
	std::string mode;
	bool isPrimaryMode;
	std::string ptLineId; //Public transit (bus or train) line identifier.

	SubTrip(std::string entId="", std::string type="Trip", unsigned int seqNumber=0,int requestTime=-1,
			DailyTime start=DailyTime(), DailyTime end=DailyTime(), Node* from=nullptr,
			std::string fromLocType="node", Node* to=nullptr, std::string toLocType="node",
			/*Trip* parent=nullptr,*/ std::string mode="", bool isPrimary=true, std::string ptLineId="");
	const std::string getMode() const ;
};


//Non-member comparison functions
bool operator==(const SubTrip& s1, const SubTrip& s2);
bool operator!=(const SubTrip& s1, const SubTrip& s2);



}
