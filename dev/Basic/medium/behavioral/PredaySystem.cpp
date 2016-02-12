//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * SystemOfModels.cpp
 *
 *  Created on: Nov 7, 2013
 *      Author: Harish Loganathan
 */

#include "PredaySystem.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdint.h>
#include "behavioral/lua/PredayLuaProvider.hpp"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"
#include "behavioral/params/TourModeParams.hpp"
#include "behavioral/params/TourModeDestinationParams.hpp"
#include "behavioral/StopType.hpp"
#include "behavioral/StopType.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "database/DatabaseHelper.hpp"
#include "database/DB_Connection.hpp"
#include "logging/Log.hpp"
#include "mongo/client/dbclient.h"
#include "PredayClasses.hpp"
#include "util/Utils.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace mongo;

namespace {
	const double HIGH_TRAVEL_TIME = 999.0;
	const double PEDESTRIAN_WALK_SPEED = 5.0; //kmph
	const double WALKABLE_DISTANCE = 3.0; //km
	const int AM_PEAK_LOW = 10;
	const int AM_PEAK_HIGH = 13;
	const int PM_PEAK_LOW = 30;
	const int PM_PEAK_HIGH = 33;
	const int MAX_STOPS_IN_HALF_TOUR = 3;

	// ISG choices
	const int WORK_CHOICE_ISG = 1;
	const int EDU_CHOICE_ISG = 2;
	const int SHOP_CHOICE_ISG = 3;
	const int OTHER_CHOICE_ISG = 4;
	const int QUIT_CHOICE_ISG = 5;

	//Time related
	const double FIRST_WINDOW = 3.25;
	const int FIRST_INDEX = 1;
	const double LAST_WINDOW = 26.75;
	const int LAST_INDEX = 48;
	const int FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE = 5;
	const int LAST_INDEX_FOR_PUBLIC_TANSIT_MODE = 45;

	//Half tours
	const uint8_t FIRST_HALF_TOUR = 1;
	const uint8_t SECOND_HALF_TOUR = 2;

	//Maximum number of sub tours per tour
	const unsigned short MAX_SUB_TOURS = 1;

	const double OPERATIONAL_COST = 0.147;

	const double TAXI_FLAG_DOWN_PRICE = 3.4;
	const double TAXI_CENTRAL_LOCATION_SURCHARGE = 3.0;
	const double TAXI_UNIT_PRICE = 0.22;
	const double UNIT_FOR_FIRST_10KM = 0.4;
	const double UNIT_AFTER_10KM = 0.35;

	std::map<int, std::string> setModeMap() {
		// 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
		// 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
		//mode_idx_ref = { 1 : 3, 2 : 5, 3 : 3, 4 : 1, 5 : 6, 6 : 6, 7 : 8, 8 : 2, 9 : 4 }
		std::map<int, std::string> res;
		res[1] = "BusTravel";
		res[2] = "MRT";
		res[3] = "PrivateBus";
		res[4] = "Car";
		res[5] = "Car Sharing 2";
		res[6] = "Car Sharing 3";
		res[7] = "Motorcycle";
		res[8] = "Walk";
		res[9] = "Taxi";
		return res;
	}

	/**
	 * This function is to assign modes which are supported by withinday
	 */
	std::map<int, std::string> setModeMapTemp() {
		// 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
		// 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
		//mode_idx_ref = { 1 : 3, 2 : 5, 3 : 3, 4 : 1, 5 : 6, 6 : 6, 7 : 8, 8 : 2, 9 : 4 }
		std::map<int, std::string> res;
		res[1] = "BusTravel";
		res[2] = "Car";
		res[3] = "Car";
		res[4] = "Car";
		res[5] = "Car";
		res[6] = "Car";
		res[7] = "Motorcycle";
		res[8] = "Walk";
		res[9] = "Taxi";
		return res;
	}

	const std::map<int,std::string> modeMap = setModeMap();

	/*
	 * There are 48 half-hour indexes in a day from 3.25 to 26.75.
	 * Given a index i, its choice time window can be determined by (i * 0.5 + 2.75)
	 */
	inline double getTimeWindowFromIndex(const double index) {
		return ((index * 0.5 /*half hour windows*/) + 2.75 /*the day starts at 3.25*/);
	}

	inline double getIndexFromTimeWindow(const double window) {
		return ((window - 2.75 /*the day starts at 3.25*/) / 0.5);
	}

	double alignTime(double time, double lowerBound, double upperBound, const std::string& personId, std::string caller) {
		if(lowerBound > upperBound)
		{
			std::stringstream ss;
			ss << "Cannot align time with invalid bounds|Person: " << personId << "|" << caller << "|" << "alignTime(" << time << "," << lowerBound << "," << upperBound << ")" << std::endl;
			throw std::runtime_error(ss.str());
		}

		// align to corresponding time window
		//1. split the computed tour end time into integral and fractional parts
		double intPart,fractPart;
		fractPart = std::modf(time, &intPart);

		//2. perform sanity checks on the integral part and align the fractional part to nearest time window
		if (time < lowerBound) {
			time = lowerBound;
		}
		else if (time > upperBound) {
			time = upperBound;
		}
		else if(std::abs(fractPart) < 0.5) {
			time = intPart + 0.25;
		}
		else {
			time = intPart + 0.75;
		}

		return time;
	}

	/**
	 * generates a random time  within the time window passed in preday's representation.
	 *
	 * @param window time window in preday format (E.g. 4.75 => 4:30 to 4:59 AM)
	 * @return a random time within the window in hh24:mm:ss format
	 */
	std::string getRandomTimeInWindow(double mid) {
		int hour = int(std::floor(mid));
		int minute = (Utils::generateInt(0,29)) + ((mid - hour - 0.25)*60);
		std::stringstream random_time;
		hour = hour % 24;
		if (hour < 10) {
			random_time << "0" << hour << ":";
		}
		else {
			random_time << hour << ":";
		}
		if (minute < 10) {
			random_time << "0" << minute << ":";
		}
		else {
			random_time << minute << ":";
		}
		random_time << "00"; //seconds
		return random_time.str();
	}

	inline std::string constructTripChainItemId(const std::string& pid, uint8_t tourNum, uint8_t seqNum, const std::string suffix="") {
		std::stringstream id;
		id << pid << "-" << tourNum << "-" << seqNum << suffix;
		return id.str();
	}
} // anon namespace

PredaySystem::PredaySystem(PersonParams& personParams,
		const ZoneMap& zoneMap, const boost::unordered_map<int,int>& zoneIdLookup,
		const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap,
		const std::map<std::string, db::MongoDao*>& mongoDao,
		const std::vector<OD_Pair>& unavailableODs,
		const std::map<int, int>& mtz12_08Map)
: personParams(personParams), zoneMap(zoneMap), zoneIdLookup(zoneIdLookup),
  amCostMap(amCostMap), pmCostMap(pmCostMap), opCostMap(opCostMap),
  mongoDao(mongoDao), unavailableODs(unavailableODs), MTZ12_MTZ08_Map(mtz12_08Map),
  firstAvailableTimeIndex(FIRST_INDEX), logStream(std::stringstream::out)
{}

PredaySystem::~PredaySystem()
{
	for(TourList::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		safe_delete_item(*tourIt);
	}
	tours.clear();
}

bool sim_mob::medium::PredaySystem::predictUsualWorkLocation(bool firstOfMultiple) {
    UsualWorkParams usualWorkParams;
	usualWorkParams.setFirstOfMultiple((int) firstOfMultiple);
	usualWorkParams.setSubsequentOfMultiple((int) !firstOfMultiple);

	usualWorkParams.setZoneEmployment(zoneMap.at(zoneIdLookup.at(personParams.getFixedWorkLocation()))->getEmployment());

	if(personParams.getHomeLocation() != personParams.getFixedWorkLocation()) {
		usualWorkParams.setWalkDistanceAm(amCostMap.at(personParams.getHomeLocation()).at(personParams.getFixedWorkLocation())->getDistance());
		usualWorkParams.setWalkDistancePm(pmCostMap.at(personParams.getHomeLocation()).at(personParams.getFixedWorkLocation())->getDistance());
	}
	else {
		usualWorkParams.setWalkDistanceAm(0);
		usualWorkParams.setWalkDistancePm(0);
	}
	return PredayLuaProvider::getPredayModel().predictUsualWorkLocation(personParams, usualWorkParams);
}

void PredaySystem::constructTourModeParams(TourModeParams& tmParams, int destination, StopType tourType)
{
	tmParams.setStopType(tourType);

	ZoneParams* znOrgObj = zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()));
	ZoneParams* znDesObj = zoneMap.at(zoneIdLookup.at(destination));
	tmParams.setCostCarParking(znDesObj->getParkingRate());
	tmParams.setCentralZone(znDesObj->getCentralDummy());
	tmParams.setCbdOrgZone(znOrgObj->getCbdDummy());
	tmParams.setCbdDestZone(znDesObj->getCbdDummy());
	tmParams.setResidentSize(znOrgObj->getResidentWorkers());
	tmParams.setWorkOp(znDesObj->getEmployment());
	tmParams.setEducationOp(znDesObj->getTotalEnrollment());
	tmParams.setOriginArea(znOrgObj->getArea());
	tmParams.setDestinationArea(znDesObj->getArea());
	tmParams.setCostIncrease(0);
	if(personParams.getHomeLocation() != destination)
	{
		CostParams* amObj = amCostMap.at(personParams.getHomeLocation()).at(destination);
		CostParams* pmObj = pmCostMap.at(destination).at(personParams.getHomeLocation());
		tmParams.setCostPublicFirst(amObj->getPubCost());
		tmParams.setCostPublicSecond(pmObj->getPubCost());
		tmParams.setCostCarErpFirst(amObj->getCarCostErp());
		tmParams.setCostCarErpSecond(pmObj->getCarCostErp());
		tmParams.setCostCarOpFirst(amObj->getDistance() * OPERATIONAL_COST);
		tmParams.setCostCarOpSecond(pmObj->getDistance() * OPERATIONAL_COST);
		tmParams.setWalkDistance1(amObj->getDistance());
		tmParams.setWalkDistance2(pmObj->getDistance());
		tmParams.setTtPublicIvtFirst(amObj->getPubIvt());
		tmParams.setTtPublicIvtSecond(pmObj->getPubIvt());
		tmParams.setTtPublicWaitingFirst(amObj->getPubWtt());
		tmParams.setTtPublicWaitingSecond(pmObj->getPubWtt());
		tmParams.setTtPublicWalkFirst(amObj->getPubWalkt());
		tmParams.setTtPublicWalkSecond(pmObj->getPubWalkt());
		tmParams.setTtCarIvtFirst(amObj->getCarIvt());
		tmParams.setTtCarIvtSecond(pmObj->getCarIvt());
		tmParams.setAvgTransfer((amObj->getAvgTransfer() + pmObj->getAvgTransfer())/2);

		//set availabilities
		tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
		tmParams.setShare2Available(1);
		tmParams.setShare3Available(1);
		tmParams.setPublicBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		tmParams.setMrtAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		tmParams.setPrivateBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
		tmParams.setWalkAvailable(amObj->getDistance() <= WALKABLE_DISTANCE && pmObj->getDistance() <= WALKABLE_DISTANCE);
		tmParams.setTaxiAvailable(1);
		tmParams.setMotorAvailable(personParams.getMotorLicense() * personParams.getMotorOwn());
	}
	else
	{
		tmParams.setCostPublicFirst(0);
		tmParams.setCostPublicSecond(0);
		tmParams.setCostCarErpFirst(0);
		tmParams.setCostCarErpSecond(0);
		tmParams.setCostCarOpFirst(0);
		tmParams.setCostCarOpSecond(0);
		tmParams.setCostCarParking(0);
		tmParams.setWalkDistance1(0);
		tmParams.setWalkDistance2(0);
		tmParams.setTtPublicIvtFirst(0);
		tmParams.setTtPublicIvtSecond(0);
		tmParams.setTtPublicWaitingFirst(0);
		tmParams.setTtPublicWaitingSecond(0);
		tmParams.setTtPublicWalkFirst(0);
		tmParams.setTtPublicWalkSecond(0);
		tmParams.setTtCarIvtFirst(0);
		tmParams.setTtCarIvtSecond(0);
		tmParams.setAvgTransfer(0);

		//set availabilities
		tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
		tmParams.setShare2Available(1);
		tmParams.setShare3Available(1);
		tmParams.setPublicBusAvailable(1);
		tmParams.setMrtAvailable(1);
		tmParams.setPrivateBusAvailable(1);
		tmParams.setWalkAvailable(1);
		tmParams.setTaxiAvailable(1);
		tmParams.setMotorAvailable(1);
	}
}

void PredaySystem::predictTourMode(Tour& tour)
{
	TourModeParams tmParams;
	constructTourModeParams(tmParams, tour.getTourDestination(), tour.getTourType());
	tour.setTourMode(PredayLuaProvider::getPredayModel().predictTourMode(personParams, tmParams));
}

void sim_mob::medium::PredaySystem::predictSubTours(Tour& parentTour)
{
	SubTourParams workBasedSubTourParams(parentTour);

	TourList& subToursList = parentTour.subTours;
	int choice = 0;
	for(unsigned short i=0; i<MAX_SUB_TOURS; i++)
	{
		choice = PredayLuaProvider::getPredayModel().predictWorkBasedSubTour(personParams, workBasedSubTourParams);
		if(choice == 2) { break; } //QUIT
		else { subToursList.push_back(Tour(NULL_STOP,true)); } //NON-QUIT
	}

	// mode/destination and time of day for each sub tour
	for(TourList::iterator tourIt=subToursList.begin(); tourIt!=subToursList.end(); tourIt++)
	{
		//set mode and destination
		Tour& subTour = *tourIt;
		predictSubTourModeDestination(subTour, parentTour);

		//unavail travel time to predicted destination by predicted mode
		blockTravelTimeToSubTourLocation(subTour, parentTour, workBasedSubTourParams);
		if(workBasedSubTourParams.allWindowsUnavailable())
		{
			//no-time for subtours. remove this and all subsequent sub-tours
			while(tourIt!=subToursList.end()) { tourIt = subToursList.erase(tourIt); }
			return;
		}

		// predict time of day
		TimeWindowAvailability timeWindow = predictSubTourTimeOfDay(subTour, workBasedSubTourParams);

		Stop* primaryActivity = new Stop(subTour.getTourType(), subTour, true /*primary activity*/, true /*stop in first half tour*/);
		primaryActivity->setStopMode(subTour.getTourMode());
		primaryActivity->setStopLocation(subTour.getTourDestination());
		primaryActivity->setStopLocationId(zoneIdLookup.at(subTour.getTourDestination()));
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		subTour.setPrimaryStop(primaryActivity);
		subTour.addStop(primaryActivity);
		calculateSubTourTimeWindow(subTour, parentTour); // estimate travel time to/from activity location
		workBasedSubTourParams.blockTime(subTour.getStartTime(), subTour.getEndTime());
	}
}

void PredaySystem::predictSubTourModeDestination(Tour& subTour, const Tour& parentTour)
{
	TourModeDestinationParams stmdParams(zoneMap, amCostMap, pmCostMap, personParams, subTour.getTourType(), unavailableODs, MTZ12_MTZ08_Map);
	stmdParams.setOrigin(parentTour.getTourDestination()); //origin is primary activity location of parentTour (not home location)
	stmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(parentTour.getTourDestination()))->getCbdDummy());
	stmdParams.setModeForParentWorkTour(parentTour.getTourMode());
	int modeDest = PredayLuaProvider::getPredayModel().predictSubTourModeDestination(personParams, stmdParams);
	subTour.setTourMode(stmdParams.getMode(modeDest));
	int zone_id = stmdParams.getDestination(modeDest);
	subTour.setTourDestination(zoneMap.at(zone_id)->getZoneCode());
}

void PredaySystem::predictTourModeDestination(Tour& tour)
{
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, tour.getTourType(), unavailableODs, MTZ12_MTZ08_Map);
	tmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()))->getCbdDummy());
	int modeDest = PredayLuaProvider::getPredayModel().predictTourModeDestination(personParams, tmdParams);
	tour.setTourMode(tmdParams.getMode(modeDest));
	int zone_id = tmdParams.getDestination(modeDest);
	tour.setTourDestination(zoneMap.at(zone_id)->getZoneCode());
}

TimeWindowAvailability PredaySystem::predictSubTourTimeOfDay(Tour& subTour, SubTourParams& subTourParams)
{
	int timeWndw;
	if(!subTour.isSubTour()) { throw std::runtime_error("predictSubTourTimeOfDay() is only for sub-tours"); };
	timeWndw = PredayLuaProvider::getPredayModel().predictSubTourTimeOfDay(personParams, subTourParams);
	return TimeWindowAvailability::timeWindowsLookup.at(timeWndw - 1); //timeWndw ranges from 1 - 1176. Vector starts from 0.
}

TimeWindowAvailability PredaySystem::predictTourTimeOfDay(Tour& tour) {
	int timeWndw;
	if(tour.isSubTour()) { throw std::runtime_error("predictTourTimeOfDay() is not meant for sub tours"); }
	int origin_2012 = personParams.getHomeLocation();
	int destination_2012 = tour.getTourDestination();
	int origin = MTZ12_MTZ08_Map.at(origin_2012);
	int destination = MTZ12_MTZ08_Map.at(destination_2012);
	TourTimeOfDayParams todParams;
	todParams.setTourMode(tour.getTourMode());
	todParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin_2012))->getCbdDummy());
	todParams.setCbdDestZone(zoneMap.at(zoneIdLookup.at(destination_2012))->getCbdDummy());
	std::vector<double>& ttFirstHalfTour = todParams.travelTimesFirstHalfTour;
	std::vector<double>& ttSecondHalfTour = todParams.travelTimesSecondHalfTour;

	if(origin != destination && origin_2012 != destination_2012) {
		for (uint32_t i=FIRST_INDEX; i<=LAST_INDEX; i++) {
			switch (tour.getTourMode())
			{
			case 1: // Fall through
			case 2:
			case 3:
			{
				BSONObj bsonObjTT = BSON("origin" << origin << "destination" << destination);
				BSONObj tCostBusDoc;
				mongoDao["tcost_bus"]->getOne(bsonObjTT, tCostBusDoc);

				std::stringstream arrivalField, departureField;
				arrivalField << "TT_bus_arrival_" << i;
				departureField << "TT_bus_departure_" << i;
				if(tCostBusDoc.getField(arrivalField.str()).isNumber())
				{
					ttFirstHalfTour.push_back(tCostBusDoc.getField(arrivalField.str()).Number());
				}
				else
				{
					ttFirstHalfTour.push_back(HIGH_TRAVEL_TIME);
				}
				if(tCostBusDoc.getField(departureField.str()).isNumber())
				{
					ttSecondHalfTour.push_back(tCostBusDoc.getField(departureField.str()).Number());
				}
				else
				{
					ttSecondHalfTour.push_back(HIGH_TRAVEL_TIME);
				}
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				BSONObj bsonObjTT = BSON("origin" << origin << "destination" << destination);
				BSONObj tCostCarDoc;
				mongoDao["tcost_car"]->getOne(bsonObjTT, tCostCarDoc);
				std::stringstream arrivalField, departureField;
				arrivalField << "TT_car_arrival_" << i;
				departureField << "TT_car_departure_" << i;
				if(tCostCarDoc.getField(arrivalField.str()).isNumber())
				{
					ttFirstHalfTour.push_back(tCostCarDoc.getField(arrivalField.str()).Number());
				}
				else
				{
					ttFirstHalfTour.push_back(HIGH_TRAVEL_TIME);
				}
				if(tCostCarDoc.getField(departureField.str()).isNumber())
				{
					ttSecondHalfTour.push_back(tCostCarDoc.getField(departureField.str()).Number());
				}
				else
				{
					ttSecondHalfTour.push_back(HIGH_TRAVEL_TIME);
				}
				break;
			}
			case 8:
			{
				double travelTime = 0.0;
				if(i>=AM_PEAK_LOW && i<=AM_PEAK_HIGH) // if i is in AM peak period
				{
					CostParams* amDistanceObj = amCostMap.at(origin_2012).at(destination_2012);
					travelTime = amDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
				}
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{
					CostParams* pmDistanceObj = pmCostMap.at(origin_2012).at(destination_2012);
					travelTime = pmDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
				}
				else // if i is in off-peak period
				{
					CostParams* opDistanceObj = opCostMap.at(origin_2012).at(destination_2012);
					travelTime = opDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
				}

				ttFirstHalfTour.push_back(travelTime);
				ttSecondHalfTour.push_back(travelTime);
				break;
			}
			}
		}
	}
	else {
		for (uint32_t i = FIRST_INDEX; i <= LAST_INDEX; i++) {
			ttFirstHalfTour.push_back(0);
			ttSecondHalfTour.push_back(0);
		}
	}

	// find costs
	int home = personParams.getHomeLocation(), primaryStopLoc = tour.getTourDestination();
	if(home!=primaryStopLoc)
	{
		const CostParams* amHT1 = amCostMap.at(home).at(primaryStopLoc);
		const CostParams* pmHT1 = pmCostMap.at(home).at(primaryStopLoc);
		const CostParams* opHT1 = opCostMap.at(home).at(primaryStopLoc);
		const CostParams* amHT2 = amCostMap.at(primaryStopLoc).at(home);
		const CostParams* pmHT2 = pmCostMap.at(primaryStopLoc).at(home);
		const CostParams* opHT2 = opCostMap.at(primaryStopLoc).at(home);
		switch (tour.getTourMode())
		{
		case 1: // Fall through
		case 2:
		case 3:
		{	//for Public bus, MRT/LRT, private bus
			todParams.setCostHt1Am(amHT1->getPubCost());
			todParams.setCostHt1Pm(pmHT1->getPubCost());
			todParams.setCostHt1Op(opHT1->getPubCost());
			todParams.setCostHt2Am(amHT2->getPubCost());
			todParams.setCostHt2Pm(pmHT2->getPubCost());
			todParams.setCostHt2Op(opHT2->getPubCost());
			break;
		}
		case 4:
		{	// drive1
			double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
			todParams.setCostHt1Am(amHT1->getCarCostErp() + ht1ParkingRate + (amHT1->getDistance()*OPERATIONAL_COST));
			todParams.setCostHt1Pm(pmHT1->getCarCostErp() + ht1ParkingRate + (pmHT1->getDistance()*OPERATIONAL_COST));
			todParams.setCostHt1Op(opHT1->getCarCostErp() + ht1ParkingRate + (opHT1->getDistance()*OPERATIONAL_COST));
			todParams.setCostHt2Am(amHT2->getCarCostErp() + ht2ParkingRate + (amHT2->getDistance()*OPERATIONAL_COST));
			todParams.setCostHt2Pm(pmHT2->getCarCostErp() + ht2ParkingRate + (pmHT2->getDistance()*OPERATIONAL_COST));
			todParams.setCostHt2Op(opHT2->getCarCostErp() + ht2ParkingRate + (opHT2->getDistance()*OPERATIONAL_COST));
			break;
		}
		case 5:
		{	// share2
			double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
			todParams.setCostHt1Am((amHT1->getCarCostErp() + ht1ParkingRate + (amHT1->getDistance()*OPERATIONAL_COST))/2.0);
			todParams.setCostHt1Pm((pmHT1->getCarCostErp() + ht1ParkingRate + (pmHT1->getDistance()*OPERATIONAL_COST))/2.0);
			todParams.setCostHt1Op((opHT1->getCarCostErp() + ht1ParkingRate + (opHT1->getDistance()*OPERATIONAL_COST))/2.0);
			todParams.setCostHt2Am((amHT2->getCarCostErp() + ht2ParkingRate + (amHT2->getDistance()*OPERATIONAL_COST))/2.0);
			todParams.setCostHt2Pm((pmHT2->getCarCostErp() + ht2ParkingRate + (pmHT2->getDistance()*OPERATIONAL_COST))/2.0);
			todParams.setCostHt2Op((opHT2->getCarCostErp() + ht2ParkingRate + (opHT2->getDistance()*OPERATIONAL_COST))/2.0);
			break;
		}
		case 6:
		{	// share3
			double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
			todParams.setCostHt1Am((amHT1->getCarCostErp() + ht1ParkingRate + (amHT1->getDistance()*OPERATIONAL_COST))/3.0);
			todParams.setCostHt1Pm((pmHT1->getCarCostErp() + ht1ParkingRate + (pmHT1->getDistance()*OPERATIONAL_COST))/3.0);
			todParams.setCostHt1Op((opHT1->getCarCostErp() + ht1ParkingRate + (opHT1->getDistance()*OPERATIONAL_COST))/3.0);
			todParams.setCostHt2Am((amHT2->getCarCostErp() + ht2ParkingRate + (amHT2->getDistance()*OPERATIONAL_COST))/3.0);
			todParams.setCostHt2Pm((pmHT2->getCarCostErp() + ht2ParkingRate + (pmHT2->getDistance()*OPERATIONAL_COST))/3.0);
			todParams.setCostHt2Op((opHT2->getCarCostErp() + ht2ParkingRate + (opHT2->getDistance()*OPERATIONAL_COST))/3.0);
			break;
		}
		case 7:
		{	//motorcycle
			double ht1ParkingRate = zoneMap.at(zoneIdLookup.at(primaryStopLoc))->getParkingRate();
			double ht2ParkingRate = zoneMap.at(zoneIdLookup.at(home))->getParkingRate();
			todParams.setCostHt1Am(((amHT1->getCarCostErp() + (amHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt1Pm(((pmHT1->getCarCostErp() + (pmHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt1Op(((opHT1->getCarCostErp() + (opHT1->getDistance()*OPERATIONAL_COST))*0.5) + (ht1ParkingRate*0.65));
			todParams.setCostHt2Am(((amHT2->getCarCostErp() + (amHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			todParams.setCostHt2Pm(((pmHT2->getCarCostErp() + (pmHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			todParams.setCostHt2Op(((opHT2->getCarCostErp() + (opHT2->getDistance()*OPERATIONAL_COST))*0.5) + (ht2ParkingRate*0.65));
			break;
		}
		case 8:
		{	//Walk
			todParams.setCostHt1Am(0);
			todParams.setCostHt1Pm(0);
			todParams.setCostHt1Op(0);
			todParams.setCostHt2Am(0);
			todParams.setCostHt2Pm(0);
			todParams.setCostHt2Op(0);
			break;
		}
		case 9:
		{	//Taxi
			const ZoneParams* homeZoneParams = zoneMap.at(zoneIdLookup.at(home));
			const ZoneParams* destZoneParams = zoneMap.at(zoneIdLookup.at(primaryStopLoc));
			double amHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ amHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((amHT1->getDistance()<=10)? amHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((amHT1->getDistance()<=10)? 0 : (amHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double pmHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ pmHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((pmHT1->getDistance()<=10)? pmHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((pmHT1->getDistance()<=10)? 0 : (pmHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double opHT1Cost = TAXI_FLAG_DOWN_PRICE
							+ opHT1->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * homeZoneParams->getCentralDummy())
							+ (((opHT1->getDistance()<=10)? opHT1->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((opHT1->getDistance()<=10)? 0 : (opHT1->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double amHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ amHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((amHT2->getDistance()<=10)? amHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((amHT2->getDistance()<=10)? 0 : (amHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double pmHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ pmHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((pmHT2->getDistance()<=10)? pmHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((pmHT2->getDistance()<=10)? 0 : (pmHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			double opHT2Cost = TAXI_FLAG_DOWN_PRICE
							+ opHT2->getCarCostErp()
							+ (TAXI_CENTRAL_LOCATION_SURCHARGE * destZoneParams->getCentralDummy())
							+ (((opHT2->getDistance()<=10)? opHT2->getDistance() : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
							+ (((opHT2->getDistance()<=10)? 0 : (opHT2->getDistance()-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE;
			todParams.setCostHt1Am(amHT1Cost);
			todParams.setCostHt1Pm(pmHT1Cost);
			todParams.setCostHt1Op(opHT1Cost);
			todParams.setCostHt2Am(amHT2Cost);
			todParams.setCostHt2Pm(pmHT2Cost);
			todParams.setCostHt2Op(opHT2Cost);
			break;
		}
		}
	}
	else
	{
		todParams.setCostHt1Am(0);
		todParams.setCostHt1Pm(0);
		todParams.setCostHt1Op(0);
		todParams.setCostHt2Am(0);
		todParams.setCostHt2Pm(0);
		todParams.setCostHt2Op(0);
	}

	timeWndw = PredayLuaProvider::getPredayModel().predictTourTimeOfDay(personParams, todParams, tour.getTourType());
	if(timeWndw == -1)
	{
		return TimeWindowAvailability();
	}
	return TimeWindowAvailability::timeWindowsLookup.at(timeWndw - 1); //timeWndw ranges from 1 - 1176. Vector starts from 0.
}

void PredaySystem::constructIntermediateStops(Tour& tour, size_t remainingTours, double prevTourEndTime)
{
	//sanity check
	if(tour.stops.size() != 1)
	{
		stringstream errStrm;
		errStrm << "generateIntermediateStops()|tour contains " << tour.stops.size() << " stops. Exactly 1 stop (primary activity) was expected.";
		throw runtime_error(errStrm.str());
	}
	//return trivially if possible
	if ((dayPattern.at("WorkI") + dayPattern.at("EduI") + dayPattern.at("ShopI") + dayPattern.at("OthersI")) <= 0 ) { return; } //No stops

	Stop* primaryStop = tour.stops.front(); // The only stop at this point is the primary activity stop

	//generate all intermediate stops
	generateIntermediateStops(FIRST_HALF_TOUR, tour, primaryStop, remainingTours);
	generateIntermediateStops(SECOND_HALF_TOUR, tour, primaryStop, remainingTours);

	StopList& stops = tour.stops;
	if(stops.size() == 1) { return; } //No stops were generated
	StopList::iterator primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop);
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//predict mode and destinations for stops
	//first half tour
	if(primaryStopIt!=stops.begin()) // otherwise, there is no stop in the first half tour
	{
		// mode/destination is predicted for each stop in reverse chronological order from primary stop location toward home location in first half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator firstStopIt = stops.begin();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		bool stopModeDestSuccessful = false;
		do
		{
			--stopIt;
			stopModeDestSuccessful = predictStopModeDestination(*stopIt, currStop->getStopLocation());
			if(!stopModeDestSuccessful) { break; }
			currStop = *stopIt;
		}
		while(stopIt!=firstStopIt);

		if(!stopModeDestSuccessful)
		{
			unsigned short numRemoved = 0;
			++stopIt; // now stopIT points to the last valid stop
			for(StopList::iterator eraseIt=stops.begin(); eraseIt!=stopIt; )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 1st HT (IMD issue) in "
					 << tour.getTourTypeStr() << " tour with mode: " << tour.getTourMode()
					 << " and dest: " << tour.getTourDestination();
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//second half tour
	if(primaryStopIt!=(--stops.end())) // otherwise, there is no stop in the second half tour
	{
		// mode/destination is predicted for each stop in chronological order from primary stop location toward home location in second half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator lastStopIt = --stops.end();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		bool stopModeDestSuccessful = false;
		do
		{
			++stopIt;
			stopModeDestSuccessful = predictStopModeDestination(*stopIt, currStop->getStopLocation());
			if(!stopModeDestSuccessful) { break; }
			currStop = *stopIt;
		}
		while(stopIt!=lastStopIt);

		if(!stopModeDestSuccessful)
		{
			unsigned short numRemoved = 0;
			for(StopList::iterator eraseIt=stopIt; eraseIt!=stops.end(); )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 2nd HT (IMD issue) in "
					 << tour.getTourTypeStr() << " tour with mode: " << tour.getTourMode()
					 << " and dest: " << tour.getTourDestination();
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//predict time of day
	//first half tour
	if(primaryStopIt!=stops.begin()) // otherwise, there is no stop in the first half tour
	{
		// time of day is predicted for each stop in reverse chronological order from primary stop location toward home location in first half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator firstStopIt = stops.begin();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		Stop* prevStop = nullptr;
		int destLocation = 0;
		bool stopTodSuccessful = false;
		do
		{
			--stopIt;
			prevStop = *stopIt; // since we go in reverse chronological order, we predict arrival time to the stop chronologically before currStop
			// person will arrive at current stop from the next stop (chronologically)
			calculateDepartureTime(currStop, prevStop, prevTourEndTime);
			if(stopIt==firstStopIt) { destLocation = personParams.getHomeLocation(); }
			else
			{
				--stopIt;
				destLocation = (*stopIt)->getStopLocation(); //we have predicted the location for all stops already.
				++stopIt; //get back
			}
			stopTodSuccessful = predictStopTimeOfDay(prevStop, destLocation, true);  //predict arrival time for nextStop
			if(!stopTodSuccessful) { break; } //break off here if prediction was unsuccessful. This stop and remaining stops are to be deleted.
			currStop = prevStop;
		}
		while(stopIt!=firstStopIt);
		if(!stopTodSuccessful)
		{
			unsigned short numRemoved = 0;
			++stopIt; // now stopIT points to the last valid stop
			for(StopList::iterator eraseIt=stops.begin(); eraseIt!=stopIt; )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 1st HT (TOD issue)";
		}
	}

	primaryStopIt = std::find(stops.begin(), stops.end(), primaryStop); //find primary stop again
	if(primaryStopIt==stops.end()) { throw std::runtime_error("primary stop missing stops list"); }

	//second half tour
	if(primaryStopIt!=(--stops.end())) // otherwise, there is no stop in the second half tour
	{
		// time of day is predicted for each stop in chronological order from primary stop location toward home location in second half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator lastStopIt = --stops.end();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		Stop* nextStop = nullptr;
		int destLocation = 0;
		bool stopTodSuccessful = false;
		do
		{
			++stopIt;
			nextStop = *stopIt; // since we go in chronological order, we predict departure time for the stop chronologically after currStop
			// person will arrive at next stop from the current stop (chronologically)
			calculateArrivalTime(currStop, nextStop);
			if(stopIt==lastStopIt) { destLocation = personParams.getHomeLocation(); }
			else
			{
				++stopIt;
				destLocation = (*stopIt)->getStopLocation(); //we have predicted the location for all stops already.
				--stopIt; //get back
			}
			stopTodSuccessful = predictStopTimeOfDay(nextStop, destLocation, false); //predict departure time for nextStop
			if(!stopTodSuccessful) { break; } //break off here if prediction was unsuccessful. This stop and remaining stops are to be deleted.
			currStop = nextStop;
		}
		while(stopIt!=lastStopIt);
		if(!stopTodSuccessful)
		{
			unsigned short numRemoved = 0;
			for(StopList::iterator eraseIt=stopIt; eraseIt!=stops.end(); )
			{
				safe_delete_item(*eraseIt);
				eraseIt = stops.erase(eraseIt);
				numRemoved++;
			}
			logStream << "| Removed " << numRemoved << " stops in 2nd HT (TOD issue)";
		}
	}
}

void PredaySystem::generateIntermediateStops(uint8_t halfTour, Tour& tour, const Stop* primaryStop, size_t remainingTours)
{
	if(!primaryStop) { return; } // cannot generate intermediate stops without primary activity
	if(halfTour != FIRST_HALF_TOUR && halfTour != SECOND_HALF_TOUR) { throw std::runtime_error("invalid value for halfTour"); }

	//initialize isgParams
	int origin = personParams.getHomeLocation();
	int destination = primaryStop->getStopLocation();
	StopGenerationParams isgParams(tour, primaryStop, dayPattern);
	isgParams.setFirstHalfTour((halfTour==FIRST_HALF_TOUR));
	isgParams.setNumRemainingTours(remainingTours);
	if(origin == destination) { isgParams.setDistance(0.0); }
	else
	{
		switch(halfTour)
		{
		case FIRST_HALF_TOUR:
		{	//first half tour
			// use AM costs for first half tour
			CostParams* amDistanceObj = amCostMap.at(destination).at(origin); //TODO: check with Siyu
			isgParams.setDistance(amDistanceObj->getDistance());
			break;
		}
		case SECOND_HALF_TOUR:
		{
			// use PM costs for first half tour
			CostParams* pmDistanceObj = pmCostMap.at(destination).at(origin); //TODO: check with Siyu
			isgParams.setDistance(pmDistanceObj->getDistance());
			break;
		}
		}
	}

	switch(halfTour)
	{
	case FIRST_HALF_TOUR:
	{
		double startTime = FIRST_INDEX;
		if(!tour.isFirstTour())
		{
			TourList::iterator currTourIt = std::find(tours.begin(), tours.end(), tour);
			const Tour& prevTour = *(--currTourIt);
			startTime = prevTour.getEndTime();
		}
		double endTime = primaryStop->getArrivalTime();
		if(startTime > endTime)
		{
			std::stringstream ss;
			ss << "start time is greater than end time; FIRST HT: start-" << startTime << " end-" << endTime << std::endl;
			throw std::runtime_error(ss.str());
		}
		isgParams.setTimeWindowFirstBound((endTime - startTime + 1)/2); //HOURS
		isgParams.setTimeWindowSecondBound(0);
		break;
	}
	case SECOND_HALF_TOUR:
	{
		double startTime = primaryStop->getDepartureTime();
		double endTime = LAST_INDEX;
		if(startTime > endTime)
		{
			std::stringstream ss;
			ss << "start time is greater than end time; SECOND HT: start-" << startTime << " end-" << endTime << std::endl;
			throw std::runtime_error(ss.str());
		}
		isgParams.setTimeWindowFirstBound(0);
		isgParams.setTimeWindowSecondBound((endTime - startTime + 1)/2); //HOURS
		break;
	}
	}

	int choice = 0;
	Stop* generatedStop = nullptr;
	for(uint8_t stopCounter=0; stopCounter<MAX_STOPS_IN_HALF_TOUR; stopCounter++)
	{
		isgParams.setNumPreviousStops(stopCounter);
		choice = PredayLuaProvider::getPredayModel().generateIntermediateStop(personParams, isgParams);
		if(choice == QUIT_CHOICE_ISG) { break; } //Quit
		switch(choice)
		{
		case WORK_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::WORK, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case EDU_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::EDUCATION, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case SHOP_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::SHOP, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case OTHER_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::OTHER, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		default:
			throw std::runtime_error("invalid choice predicted by ISG model");
		}
	}
}

bool PredaySystem::predictStopModeDestination(Stop* stop, int origin)
{
	StopModeDestinationParams imdParams(zoneMap, amCostMap, pmCostMap, personParams, stop, origin, unavailableODs, MTZ12_MTZ08_Map);
	imdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin))->getCbdDummy());
	int modeDest = PredayLuaProvider::getPredayModel().predictStopModeDestination(personParams, imdParams);
	if(modeDest == -1)
	{
		// in theory, it is possible to have a scenario where the availability of all alternatives was set to zero
		// if that happened, all probabilities would be zero and the lua would return -1 to indicate an error
		return false;
	}
	stop->setStopMode(imdParams.getMode(modeDest));
	int zone_id = imdParams.getDestination(modeDest);
	stop->setStopLocationId(zone_id);
	stop->setStopLocation(zoneMap.at(zone_id)->getZoneCode());
	return true;
}

bool PredaySystem::predictStopTimeOfDay(Stop* stop, int destination_2012, bool isBeforePrimary)
{
	if(!stop) { throw std::runtime_error("predictStopTimeOfDay() - stop is null"); }
	StopTimeOfDayParams stodParams(stop->getStopTypeID(), isBeforePrimary);
	int origin_2012 = stop->getStopLocation();
	int origin = MTZ12_MTZ08_Map.at(origin_2012);
	int destination = MTZ12_MTZ08_Map.at(destination_2012);
	stodParams.setStopMode(stop->getStopMode());
	stodParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(origin_2012))->getCbdDummy());
	stodParams.setCbdDestZone(zoneMap.at(zoneIdLookup.at(destination_2012))->getCbdDummy());

	if(origin_2012 == destination_2012 || origin == destination)
	{
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++) { stodParams.travelTimes.push_back(0.0); }
	}
	else
	{
		BSONObj bsonObjTT = BSON("origin" << origin << "destination" << destination);
		BSONObj tCostBusDoc, tCostCarDoc;
		mongoDao["tcost_bus"]->getOne(bsonObjTT, tCostBusDoc);
		mongoDao["tcost_car"]->getOne(bsonObjTT, tCostCarDoc);
		std::stringstream fieldName;

		for (uint32_t i = FIRST_INDEX; i <= LAST_INDEX; i++)
		{
			switch (stop->getStopMode())
			{
			case 1: // Fall through
			case 2:
			case 3:
			{

				if(stodParams.getFirstBound()) { fieldName << "TT_bus_arrival_" << i; }
				else { fieldName << "TT_bus_departure_" << i; }
				if(tCostBusDoc.getField(fieldName.str()).isNumber()) { stodParams.travelTimes.push_back(tCostBusDoc.getField(fieldName.str()).Number()); }
				else { stodParams.travelTimes.push_back(HIGH_TRAVEL_TIME); }
				fieldName.str(std::string());
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				if(stodParams.getFirstBound()) { fieldName << "TT_car_arrival_" << i; }
				else { fieldName << "TT_car_departure_" << i; }
				if(tCostCarDoc.getField(fieldName.str()).isNumber()) { stodParams.travelTimes.push_back(tCostCarDoc.getField(fieldName.str()).Number()); }
				else { stodParams.travelTimes.push_back(HIGH_TRAVEL_TIME); }
				fieldName.str(std::string());
				break;
			}
			case 8:
			{
				double travelTime = 0.0;
				if(i>=AM_PEAK_LOW && i<=AM_PEAK_HIGH) /*if i is in AM peak period*/
				{ travelTime = amCostMap.at(origin_2012).at(destination_2012)->getDistance()/PEDESTRIAN_WALK_SPEED; }
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{ travelTime = pmCostMap.at(origin_2012).at(destination_2012)->getDistance()/PEDESTRIAN_WALK_SPEED; }
				else // if i is in off-peak period
				{ travelTime = opCostMap.at(origin_2012).at(destination_2012)->getDistance()/PEDESTRIAN_WALK_SPEED; }
				stodParams.travelTimes.push_back(travelTime);
				break;
			}
			}
		}
	}

	// high and low tod
	if(isBeforePrimary)
	{
		stodParams.setTodHigh(stop->getDepartureTime());
		stodParams.setTodLow(firstAvailableTimeIndex);
	}
	else
	{
		stodParams.setTodLow(stop->getArrivalTime());
		stodParams.setTodHigh(LAST_INDEX); // end of day
	}

	if(stodParams.getTodHigh() < stodParams.getTodLow()) { return false; } //Invalid low and high TODs for stop

	ZoneParams* zoneDoc = zoneMap.at(zoneIdLookup.at(origin_2012));
	if(origin_2012 != destination_2012)
	{
		// calculate costs
		CostParams* amDoc = amCostMap.at(origin_2012).at(destination_2012);
		CostParams* pmDoc = pmCostMap.at(origin_2012).at(destination_2012);
		CostParams* opDoc = opCostMap.at(origin_2012).at(destination_2012);
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++)
		{
			if(stodParams.getFirstBound()) { duration = stodParams.getTodHigh() - i + 1; }
			else { duration = i - stodParams.getTodLow() + 1; }
			duration = 0.25+(duration-1)*0.5;
			parkingRate = zoneDoc->getParkingRate();
			costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

			if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) // time window indexes 10 to 14 are AM Peak windows
			{
				costCarERP = amDoc->getCarCostErp();
				costCarOP = amDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = amDoc->getDistance();
			}
			else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) // time window indexes 30 to 34 are PM Peak indexes
			{
				costCarERP = pmDoc->getCarCostErp();
				costCarOP = pmDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = pmDoc->getDistance();
			}
			else // other time window indexes are Off Peak indexes
			{
				costCarERP = opDoc->getCarCostErp();
				costCarOP = opDoc->getDistance() * OPERATIONAL_COST;
				walkDistance = opDoc->getDistance();
			}

			switch (stop->getStopMode())
			{
			case 1: // Fall through
			case 2:
			case 3:
			{
				if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) { stodParams.travelCost.push_back(amDoc->getPubCost()); }
				else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) { stodParams.travelCost.push_back(pmDoc->getPubCost()); }
				else { stodParams.travelCost.push_back(opDoc->getPubCost()); }
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			{
				stodParams.travelCost.push_back((costCarParking+costCarOP+costCarERP)/(stop->getStopMode()-3.0));
				break;
			}
			case 7:
			{
				stodParams.travelCost.push_back((0.5*costCarERP+0.5*costCarOP+0.65*costCarParking)); // these constants 0.5 and 0.65 are from LTA
				break;
			}
			case 9:
			{
				stodParams.travelCost.push_back(
						TAXI_FLAG_DOWN_PRICE
						+ costCarERP
						+ (TAXI_CENTRAL_LOCATION_SURCHARGE * zoneDoc->getCentralDummy())
						+ (((walkDistance<=10)? walkDistance : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
						+ (((walkDistance<=10)? 0 : (walkDistance-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE
						);
				break;
			}
			case 8:
			{
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}
	else // if origin and destination are same
	{
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=FIRST_INDEX; i<=LAST_INDEX; i++)
		{
			if(stodParams.getFirstBound()) { duration = stodParams.getTodHigh() - i + 1; }
			else { duration = i - stodParams.getTodLow() + 1; }
			duration = 0.25+(duration-1)*0.5;
			parkingRate = zoneDoc->getParkingRate();
			costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

			costCarERP = 0;
			costCarOP = 0;
			walkDistance = 0;

			switch (stop->getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				if(i >= AM_PEAK_LOW && i <= AM_PEAK_HIGH) { stodParams.travelCost.push_back(0); }
				else if (i >= PM_PEAK_LOW && i <= PM_PEAK_HIGH) { stodParams.travelCost.push_back(0); }
				else { stodParams.travelCost.push_back(0); }
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			{
				stodParams.travelCost.push_back((costCarParking+costCarOP+costCarERP)/(stop->getStopMode()-3.0));
				break;
			}
			case 7:
			{
				stodParams.travelCost.push_back((0.5*costCarERP+0.5*costCarOP+0.65*costCarParking));
				break;
			}
			case 9:
			{
				stodParams.travelCost.push_back(
						TAXI_FLAG_DOWN_PRICE
						+ costCarERP
						+ TAXI_CENTRAL_LOCATION_SURCHARGE*zoneDoc->getCentralDummy()
						+ (((walkDistance<=10)? walkDistance : 10)/UNIT_FOR_FIRST_10KM) * TAXI_UNIT_PRICE
						+ (((walkDistance<=10)? 0 : (walkDistance-10))/UNIT_AFTER_10KM) * TAXI_UNIT_PRICE
						);
				break;
			}
			case 8: {
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}

	stodParams.updateAvailabilities();

	int timeWindowIdx = PredayLuaProvider::getPredayModel().predictStopTimeOfDay(personParams, stodParams);
	if(timeWindowIdx == -1)
	{
		return false;
	}
	if(isBeforePrimary) {
		if(timeWindowIdx > stop->getDepartureTime()) { return false; } // Predicted arrival time must not be greater than the estimated departure time
		stop->setArrivalTime(timeWindowIdx);
	}
	else {
		if(timeWindowIdx < stop->getArrivalTime()) { return false; } //Predicted departure time must not be lesser than the estimated arrival time
		stop->setDepartureTime(timeWindowIdx);
	}
	return true;
}

double PredaySystem::fetchTravelTime(int origin_2012, int destination_2012, int mode,  bool isArrivalBased, double timeIdx)
{
	int origin = MTZ12_MTZ08_Map.at(origin_2012);
	int destination = MTZ12_MTZ08_Map.at(destination_2012);
	double travelTime = 0.0;
	if(origin != destination)
	{
		switch(mode) {
		case 1: // Fall through
		case 2:
		case 3:
		{
			BSONObj bsonObj = BSON("origin" << origin << "destination" << destination);
			BSONObj tCostBusDoc;
			mongoDao["tcost_bus"]->getOne(bsonObj, tCostBusDoc);
			std::stringstream fieldName;
			fieldName << (isArrivalBased? "TT_bus_arrival_":"TT_bus_departure_") << timeIdx;
			if(tCostBusDoc.getField(fieldName.str()).isNumber()) {
				travelTime = tCostBusDoc.getField(fieldName.str()).Number();
			}
			break;
		}
		case 4: // Fall through
		case 5:
		case 6:
		case 7:
		case 9:
		{
			BSONObj bsonObj = BSON("origin" << origin << "destination" << destination);
			BSONObj tCostCarDoc;
			mongoDao["tcost_car"]->getOne(bsonObj, tCostCarDoc);
			std::stringstream fieldName;
			fieldName << (isArrivalBased? "TT_car_arrival_":"TT_car_departure_") << timeIdx;
			if(tCostCarDoc.getField(fieldName.str()).isNumber()) {
				travelTime = tCostCarDoc.getField(fieldName.str()).Number();
			}
			break;
		}
		case 8:
		{
			if(timeIdx>=AM_PEAK_LOW && timeIdx<=AM_PEAK_HIGH) // if i is in AM peak period
			{
				CostParams* amDistanceObj = amCostMap.at(origin_2012).at(destination_2012);
				travelTime = amDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			}
			else if(timeIdx>=PM_PEAK_LOW && timeIdx<=PM_PEAK_HIGH) // if i is in PM peak period
			{
				CostParams* pmDistanceObj = pmCostMap.at(origin_2012).at(destination_2012);
				travelTime = pmDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			}
			else // if i is in off-peak period
			{
				CostParams* opDistanceObj = opCostMap.at(origin_2012).at(destination_2012);
				travelTime = opDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			}
			break;
		}
		default:
		{
			throw std::runtime_error("invalid mode");
		}
		}
	}
	return travelTime;
}

void PredaySystem::calculateArrivalTime(Stop* currStop,  Stop* nextStop) {
	// person will arrive at the next stop from the current stop
	// this function sets the arrival time for next stop
	double currActivityDepartureIndex = currStop->getDepartureTime();
	double timeWindow = getTimeWindowFromIndex(currActivityDepartureIndex);
	double travelTime = fetchTravelTime(currStop->getStopLocation(), nextStop->getStopLocation(), nextStop->getStopMode(), false, currActivityDepartureIndex);
	double nextStopArrTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	nextStopArrTime = alignTime(nextStopArrTime, timeWindow, LAST_WINDOW, personParams.getPersonId(), "calculateArrivalTime()");
	nextStopArrTime = getIndexFromTimeWindow(nextStopArrTime);
	nextStop->setArrivalTime(nextStopArrTime);
}

void PredaySystem::calculateDepartureTime(Stop* currStop,  Stop* prevStop, double prevTourEndTimeIdx) {
	// person will arrive at the current stop from the previous stop
	// this function sets the departure time for the prevStop
	double currActivityArrivalIndex = currStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(currActivityArrivalIndex);
	double travelTime = fetchTravelTime(currStop->getStopLocation(), prevStop->getStopLocation(), currStop->getStopMode(), true, currActivityArrivalIndex);
	double prevStopDepTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	prevStopDepTime = alignTime(prevStopDepTime, getTimeWindowFromIndex(prevTourEndTimeIdx), timeWindow, personParams.getPersonId(), "calculateDepartureTime()");
	prevStopDepTime = getIndexFromTimeWindow(prevStopDepTime);
	prevStop->setDepartureTime(prevStopDepTime);
}

void PredaySystem::blockTravelTimeToSubTourLocation(const Tour& subTour, const Tour& parentTour, SubTourParams& stParams)
{
	double tourPrimArrivalIdx = parentTour.getPrimaryStop()->getArrivalTime();
	double tourPrimDepartureIdx = parentTour.getPrimaryStop()->getDepartureTime();
	double tourPrimArrivalWindow = getTimeWindowFromIndex(tourPrimArrivalIdx);
	double tourPrimDepartureWindow = getTimeWindowFromIndex(tourPrimDepartureIdx);

	//get travel time from parentTour destination to subTour destination and block that time
	double travelTime = fetchTravelTime(parentTour.getTourDestination(), subTour.getTourDestination(), subTour.getTourMode(), false, tourPrimArrivalIdx);
	double firstPossibleArrTimeWindow = tourPrimArrivalWindow + travelTime; //first possible arrival time window to sub-tour location
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	firstPossibleArrTimeWindow = alignTime(firstPossibleArrTimeWindow, tourPrimArrivalWindow, tourPrimDepartureWindow, personParams.getPersonId(), "blockTravelTimeToSubTourLocation() - arr");
	stParams.blockTime(tourPrimArrivalIdx, getIndexFromTimeWindow(firstPossibleArrTimeWindow));

	//get travel time from subTour destination to parentTour destination and block that time
	travelTime = fetchTravelTime(subTour.getTourDestination(), parentTour.getTourDestination(), subTour.getTourMode(), true, tourPrimDepartureIdx);
	double lastPossibleDepTimeWindow = tourPrimDepartureWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	lastPossibleDepTimeWindow = alignTime(lastPossibleDepTimeWindow, firstPossibleArrTimeWindow, tourPrimDepartureWindow, personParams.getPersonId(), "blockTravelTimeToSubTourLocation() - dep");
	stParams.blockTime(getIndexFromTimeWindow(lastPossibleDepTimeWindow), tourPrimDepartureIdx);
}

void PredaySystem::calculateSubTourTimeWindow(Tour& subTour, const Tour& parentTour)
{
	const Stop* primaryStop = subTour.getPrimaryStop();
	// estimate tour start time
	double activityArrivalIndex = primaryStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(activityArrivalIndex);
	double travelTime = fetchTravelTime(primaryStop->getStopLocation(), parentTour.getTourDestination(), subTour.getTourMode(), true, activityArrivalIndex);
	double tourStartTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourStartTime = alignTime(tourStartTime, getTimeWindowFromIndex(parentTour.getPrimaryStop()->getArrivalTime()), timeWindow, personParams.getPersonId(), "calculateSubTourTimeWindow() - start");
	tourStartTime = getIndexFromTimeWindow(tourStartTime);
	subTour.setStartTime(tourStartTime);

	//estimate tour end time
	double activityDepartureIndex = primaryStop->getDepartureTime();
	timeWindow = getTimeWindowFromIndex(activityDepartureIndex);
	travelTime = fetchTravelTime(primaryStop->getStopLocation(), parentTour.getTourDestination(), subTour.getTourMode(), false, activityDepartureIndex);
	double tourEndTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourEndTime = alignTime(tourEndTime, timeWindow, getTimeWindowFromIndex(parentTour.getPrimaryStop()->getDepartureTime()), personParams.getPersonId(), "calculateSubTourTimeWindow() - end");
	tourEndTime = getIndexFromTimeWindow(tourEndTime);
	subTour.setEndTime(tourEndTime);
}

void PredaySystem::calculateTourStartTime(Tour& tour, double lowerBoundIdx)
{
	Stop* firstStop = tour.stops.front();
	double firstActivityArrivalIndex = firstStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(firstActivityArrivalIndex);
	double travelTime = fetchTravelTime(personParams.getHomeLocation(), firstStop->getStopLocation(), firstStop->getStopMode(), true, firstActivityArrivalIndex);
	double tourStartTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourStartTime = alignTime(tourStartTime, getTimeWindowFromIndex(lowerBoundIdx), timeWindow, personParams.getPersonId(), "calculateTourStartTime()");
	tourStartTime = getIndexFromTimeWindow(tourStartTime);
	tour.setStartTime(tourStartTime);
}

void PredaySystem::calculateTourEndTime(Tour& tour)
{
	Stop* lastStop = tour.stops.back();
	double lastActivityDepartureIndex = lastStop->getDepartureTime();
	double timeWindow = getTimeWindowFromIndex(lastActivityDepartureIndex);
	double travelTime = fetchTravelTime(lastStop->getStopLocation(), personParams.getHomeLocation(), tour.getTourMode(), false, lastActivityDepartureIndex);
	double tourEndTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourEndTime = alignTime(tourEndTime, timeWindow, LAST_WINDOW, personParams.getPersonId(), "calculateTourEndTime()");
	tourEndTime = getIndexFromTimeWindow(tourEndTime);
	tour.setEndTime(tourEndTime);
}

void PredaySystem::constructTours() {
	if(numTours.size() != 4) {
		// Probably predictNumTours() was not called prior to this function
		throw std::runtime_error("Tours cannot be constructed before predicting number of tours for each tour type");
	}

	//Construct work tours
	bool firstOfMultiple = true;
	for(int i=0; i<numTours["WorkT"]; i++) {
		bool attendsUsualWorkLocation = false;
		if(!(personParams.isStudent() == 1) && (personParams.getFixedWorkLocation() != 0)) {
			//if person not a student and has a fixed work location
			attendsUsualWorkLocation = predictUsualWorkLocation(firstOfMultiple); // Predict if this tour is to a usual work location
			firstOfMultiple = false;
		}
		Tour workTour(WORK);
		workTour.setUsualLocation(attendsUsualWorkLocation);
		if(attendsUsualWorkLocation) {
			workTour.setTourDestination(personParams.getFixedWorkLocation());
		}
		tours.push_back(workTour);
	}

	// Construct education tours
	for(int i=0; i<numTours["EduT"]; i++) {
		Tour eduTour(EDUCATION);
		eduTour.setUsualLocation(true); // Education tours are always to usual locations
		eduTour.setTourDestination(personParams.getFixedSchoolLocation());
		if(personParams.isStudent()) {
			// if the person is a student, his education tours should be processed before other tour types.
			tours.push_front(eduTour); // if the person is a student, his education tours should be processed before other tour types.
		}
		else {
			tours.push_back(eduTour);
		}
	}

	// Construct shopping tours
	for(int i=0; i<numTours["ShopT"]; i++) {
		Tour shopTour(SHOP);
		tours.push_back(shopTour);
	}

	// Construct other tours
	for(int i=0; i<numTours["OthersT"]; i++) {
		Tour otherTour(OTHER);
		tours.push_back(otherTour);
	}
}

void PredaySystem::planDay()
{
	personParams.initTimeWindows();

	//Predict day pattern
	logStream << "Person: " << personParams.getPersonId() << "| home: " << personParams.getHomeLocation();
	logStream << "| Day Pattern: ";
	PredayLuaProvider::getPredayModel().predictDayPattern(personParams, dayPattern);
	if (dayPattern.empty())
	{
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
	}
	logStream << dayPattern["WorkT"] << dayPattern["EduT"] << dayPattern["ShopT"] << dayPattern["OthersT"] << dayPattern["WorkI"] << dayPattern["EduI"]
			<< dayPattern["ShopI"] << dayPattern["OthersI"];

	//Predict number of Tours
	logStream << "| Num. Tours: ";
	PredayLuaProvider::getPredayModel().predictNumTours(personParams, dayPattern, numTours);
	logStream << numTours["WorkT"] << numTours["EduT"] << numTours["ShopT"] << numTours["OthersT"];

	//Construct tours.
	constructTours();
	if (!tours.empty())
	{
		tours.front().setFirstTour(true);
	} // make first tour aware that it is the first tour for person

	//Process each tour
	size_t remainingTours = tours.size();
	TourList::iterator tourIt = tours.begin();
	bool toursConstructed = true;
	for (; tourIt != tours.end(); tourIt++)
	{
		Tour& tour = *tourIt;
		remainingTours = remainingTours - 1; // 1 less tours to be processed after current tour
		if (tour.isUsualLocation())
		{
			// Predict just the mode for tours to usual location
			predictTourMode(tour);
		}
		else
		{
			// Predict mode and destination for tours to not-usual locations
			predictTourModeDestination(tour);
		}

		int tourMode = tour.getTourMode();
		if(tourMode == 1 || tourMode == 2) //if chosen mode for primary activity was a PT mode
		{
			//block early morning and late night hours where PT trips are rare
			if(firstAvailableTimeIndex < FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE)
			{
				personParams.blockTime(firstAvailableTimeIndex, FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE);
				firstAvailableTimeIndex = FIRST_INDEX_FOR_PUBLIC_TANSIT_MODE;
			}
		}

		// Predict time of day for this tour
		TimeWindowAvailability timeWindow = predictTourTimeOfDay(tour);
		if(timeWindow.getStartTime() == 0 && timeWindow.getEndTime() == 0)
		{
			toursConstructed = false;
			logStream << "| (no-time) mode: " << tour.getTourMode() << ", dest: " << tour.getTourDestination();
			break;
		}

		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true /*primary activity*/, true /*stop in first half tour*/);
		primaryActivity->setStopMode(tour.getTourMode());
		primaryActivity->setStopLocation(tour.getTourDestination());
		primaryActivity->setStopLocationId(zoneIdLookup.at(tour.getTourDestination()));
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		tour.setPrimaryStop(primaryActivity);
		tour.addStop(primaryActivity);
		personParams.blockTime(timeWindow.getStartTime(), timeWindow.getEndTime());

		//Generate sub tours for work tours
		if (tour.getTourType() == sim_mob::WORK)
		{
			predictSubTours(tour);
		}

		//Generate stops for this tour
		constructIntermediateStops(tour, remainingTours, firstAvailableTimeIndex);

		calculateTourStartTime(tour, firstAvailableTimeIndex);
		calculateTourEndTime(tour);
		personParams.blockTime(firstAvailableTimeIndex, tour.getEndTime());
		firstAvailableTimeIndex = tour.getEndTime();
	}

	if(!toursConstructed)
	{
		short numRemovedTours = 0;
		while(tourIt != tours.end())
		{
			tourIt = tours.erase(tourIt);
			numRemovedTours++;
		}
		logStream << "|" << numRemovedTours << " tours removed due to time in-availability";
	}
	logStream << "\n";
}

void sim_mob::medium::PredaySystem::insertDayPattern()
{
	int tourCount = numTours["WorkT"] + numTours["EduT"] + numTours["ShopT"] + numTours["OthersT"];
	std::stringstream dpStream;
	dpStream << dayPattern["WorkT"] << "," << dayPattern["EduT"] << "," << dayPattern["ShopT"] << "," << dayPattern["OthersT"]
	        << "," << dayPattern["WorkI"] << "," << dayPattern["EduI"] << "," << dayPattern["ShopI"] << "," << dayPattern["OthersI"];
	BSONObj dpDoc = BSON(
					"_id" << personParams.getPersonId() <<
					"num_tours" << tourCount <<
					"day_pattern" << dpStream.str() <<
					"person_type_id" << personParams.getPersonTypeId() <<
					"hhfactor" << personParams.getHouseholdFactor()
					);
	mongoDao["Output_DayPattern"]->insert(dpDoc);
}

void sim_mob::medium::PredaySystem::insertTour(const Tour& tour, int tourNumber) {
	int tourCount = numTours["WorkT"] + numTours["EduT"] + numTours["ShopT"] + numTours["OthersT"];
	BSONObj tourDoc = BSON(
		"person_type_id" << personParams.getPersonTypeId() <<
		"num_stops" << (int)tour.stops.size() <<
		"prim_arr" << tour.getPrimaryStop()->getArrivalTime() <<
		"start_time" << tour.getStartTime() <<
		"destination" << tour.getTourDestination() <<
		"tour_type" << tour.getTourTypeStr() <<
		"num_tours" << tourCount <<
		"prim_dept" << tour.getPrimaryStop()->getDepartureTime() <<
		"end_time" << tour.getEndTime() <<
		"person_id" << personParams.getPersonId() <<
		"tour_mode" << tour.getTourMode() <<
		"tour_num" << tourNumber <<
		"usual_location" << (tour.isUsualLocation()? 1 : 0) <<
		"hhfactor" << personParams.getHouseholdFactor()
	);
	mongoDao["Output_Tour"]->insert(tourDoc);
}

void sim_mob::medium::PredaySystem::insertSubTour(const Tour& subTour, const Tour& parentTour, int tourNumber, int subTourNumber) {
	BSONObj tourDoc = BSON(
		"person_id" << personParams.getPersonId() <<
		"person_type_id" << personParams.getPersonTypeId() <<
		"parent_tour_num" << tourNumber <<
		"parent_tour_destination" << parentTour.getTourDestination() <<
		"parent_tour_mode" << parentTour.getTourMode() <<
		"parent_activity_arr" << parentTour.getPrimaryStop()->getArrivalTime() <<
		"parent_activity_dep" << parentTour.getPrimaryStop()->getDepartureTime() <<
		"sub_tour_type" << subTour.getTourTypeStr() <<
		"sub_tour_num" << subTourNumber <<
		"mode" << subTour.getTourMode() <<
		"destination" << subTour.getTourDestination() <<
		"sub_tour_activity_arr" << subTour.getPrimaryStop()->getArrivalTime() <<
		"sub_tour_activity_dep" << subTour.getPrimaryStop()->getDepartureTime() <<
		"start_time" << subTour.getStartTime() <<
		"end_time" << subTour.getEndTime() <<
		"hhfactor" << personParams.getHouseholdFactor()
	);
	mongoDao["Output_SubTour"]->insert(tourDoc);
}

void sim_mob::medium::PredaySystem::insertStop(const Stop* stop, int stopNumber, int tourNumber)
{
	BSONObj stopDoc = BSON(
	"arrival" << stop->getArrivalTime() <<
	"destination" << stop->getStopLocation() <<
	"primary" << stop->isPrimaryActivity() <<
	"departure" << stop->getDepartureTime() <<
	"stop_ctr" << stopNumber <<
	"stop_type" << stop->getStopTypeStr() <<
	"person_id" << personParams.getPersonId() <<
	"tour_num" << tourNumber <<
	"stop_mode" << stop->getStopMode() <<
	"hhfactor" << personParams.getHouseholdFactor() <<
	"first_bound" << stop->isInFirstHalfTour()
	);
	mongoDao["Output_Activity"]->insert(stopDoc);
}

long sim_mob::medium::PredaySystem::getRandomNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const {
	size_t numNodes = nodes.size();
	if(numNodes == 0) { return 0; }
	if(numNodes == 1)
	{
		const ZoneNodeParams* znNdPrms = nodes.front();
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode() || znNdPrms->isBusTerminusNode()) { return 0; }
		return znNdPrms->getAimsunNodeId();
	}

	int offset = Utils::generateInt(0,numNodes-1);
	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	std::advance(it, offset);
	size_t numAttempts = 1;
	while(numAttempts <= numNodes)
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode() || znNdPrms->isBusTerminusNode())
		{
			it++; // check the next one
			if(it==nodes.end()) { it = nodes.begin(); } // loop around
			numAttempts++;
		}
		else { return znNdPrms->getAimsunNodeId(); }
	}
	return 0;
}

long sim_mob::medium::PredaySystem::getFirstNodeInZone(const std::vector<ZoneNodeParams*>& nodes) const {
	size_t numNodes = nodes.size();
	if(numNodes == 0) { return 0; }
	if(numNodes == 1)
	{
		const ZoneNodeParams* znNdPrms = nodes.front();
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode() || znNdPrms->isBusTerminusNode()) { return 0; }
		return znNdPrms->getAimsunNodeId();
	}

	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	while(it!=nodes.end())
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode() || znNdPrms->isBusTerminusNode()) { it++; }// check the next one
		else { return znNdPrms->getAimsunNodeId(); }
	}
	return 0;
}

void sim_mob::medium::PredaySystem::computeLogsums()
{
	if(personParams.hasFixedWorkPlace())
	{
		TourModeParams tmParams;
		constructTourModeParams(tmParams, personParams.getFixedWorkLocation(), WORK);
		PredayLuaProvider::getPredayModel().computeTourModeLogsum(personParams, tmParams);
	}
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, NULL_STOP, unavailableODs, MTZ12_MTZ08_Map);
	tmdParams.setCbdOrgZone(zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()))->getCbdDummy());
	PredayLuaProvider::getPredayModel().computeTourModeDestinationLogsum(personParams, tmdParams);
	PredayLuaProvider::getPredayModel().computeDayPatternLogsums(personParams);

	logStream << "Person: " << personParams.getPersonId()
			<< "|updated logsums- work: " << personParams.getWorkLogSum()
			<< ", shop: " << personParams.getShopLogSum()
			<< ", other: " << personParams.getOtherLogSum()
			<< ", dpt: " << personParams.getDptLogsum()
			<< ", dps: " << personParams.getDpsLogsum()
			<<std::endl;
}

void sim_mob::medium::PredaySystem::computeLogsumsForLT(std::stringstream& outStream)
{
	computeLogsums();
	PredayLuaProvider::getPredayModel().computeDayPatternBinaryLogsums(personParams);
	outStream << personParams.getPersonId()
			<< "," << personParams.getHomeLocation()
			<< "," << personParams.getFixedWorkLocation()
			<< "," << personParams.getHhId()
			<< "," << personParams.getDpbLogsum()
			<< "\n";
}

void sim_mob::medium::PredaySystem::outputPredictionsToMongo() {
	insertDayPattern();
	int tourNum=0;
	for(TourList::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		tourNum++;
		const Tour& currTour = *tourIt;
		insertTour(currTour, tourNum);
		int subTourNum=0;
		const TourList& subTourLst = currTour.subTours;
		for(TourList::const_iterator subTourIt=subTourLst.begin(); subTourIt!=subTourLst.end(); subTourIt++) {
			subTourNum++;
			insertSubTour(*subTourIt, currTour, tourNum, subTourNum);
		}
		int stopNum=0;
		const StopList& stopLst = currTour.stops;
		for(StopList::const_iterator stopIt=stopLst.begin(); stopIt!=stopLst.end(); stopIt++) {
			stopNum++;
			insertStop(*stopIt, stopNum, tourNum);
		}
	}
}

void sim_mob::medium::PredaySystem::updateLogsumsToMongo()
{
	Query query = QUERY("_id" << personParams.getPersonId());
	BSONObj updateObj = BSON("$set" << BSON(
			MONGO_FIELD_WORK_LOGSUM << personParams.getWorkLogSum() <<
			MONGO_FIELD_SHOP_LOGSUM << personParams.getShopLogSum() <<
			MONGO_FIELD_OTHER_LOGSUM << personParams.getOtherLogSum() <<
			MONGO_FIELD_DPT_LOGSUM << personParams.getDptLogsum() <<
			MONGO_FIELD_DPS_LOGSUM << personParams.getDpsLogsum()
			));
	mongoDao["population"]->update(query, updateObj);
}

void sim_mob::medium::PredaySystem::outputActivityScheduleToStream(const ZoneNodeMap& zoneNodeMap, std::stringstream& outStream)
{
	size_t numTours = tours.size();
	if (numTours == 0) { return; }
	std::string personId = personParams.getPersonId();
	long hhFactor = (long)std::ceil(personParams.getHouseholdFactor());
	for(long k=1; k<=hhFactor; k++)
	{
		int homeZone = personParams.getHomeLocation();
		int homeNode = 0;
		long homeAddressId = personParams.getHomeAddressId();
		if(homeAddressId < 0) //home address id is -1 if not explicitly set
		{
			if(zoneNodeMap.find(homeZone) != zoneNodeMap.end())
			{
				homeNode =  getRandomNodeInZone(zoneNodeMap.at(homeZone));
			}
			if(homeNode == 0) { return; } //return if homeless
		}
		else
		{
			homeNode = personParams.getSimMobNodeForAddressId(homeAddressId);
		}

		std::string pid;
		{
			std::stringstream sclPersonIdStrm;
			sclPersonIdStrm << personId << "-" << k;
			pid = sclPersonIdStrm.str();
		}

		int tourNum = 1;
		double homeActivityEndTime = getTimeWindowFromIndex(tours.front().getStartTime());
		for(TourList::const_iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++)
		{
			std::stringstream tourStream;
			const Tour& tour = (*tourIt);
			int stopNum = 0;
			bool nodeMappingFailed = false;
			const StopList& stops = tour.stops;

			int prevStopNode = homeNode;
			int prevStopZone = homeZone;
			double prevStopEndTime = homeActivityEndTime;
			int currStopZone, currStopNode;
			double currStopEndTime;
			long activityAddressId = 0;
			for(StopList::const_iterator stopIt=stops.begin(); stopIt!=stops.end(); stopIt++)
			{
				const Stop* stop = (*stopIt);
				currStopZone = stop->getStopLocation();
				currStopNode = 0;
				if(stop->isPrimaryActivity() && tour.isUsualLocation() &&
						((stop->getStopType() == sim_mob::WORK && personParams.getFixedWorkLocation() != 0)
								|| (stop->getStopType() == sim_mob::EDUCATION && personParams.getFixedSchoolLocation() != 0)))
				{
					currStopNode = personParams.getSimMobNodeForAddressId(personParams.getActivityAddressId());
				}
				else
				{
					const std::vector<long>& addressesInZone = personParams.getAddressIdsInZone(currStopZone);
					if(addressesInZone.empty())
					{
						ZoneNodeMap::const_iterator zoneNodeMapIt = zoneNodeMap.find(currStopZone);
						if (zoneNodeMapIt != zoneNodeMap.end())
						{
							currStopNode = getRandomNodeInZone(zoneNodeMapIt->second);
						}
					}
					else if(addressesInZone.size() == 1) //trivial
					{
						currStopNode = personParams.getSimMobNodeForAddressId(addressesInZone.front());
					}
					else
					{
						ZoneAddressParams znAddrParams = ZoneAddressParams(personParams.getAddressLookup(), addressesInZone);
						long addressId = PredayLuaProvider::getPredayModel().predictAddress(znAddrParams);
						currStopNode = personParams.getSimMobNodeForAddressId(addressId);
					}
					if (currStopNode == 0)
					{
						nodeMappingFailed = true;
						break; // if there is no next node, cut the trip chain for this tour here
					}
				}
				currStopEndTime = getTimeWindowFromIndex(stop->getDepartureTime());
				stopNum++;
				//person_id character,tour_no,tour_type,stop_no integer NOT NULL,stop_type,stop_location,stop_mode,is_primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_departure_time
				tourStream << pid << ","
						<< tourNum << ","
						<< tour.getTourTypeStr() << ","
						<< stopNum << ","
						<< stop->getStopTypeStr() << ","
						<< currStopNode << ","
						<< currStopZone << ","
						<< modeMap.at(stop->getStopMode()) << ","
						<< (stop->isPrimaryActivity()? "True":"False")  << ","
						<< getTimeWindowFromIndex(stop->getArrivalTime()) << ","
						<< currStopEndTime << ","
						<< prevStopNode << ","
						<< prevStopZone << ","
						<< prevStopEndTime <<"\n";
				prevStopZone = currStopZone;
				prevStopNode = currStopNode;
				prevStopEndTime = currStopEndTime;
			}

			if(stopNum > 0) // if there was atleast one stop (with valid node) in tour
			{
				homeActivityEndTime = LAST_WINDOW;
				TourList::const_iterator nextTourIt=tourIt; nextTourIt++; //copy and then increment
				if(nextTourIt!=tours.end()) { homeActivityEndTime = getTimeWindowFromIndex((*nextTourIt).getStartTime()); }

				//Home activity
				//person_id character,tour_no,tour_type,stop_no,stop_type,stop_location,stop_mode,is_primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_departure_time
				tourStream << pid << ","
						<< tourNum << ","
						<< tour.getTourTypeStr() << ","
						<< ++stopNum << ","
						<< "Home" << ","
						<< homeNode << ","
						<< homeZone << ","
						<< modeMap.at(tour.getTourMode()) << ","
						<< "False"  << ","
						<< getTimeWindowFromIndex(tour.getEndTime()) << ","
						<< homeActivityEndTime << ","
						<< prevStopNode << ","
						<< prevStopZone << ","
						<< prevStopEndTime << "\n";
				outStream << tourStream.str();
				tourNum++;
			}
		}
	}
}

void sim_mob::medium::PredaySystem::printLogs()
{
	Print() << logStream.str();
	logStream.str(std::string());
}

void sim_mob::medium::PredaySystem::updateStatistics(CalibrationStatistics& statsCollector) const
{
	double householdFactor = personParams.getHouseholdFactor();
	statsCollector.addToTourCountStats(tours.size(), householdFactor);
	for(TourList::const_iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++)
	{
		const Tour& tour = (*tourIt);
		statsCollector.addToStopCountStats(tour.stops.size()-1, householdFactor);
		statsCollector.addToTourModeShareStats(tour.getTourMode(), householdFactor);
		const StopList& stops = tour.stops;
		int origin = personParams.getHomeLocation();
		int destination = 0;
		for(StopList::const_iterator stopIt=stops.begin(); stopIt!=stops.end(); stopIt++)
		{
			const Stop* stop = (*stopIt);
			if(!stop->isPrimaryActivity())
			{
				statsCollector.addToTripModeShareStats(stop->getStopMode(), householdFactor);
			}
			destination = stop->getStopLocation();
			if(origin != destination) { statsCollector.addToTravelDistanceStats(opCostMap.at(origin).at(destination)->getDistance(), householdFactor); }
			else { statsCollector.addToTravelDistanceStats(0, householdFactor); }
			origin = destination;
		}
		//There is still one more trip from last stop to home
		destination = personParams.getHomeLocation();
		if(origin != destination) { statsCollector.addToTravelDistanceStats(opCostMap.at(origin).at(destination)->getDistance(), householdFactor); }
		else { statsCollector.addToTravelDistanceStats(0, householdFactor); }
	}
}
