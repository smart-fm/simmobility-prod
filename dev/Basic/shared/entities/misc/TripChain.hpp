//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <string>
#include <string>

#include "geospatial/WayPoint.hpp"
#include "util/LangHelpers.hpp"
#include "util/DailyTime.hpp"
#include "util/OneTimeFlag.hpp"

#include "conf/settings/DisableMPI.h"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif
#include <boost/shared_ptr.hpp>
namespace geo
{
//Forward Declaration
class Trip_t_pimpl;
class SubTrip_t;
}
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
 * \author Harish L
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
		IT_TRIP, IT_ACTIVITY, IT_BUSTRIP, IT_WAITBUSACTIVITY
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
	WayPoint origin;
	WayPoint destination;
	LocationType originType;
	LocationType destinationType;
	std::string travelMode;
	std::string startLocationId;
	std::string endLocationId;
	std::string startLocationType;
	std::string endLocationType;

	//Get/set personID. Please make sure not to set the personID to an Integer!
	std::string getPersonID() const;
	void setPersonID(const std::string& val);
	void setPersonID(int val);

	//TripChainItem();
	TripChainItem(std::string entId = "", std::string type = "Trip",
				DailyTime start = DailyTime(), DailyTime end = DailyTime(),
				unsigned int seqNumber = 0, int requestTime = -1);
	virtual ~TripChainItem();

	static LocationType getLocationType(std::string locType);
	static ItemType getItemType(std::string itemType);

	//initialization within person's constructor with respect to tripchain

	virtual bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *)
	{
		return false;
	}
	virtual const std::string getMode() const; //can't make it pure virtual coz the class will turn to abstract and we will face problem in XML reader

	//Helper: Convert a location type string to an object of that type.
	//TODO: This SHOULD NOT be different for the database and for XML.
	static sim_mob::TripChainItem::LocationType GetLocationTypeXML(std::string name);
};

/**
 * An activity within a trip chain. Has a location and a description.
 * \author Seth N. Hetu
 * \author Harish L
 */
class Activity : public sim_mob::TripChainItem
{
public:
	//NOTE: I've gone with Harish's implementation here. Please double-check. ~Seth
	std::string description;
	bool isPrimary;
	bool isFlexible;
	bool isMandatory;

	Activity(std::string locType = "node");
	virtual ~Activity();
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);
	virtual const std::string getMode() const;
};

/**
 * \author Seth N. Hetu
 * \author Harish
 * \author zhang huai peng
 */
class Trip : public sim_mob::TripChainItem
{
	friend class ::geo::Trip_t_pimpl;
	friend class ::geo::SubTrip_t;

public:
	std::string tripID;

	Trip(std::string entId = "", std::string type = "Trip", unsigned int seqNumber = 0, int requestTime = -1,
		DailyTime start = DailyTime(), DailyTime end = DailyTime(),
		std::string tripId = "", const Node* from = nullptr, std::string fromLocType = "node",
		const Node* to = nullptr, std::string toLocType = "node");
	virtual ~Trip();

	void addSubTrip(const sim_mob::SubTrip& aSubTrip);

	const std::vector<sim_mob::SubTrip>& getSubTrips() const;

	std::vector<sim_mob::SubTrip>& getSubTripsRW();

	void setSubTrips(const std::vector<sim_mob::SubTrip>& subTrips);
	bool setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip *);

	/**
	 * get mode of first subtrip of trip
	 * @return mode of first sub-trip of trip, if it exists; empty string otherwise
	 */
	virtual const std::string getMode() const;

private:
	std::vector<sim_mob::SubTrip> subTrips;
};

class FMODSchedule
{
public:

	struct STOP
	{
		int stopId;
		int scheduleId;
		double dwellTime;
		std::string arrivalTime;
		std::string depatureTime;
		std::vector< int > boardingPassengers;
		std::vector< int > alightingPassengers;
	};
	std::vector<STOP> stopSchdules;
	std::vector<Node*> routes;
	std::vector<const Person*> insidePassengers;
};

/**
 * \author Harish
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

	std::string mode;
	bool isPrimaryMode;
	std::string ptLineId; //Public transit (bus or train) line identifier.

	FMODSchedule* schedule;
	mutable sim_mob::TravelMetric::CDB_TraverseType cbdTraverseType;
	const std::string getMode() const;
	const std::string getBusLineID() const;

	bool isPT_Walk;
	double walkTime;

};

//Non-member comparison functions
bool operator==(const SubTrip& s1, const SubTrip& s2);
bool operator!=(const SubTrip& s1, const SubTrip& s2);

}
