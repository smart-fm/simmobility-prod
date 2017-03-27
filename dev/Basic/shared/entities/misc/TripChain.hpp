//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <string>
#include "behavioral/StopType.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"
#include "util/OneTimeFlag.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif
#include <boost/shared_ptr.hpp>

namespace sim_mob
{

//Forward declarations
class Node;
class SubTrip;
class Person;


/// simple structure used to collect travel time information

struct TravelMetric
{
	WayPoint origin, destination;
	DailyTime startTime, endTime;
	double travelTime;
	double distance;
	bool started, finalized, valid;

	TravelMetric() : started(false), finalized(false),
	valid(false), cbdTraverseType(CBD_NONE),
	travelTime(0), cbdTravelTime(0),
	cbdOrigin(WayPoint()), cbdDestination(WayPoint()),
	origin(WayPoint()), destination(WayPoint()),
	startTime(DailyTime()), endTime(DailyTime()),
	cbdStartTime(DailyTime()), cbdEndTime(DailyTime()),
	distance(0), cbdDistance(0)
	{

	}

	//CBD information

	enum CDB_TraverseType
	{
		CBD_ENTER,
		CBD_EXIT,
		CBD_PASS,
		CBD_NONE,
	};

	CDB_TraverseType cbdTraverseType;
	WayPoint cbdOrigin, cbdDestination;
	DailyTime cbdStartTime, cbdEndTime;
	OneTimeFlag cbdEntered, cbdExitted;
	double cbdDistance;
	double cbdTravelTime;

	static double getTimeDiffHours(const DailyTime &end, const DailyTime &start)
	{
		double t = (double((end - start).getValue()) / 1000) / 3600;
		return t;
	}

	void reset()
	{
		started = false;
		finalized = false;
		valid = false;
		cbdTraverseType = CBD_NONE;
		travelTime = 0;
		cbdTravelTime = 0;
		cbdOrigin = WayPoint();
		cbdDestination = WayPoint();
		origin = WayPoint();
		destination = WayPoint();
		startTime = DailyTime();
		endTime = DailyTime();
		cbdStartTime = DailyTime();
		cbdEndTime = DailyTime();
		distance = 0;
		cbdDistance = 0;
	}
};

/**
 * Base class for elements in a trip chain.
 * \author Harish Loganathan
 */
class TripChainItem
{
public:

	/**
	 * Type of location of this trip chain item.
	 *
	 * \note
	 * If you make changes in the following enum, you have to manually make required modifications
	 * in the xml reader too.
	 */
	enum LocationType
	{
		LT_BUILDING, LT_NODE, LT_LINK, LT_PUBLIC_TRANSIT_STOP
	};

	/**
	 * Type of this trip chain item.
	 *
	 * \note
	 * If you make changes in the following enum, you have to manually make required modifications
	 * in the xml reader too.
	 */
	enum ItemType
	{
		IT_TRIP, IT_ACTIVITY, IT_BUSTRIP, IT_FMODSIM, IT_WAITBUSACTIVITY, IT_WAITTRAINACTIVITY,IT_TRAINTRIP
	};


private:
	///Note: The personID was being used quite randomly; being set to -1, to agent.getId(), and to other
	//       bogus integer values. So I'm making it private, and requiring all modifications to use the
	//       setPersonID() public function. Please be careful! This kind of usage can easily corrupt memory. ~Seth
	std::string personID; //replaces entityID

public:
	ItemType itemType;
	StopType purpose;
	unsigned int sequenceNumber;
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;
	int requestTime;
	WayPoint origin;
	WayPoint destination;
	LocationType originType;
	LocationType destinationType;
	int originZoneCode;
	int destinationZoneCode;
	std::string travelMode;
	std::string startLocationId;
	std::string endLocationId;
	std::string startLocationType;
	std::string endLocationType;
	unsigned int edgeId;
	std::string serviceLine;
	
	/**Indicates the number of times the trip is to be loaded [Added for short-term demand calibration]*/
	unsigned int load_factor;

	//Get/set personID. Please make sure not to set the personID to an Integer!
	std::string getPersonID() const;
	void setPersonID(const std::string& val);
	void setPersonID(int val);

	TripChainItem(std::string purpose = std::string(),
			std::string entId = std::string(),
			std::string type = "Trip",
			DailyTime start = DailyTime(), DailyTime end = DailyTime(),
			unsigned int seqNumber=0,
			int requestTime=-1,
			std::string mode=std::string(),
			unsigned edgeId=0);
	virtual ~TripChainItem();

	static LocationType getLocationType(std::string locType);
	static ItemType getItemType(std::string itemType);
	static StopType getItemPurpose(std::string purpose);

	//initialization within person's constructor with respect to tripchain

	virtual bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *)
	{
		return false;
	}
	const std::string getMode() const;

	//Helper: Convert a location type string to an object of that type.
	//TODO: This SHOULD NOT be different for the database and for XML.
	static sim_mob::TripChainItem::LocationType GetLocationTypeXML(std::string name);
};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class Activity : public sim_mob::TripChainItem
{
public:
	const Node* location;
	bool isPrimary;
	bool isFlexible;
	bool isMandatory;

	Activity(std::string locType = "node", std::string purpose = std::string());
	virtual ~Activity();
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);
};

/**
 * \author Seth N. Hetu
 * \author Harish Loganathan
 * \author zhang huai peng
 */
class Trip : public sim_mob::TripChainItem
{
public:
	std::string tripID;

	Trip(std::string entId = "",
		std::string type = "Trip",
		unsigned int seqNumber = 0,
		int requestTime = -1,
		DailyTime start = DailyTime(),
		DailyTime end = DailyTime(),
		std::string tripId = "",
		const Node* from = nullptr,
		std::string fromLocType = "node",
		const Node* to = nullptr,
		std::string toLocType = "node",
		std::string mode = std::string(),
		std::string purpose = std::string());
	virtual ~Trip();

	void addSubTrip(const sim_mob::SubTrip& aSubTrip);

	const std::vector<sim_mob::SubTrip>& getSubTrips() const;

	std::vector<sim_mob::SubTrip>& getSubTripsRW();

	void setSubTrips(const std::vector<sim_mob::SubTrip>& subTrips);
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);

private:
	std::vector<sim_mob::SubTrip> subTrips;
} ;

/**
 * \author Harish Loganathan
 * \author zhang huai peng
 */
class SubTrip : public sim_mob::Trip
{
public:
	SubTrip(std::string entId = "", std::string type = "Trip", unsigned int seqNumber = 0, int requestTime = -1,
			DailyTime start = DailyTime(), DailyTime end = DailyTime(), const Node* from = nullptr,
			std::string fromLocType = "node", const Node* to = nullptr, std::string toLocType = "node",
			std::string mode = "", bool isPrimary = true, std::string ptLineId = "");
	virtual ~SubTrip();

	std::string ptLineId; //Public transit (bus or train) line identifier.

	mutable sim_mob::TravelMetric::CDB_TraverseType cbdTraverseType;
	const std::string getBusLineID() const;

	bool isPT_Walk;
	double walkTime;


	bool isTT_Walk;

} ;

//Non-member comparison functions
bool operator==(const SubTrip& s1, const SubTrip& s2);
bool operator!=(const SubTrip& s1, const SubTrip& s2);

}
