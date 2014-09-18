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
#include <cmath>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdint.h>
#include "behavioral/lua/PredayLuaProvider.hpp"
#include "behavioral/params/ModeDestinationParams.hpp"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"
#include "behavioral/params/TourModeParams.hpp"
#include "behavioral/params/TripChainItemParams.hpp"
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
	const double WALKABLE_DISTANCE = 3.0;
	const double PEDESTRIAN_WALK_SPEED = 5.0; //kmph

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
		res[1] = "Bus";
		res[2] = "MRT";
		res[3] = "Bus";
		res[4] = "Car";
		res[5] = "Car Sharing";
		res[6] = "Car Sharing";
		res[7] = "Bike";
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
		return (index * 0.5 /*half hour windows*/) + 2.75 /*the day starts at 3.25*/;
	}

	inline double getIndexFromTimeWindow(const double window) {
		return (window - 2.75 /*the day starts at 3.25*/) / 0.5;
	}

	double alignTime(double time) {
		// align to corresponding time window
		//1. split the computed tour end time into integral and fractional parts
		double intPart,fractPart;
		fractPart = std::modf(time, &intPart);

		//2. perform sanity checks on the integral part and align the fractional part to nearest time window
		if (time < FIRST_WINDOW) {
			time = FIRST_WINDOW;
		}
		else if (time > LAST_WINDOW) {
			time = LAST_WINDOW;
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

	void constructTrip(TripChainItemParams& tcTrip, int prevNode, int nextNode, const std::string& startTime) {
		//tcTrip.setSubtripMode(modeMap.at(stop->getStopMode()));
		//tcTrip.setPrimaryMode((tour->getTourMode() == stop->getStopMode()));
		tcTrip.setSubtripMode(modeMap.at(4)); /*~ all trips are made to car trips. Done for running mid-term for TRB paper. ~*/
		tcTrip.setPrimaryMode(true); /*~ running mid-term for TRB paper. ~*/
		tcTrip.setTripOrigin(prevNode);
		tcTrip.setTripDestination(nextNode);
		tcTrip.setSubtripOrigin(prevNode);
		tcTrip.setSubtripDestination(nextNode);
		tcTrip.setStartTime(startTime);
	}

	std::string constructActivity(TripChainItemParams& tcActivity, const Stop* stop, int nextNode, const std::string& arrTimeStr, const std::string& deptTimeStr) {
		tcActivity.setActivityType(stop->getStopTypeStr());
		tcActivity.setActivityLocation(nextNode);
		tcActivity.setPrimaryActivity(stop->isPrimaryActivity());
		tcActivity.setActivityStartTime(arrTimeStr);
		tcActivity.setActivityEndTime(deptTimeStr);
		return deptTimeStr;
	}
}

PredaySystem::PredaySystem(PersonParams& personParams,
		const ZoneMap& zoneMap, const boost::unordered_map<int,int>& zoneIdLookup,
		const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap,
		const std::map<std::string, db::MongoDao*>& mongoDao,
		const std::vector<OD_Pair>& unavailableODs)
: personParams(personParams), zoneMap(zoneMap), zoneIdLookup(zoneIdLookup),
  amCostMap(amCostMap), pmCostMap(pmCostMap), opCostMap(opCostMap),
  mongoDao(mongoDao), unavailableODs(unavailableODs), logStream(std::stringstream::out)
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

void PredaySystem::predictTourMode(Tour& tour) {
	TourModeParams tmParams;
	tmParams.setStopType(tour.getTourType());

	ZoneParams* znOrgObj = zoneMap.at(zoneIdLookup.at(personParams.getHomeLocation()));
	ZoneParams* znDesObj = zoneMap.at(zoneIdLookup.at(tour.getTourDestination()));
	tmParams.setCostCarParking(znDesObj->getParkingRate());
	tmParams.setCentralZone(znDesObj->getCentralDummy());
	tmParams.setResidentSize(znOrgObj->getResidentWorkers());
	tmParams.setWorkOp(znDesObj->getEmployment());
	tmParams.setEducationOp(znDesObj->getTotalEnrollment());
	tmParams.setOriginArea(znOrgObj->getArea());
	tmParams.setDestinationArea(znDesObj->getArea());
	if(personParams.getHomeLocation() != tour.getTourDestination()) {
		CostParams* amObj = amCostMap.at(personParams.getHomeLocation()).at(tour.getTourDestination());
		CostParams* pmObj = pmCostMap.at(tour.getTourDestination()).at(personParams.getHomeLocation());
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
		switch(tmParams.getStopType()){
		case WORK:
			tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwn());
			tmParams.setShare2Available(1);
			tmParams.setShare3Available(1);
			tmParams.setPublicBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setMrtAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setPrivateBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setWalkAvailable(amObj->getPubIvt() <= WALKABLE_DISTANCE && pmObj->getPubIvt() <= WALKABLE_DISTANCE);
			tmParams.setTaxiAvailable(1);
			tmParams.setMotorAvailable(1);
			break;
		case EDUCATION:
			tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
			tmParams.setShare2Available(1);
			tmParams.setShare3Available(1);
			tmParams.setPublicBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setMrtAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setPrivateBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setWalkAvailable(amObj->getPubIvt() <= WALKABLE_DISTANCE && pmObj->getPubIvt() <= WALKABLE_DISTANCE);
			tmParams.setTaxiAvailable(1);
			tmParams.setMotorAvailable(1);
			break;
		}
	}
	else {
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
	}

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
		logStream << "SubTour|" << "mode: " << subTour.getTourMode() << "|destination: " << subTour.getTourDestination()
				<< "|start time: " << subTour.getStartTime() << "|end time: " << subTour.getEndTime() << std::endl;
	}
}

void PredaySystem::predictSubTourModeDestination(Tour& subTour, const Tour& parentTour)
{
	TourModeDestinationParams stmdParams(zoneMap, amCostMap, pmCostMap, personParams, subTour.getTourType());
	stmdParams.setModeForParentWorkTour(parentTour.getTourMode());
	int modeDest = PredayLuaProvider::getPredayModel().predictSubTourModeDestination(personParams, stmdParams);
	subTour.setTourMode(stmdParams.getMode(modeDest));
	int zone_id = stmdParams.getDestination(modeDest);
	subTour.setTourDestination(zoneMap.at(zone_id)->getZoneCode());
}

void PredaySystem::predictTourModeDestination(Tour& tour) {
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, tour.getTourType());
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
	int origin = personParams.getHomeLocation();
	int destination = tour.getTourDestination();
	std::vector<double> ttFirstHalfTour, ttSecondHalfTour;
	if(origin != destination) {
		for (uint32_t i = FIRST_INDEX; i <= LAST_INDEX; i++) {
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
					CostParams* amDistanceObj = amCostMap.at(origin).at(destination);
					travelTime = amDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
				}
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{
					CostParams* pmDistanceObj = pmCostMap.at(origin).at(destination);
					travelTime = pmDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
				}
				else // if i is in off-peak period
				{
					CostParams* opDistanceObj = opCostMap.at(origin).at(destination);
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
	TourTimeOfDayParams todParams(ttFirstHalfTour, ttSecondHalfTour);
	timeWndw = PredayLuaProvider::getPredayModel().predictTourTimeOfDay(personParams, todParams, tour.getTourType());
	return TimeWindowAvailability::timeWindowsLookup.at(timeWndw - 1); //timeWndw ranges from 1 - 1176. Vector starts from 0.
}

void PredaySystem::constructIntermediateStops(Tour& tour, size_t remainingTours)
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
		do
		{
			--stopIt;
			predictStopModeDestination(*stopIt, currStop->getStopLocation());
			currStop = *stopIt;
		}
		while(stopIt!=firstStopIt);
	}

	//second half tour
	if(primaryStopIt!=(--stops.end())) // otherwise, there is no stop in the second half tour
	{
		// mode/destination is predicted for each stop in chronological order from primary stop location toward home location in second half tour
		StopList::iterator stopIt = primaryStopIt; //init stopIt to primartStopIt
		StopList::iterator lastStopIt = --stops.end();
		Stop* currStop = primaryStop; // init currStop to primaryStop
		do
		{
			++stopIt;
			predictStopModeDestination(*stopIt, currStop->getStopLocation());
			currStop = *stopIt;
		}
		while(stopIt!=lastStopIt);
	}

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
			calculateDepartureTime(currStop, prevStop);
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
			logStream << "Removing " << numRemoved << " stops in 1st HT due to TOD issue." << std::endl;
		}
	}

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
			}
			logStream << "Removing " << numRemoved << " stops in 2nd HT due to TOD issue." << std::endl;
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
			generatedStop = new Stop(sim_mob::medium::WORK, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case EDU_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::medium::EDUCATION, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case SHOP_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::medium::SHOP, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		case OTHER_CHOICE_ISG:
			generatedStop = new Stop(sim_mob::medium::OTHER, tour, false /*not primary*/, (halfTour==1) /*in first half tour*/);
			tour.addStop(generatedStop);
			break;
		default:
			throw std::runtime_error("invalid choice predicted by ISG model");
		}
	}
}

void PredaySystem::predictStopModeDestination(Stop* stop, int origin)
{
	StopModeDestinationParams imdParams(zoneMap, amCostMap, pmCostMap, personParams, stop, origin, unavailableODs);
	int modeDest = PredayLuaProvider::getPredayModel().predictStopModeDestination(personParams, imdParams);
	stop->setStopMode(imdParams.getMode(modeDest));
	int zone_id = imdParams.getDestination(modeDest);
	stop->setStopLocationId(zone_id);
	stop->setStopLocation(zoneMap.at(zone_id)->getZoneCode());
}

bool PredaySystem::predictStopTimeOfDay(Stop* stop, int destination, bool isBeforePrimary)
{
	if(!stop) { throw std::runtime_error("predictStopTimeOfDay() - stop is null"); }
	StopTimeOfDayParams stodParams(stop->getStopTypeID(), isBeforePrimary);
	int origin = stop->getStopLocation();

	if(origin == destination) { for(int i=FIRST_INDEX; i<=LAST_INDEX; i++) { stodParams.travelTimes.push_back(0.0); } }
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
				{ travelTime = amCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED; }
				else if(i>=PM_PEAK_LOW && i<=PM_PEAK_HIGH) // if i is in PM peak period
				{ travelTime = pmCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED; }
				else // if i is in off-peak period
				{ travelTime = opCostMap.at(origin).at(destination)->getDistance()/PEDESTRIAN_WALK_SPEED; }
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
		if(stop->getParentTour().isFirstTour()) { stodParams.setTodLow(FIRST_INDEX); }
		else
		{
			TourList::iterator currTourIt = std::find(tours.begin(), tours.end(), stop->getParentTour());
			const Tour& prevTour = *(--currTourIt);
			stodParams.setTodLow(prevTour.getEndTime());
		}
	}
	else
	{
		stodParams.setTodLow(stop->getArrivalTime());
		stodParams.setTodHigh(LAST_INDEX); // end of day
	}

	if(stodParams.getTodHigh() < stodParams.getTodLow()) { return false; } //Invalid low and high TODs for stop

	ZoneParams* zoneDoc = zoneMap.at(zoneIdLookup.at(origin));
	if(origin != destination)
	{
		// calculate costs
		CostParams* amDoc = amCostMap.at(origin).at(destination);
		CostParams* pmDoc = pmCostMap.at(origin).at(destination);
		CostParams* opDoc = opCostMap.at(origin).at(destination);
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

double PredaySystem::fetchTravelTime(int origin, int destination, int mode,  bool isArrivalBased, double timeIdx)
{
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
				CostParams* amDistanceObj = amCostMap.at(origin).at(destination);
				travelTime = amDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			}
			else if(timeIdx>=PM_PEAK_LOW && timeIdx<=PM_PEAK_HIGH) // if i is in PM peak period
			{
				CostParams* pmDistanceObj = pmCostMap.at(origin).at(destination);
				travelTime = pmDistanceObj->getDistance()/PEDESTRIAN_WALK_SPEED;
			}
			else // if i is in off-peak period
			{
				CostParams* opDistanceObj = opCostMap.at(origin).at(destination);
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
	double travelTime = fetchTravelTime(currStop->getStopLocation(), nextStop->getStopLocation(), currStop->getStopMode(), false, currActivityDepartureIndex);
	double nextStopArrTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	nextStopArrTime = alignTime(nextStopArrTime);
	nextStopArrTime = getIndexFromTimeWindow(nextStopArrTime);
	nextStop->setArrivalTime(nextStopArrTime);
}

void PredaySystem::calculateDepartureTime(Stop* currStop,  Stop* prevStop) {
	// person will arrive at the current stop from the previous stop
	// this function sets the departure time for the prevStop
	double currActivityArrivalIndex = currStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(currActivityArrivalIndex);
	double travelTime = fetchTravelTime(currStop->getStopLocation(), prevStop->getStopLocation(), currStop->getStopMode(), true, currActivityArrivalIndex);
	double prevStopDepTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	prevStopDepTime = alignTime(prevStopDepTime);
	prevStopDepTime = getIndexFromTimeWindow(prevStopDepTime);
	prevStop->setDepartureTime(prevStopDepTime);
}

void PredaySystem::blockTravelTimeToSubTourLocation(const Tour& subTour, const Tour& parentTour, SubTourParams& stParams)
{
	//get travel time from parentTour destination to subTour destination and block that time
	double activityDepartureIndex = parentTour.getPrimaryStop()->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(activityDepartureIndex);
	double travelTime = fetchTravelTime(parentTour.getTourDestination(), subTour.getTourDestination(), subTour.getTourMode(), false, activityDepartureIndex);
	double firstPossibleArrTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	firstPossibleArrTime = alignTime(firstPossibleArrTime);
	firstPossibleArrTime = getIndexFromTimeWindow(firstPossibleArrTime);
	stParams.blockTime(activityDepartureIndex, firstPossibleArrTime);

	//get travel time from subTour destination to parentTour destination and block that time
	double activityArrivalIndex = parentTour.getPrimaryStop()->getDepartureTime();
	timeWindow = getTimeWindowFromIndex(activityArrivalIndex);
	travelTime = fetchTravelTime(subTour.getTourDestination(), parentTour.getTourDestination(), subTour.getTourMode(), true, activityArrivalIndex);
	double lastPossibleDepTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	lastPossibleDepTime = alignTime(lastPossibleDepTime);
	lastPossibleDepTime = getIndexFromTimeWindow(lastPossibleDepTime);
	stParams.blockTime(lastPossibleDepTime, activityArrivalIndex);
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
	tourStartTime = alignTime(tourStartTime);
	tourStartTime = getIndexFromTimeWindow(tourStartTime);
	subTour.setStartTime(tourStartTime);

	//estimate tour end time
	double activityDepartureIndex = primaryStop->getDepartureTime();
	timeWindow = getTimeWindowFromIndex(activityDepartureIndex);
	travelTime = fetchTravelTime(primaryStop->getStopLocation(), parentTour.getTourDestination(), subTour.getTourMode(), false, activityDepartureIndex);
	double tourEndTime = timeWindow + travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourEndTime = alignTime(tourEndTime);
	tourEndTime = getIndexFromTimeWindow(tourEndTime);
	subTour.setEndTime(tourEndTime);
}

void PredaySystem::calculateTourStartTime(Tour& tour)
{
	Stop* firstStop = tour.stops.front();
	double firstActivityArrivalIndex = firstStop->getArrivalTime();
	double timeWindow = getTimeWindowFromIndex(firstActivityArrivalIndex);
	double travelTime = fetchTravelTime(firstStop->getStopLocation(), personParams.getHomeLocation(), firstStop->getStopMode(), true, firstActivityArrivalIndex);
	double tourStartTime = timeWindow - travelTime;
	// travel time can be unreasonably high sometimes. E.g. when the travel time is unknown, the default is set to 999
	tourStartTime = alignTime(tourStartTime);
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
	tourEndTime = alignTime(tourEndTime);
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
		logStream << "Attends usual work location: " << attendsUsualWorkLocation << std::endl;
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

void PredaySystem::planDay() {
	personParams.initTimeWindows();
	logStream << std::endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	//Predict day pattern
	logStream << "Person: " << personParams.getPersonId() << "| home: " << personParams.getHomeLocation() << std:: endl;
	logStream << "Day Pattern: " ;
	PredayLuaProvider::getPredayModel().predictDayPattern(personParams, dayPattern);
	if(dayPattern.empty()) { throw std::runtime_error("Cannot invoke number of tours model without a day pattern"); }
	logStream << dayPattern["WorkT"] << dayPattern["EduT"] << dayPattern["ShopT"] << dayPattern["OthersT"]
	        << dayPattern["WorkI"] << dayPattern["EduI"] << dayPattern["ShopI"] << dayPattern["OthersI"] << std::endl;

	//Predict number of Tours
	logStream << "Num. Tours: ";
	PredayLuaProvider::getPredayModel().predictNumTours(personParams, dayPattern, numTours);
	logStream << numTours["WorkT"] << numTours["EduT"] << numTours["ShopT"] << numTours["OthersT"] << std::endl;

	//Construct tours.
	constructTours();
	logStream << "Tours: " << tours.size() << std::endl;
	if(!tours.empty()) { tours.front().setFirstTour(true); } // make first tour aware that it is the first tour for person

	double prevTourEndTime = FIRST_INDEX;
	//Process each tour
	size_t remainingTours = tours.size();
	for(TourList::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		Tour& tour = *tourIt;
		remainingTours = remainingTours - 1; // 1 less tours to be processed after current tour
		if(tour.isUsualLocation()) {
			// Predict just the mode for tours to usual location
			predictTourMode(tour);
			logStream << "Tour|type: " << tour.getTourType()
					<< "(TM) Tour mode: " << tour.getTourMode() << "|Tour destination: " << tour.getTourDestination();
		}
		else {
			// Predict mode and destination for tours to not-usual locations
			predictTourModeDestination(tour);
			logStream << "Tour|type: " << tour.getTourType()
					<< "(TMD) Tour mode: " << tour.getTourMode() << "|Tour destination: " << tour.getTourDestination();
		}

		// Predict time of day for this tour
		TimeWindowAvailability timeWindow = predictTourTimeOfDay(tour);
		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true /*primary activity*/, true /*stop in first half tour*/);
		primaryActivity->setStopMode(tour.getTourMode());
		primaryActivity->setStopLocation(tour.getTourDestination());
		primaryActivity->setStopLocationId(zoneIdLookup.at(tour.getTourDestination()));
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		tour.setPrimaryStop(primaryActivity);
		tour.addStop(primaryActivity);
		personParams.blockTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		logStream << "|primary activity|arrival: " << primaryActivity->getArrivalTime() << "|departure: " << primaryActivity->getDepartureTime() << std::endl;

		//Generate sub tours for work tours
		if(tour.getTourType() == sim_mob::medium::WORK) { predictSubTours(tour); }

		//Generate stops for this tour
		constructIntermediateStops(tour, remainingTours);

		calculateTourStartTime(tour);
		calculateTourEndTime(tour);
		personParams.blockTime(prevTourEndTime, tour.getEndTime());
		prevTourEndTime = tour.getEndTime();
		logStream << "Tour|start time: " << tour.getStartTime() << "|end time: " << tour.getEndTime() << std::endl;
	}
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
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode()) { return 0; }
		return znNdPrms->getAimsunNodeId();
	}

	int offset = Utils::generateInt(0,numNodes-1);
	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	std::advance(it, offset);
	size_t numAttempts = 1;
	while(numAttempts <= numNodes)
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode())
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
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode()) { return 0; }
		return znNdPrms->getAimsunNodeId();
	}

	std::vector<ZoneNodeParams*>::const_iterator it = nodes.begin();
	while(it!=nodes.end())
	{
		const ZoneNodeParams* znNdPrms = (*it);
		if(znNdPrms->isSinkNode() || znNdPrms->isSourceNode()) { it++; }// check the next one
		else { return znNdPrms->getAimsunNodeId(); }
	}
	return 0;
}

void sim_mob::medium::PredaySystem::computeLogsums()
{
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, NULL_STOP);
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

void sim_mob::medium::PredaySystem::constructTripChains(const ZoneNodeMap& zoneNodeMap, long hhFactor, std::list<TripChainItemParams>& tripChain)
{
	std::string personId = personParams.getPersonId();
	for(long k=1; k<=hhFactor; k++)
	{
		std::string pid;
		{
			std::stringstream sclPersonIdStrm;
			sclPersonIdStrm << personId << "-" << k;
			pid = sclPersonIdStrm.str();
		}
		int seqNum = 0;
		int prevNode = 0;
		int nextNode = 0;
		std::string prevDeptTime = "";
		std::string primaryMode = "";
		bool atHome = true;
		int homeNode = 0;
		if(zoneNodeMap.find(personParams.getHomeLocation()) != zoneNodeMap.end())
		{
			homeNode =  getRandomNodeInZone(zoneNodeMap.at(personParams.getHomeLocation()));
		}
		if(homeNode == 0) { return; } //do not insert this person at all
		int tourNum = 0;
		for(TourList::const_iterator tourIt = tours.begin(); tourIt != tours.end(); tourIt++)
		{
			tourNum = tourNum + 1;
			const Tour& tour = *tourIt;
			int stopNum = 0;
			bool nodeMappingFailed = false;
			const StopList& stopLst = tour.stops;
			for(StopList::const_iterator stopIt = tour.stops.begin(); stopIt != tour.stops.end(); stopIt++)
			{
				stopNum = stopNum + 1;
				Stop* stop = *stopIt;
				nextNode = 0;
				ZoneNodeMap::const_iterator zoneNodeMapIt = zoneNodeMap.find(stop->getStopLocation());
				if(zoneNodeMapIt != zoneNodeMap.end())
				{
					nextNode = getFirstNodeInZone(zoneNodeMapIt->second);
				}
				if(nextNode == 0) { nodeMappingFailed = true; break; } // if there is no next node, cut the trip chain for this tour here

				seqNum = seqNum + 1;
				TripChainItemParams tcTrip = TripChainItemParams(pid, "Trip", seqNum);
				tcTrip.setTripId(constructTripChainItemId(pid, tourNum, seqNum));
				tcTrip.setSubtripId(constructTripChainItemId(pid, tourNum, seqNum, "-1"));
				if(atHome)
				{
					constructTrip(tcTrip, homeNode, nextNode, getRandomTimeInWindow(getTimeWindowFromIndex(tour.getStartTime())));
					atHome = false;
				}
				else
				{
					constructTrip(tcTrip, prevNode, nextNode, prevDeptTime);
				}
				tripChain.push_back(tcTrip);

				std::string arrTimeStr, deptTimeStr;
				if(stop->isPrimaryActivity() && !tour.subTours.empty()) // check for subtours in primary activity
				{
					int tourActivityNode = nextNode;
					double arrivalTime = stop->getArrivalTime();
					for(TourList::const_iterator subTourIt = tour.subTours.begin(); subTourIt != tour.subTours.end(); subTourIt++)
					{
						const Tour& subTour = *subTourIt;
						seqNum = seqNum + 1;
						TripChainItemParams tcActivity = TripChainItemParams(pid, "Activity", seqNum);
						tcActivity.setActivityId(constructTripChainItemId(pid, tourNum, seqNum));
						arrTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(arrivalTime));
						deptTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(subTour.getStartTime()));
						constructActivity(tcActivity, stop, nextNode, arrTimeStr, deptTimeStr);
						tripChain.push_back(tcActivity);
						prevNode = nextNode; //activity location
						prevDeptTime = deptTimeStr;

						const Stop* subTourPrimaryStop = subTour.getPrimaryStop(); // subtours have only one stop
						nextNode = 0;
						ZoneNodeMap::const_iterator zoneNodeMapIt = zoneNodeMap.find(subTourPrimaryStop->getStopLocation());
						if(zoneNodeMapIt != zoneNodeMap.end())
						{
							nextNode = getFirstNodeInZone(zoneNodeMapIt->second);
						}
						if(nextNode == 0) { nodeMappingFailed = true; break; } // if there is no next node, cut the trip chain for this tour here

						// insert trip from activity location to sub-tour activity location
						seqNum = seqNum + 1;
						TripChainItemParams tcSubTourTrip = TripChainItemParams(pid, "Trip", seqNum);
						tcSubTourTrip.setTripId(constructTripChainItemId(pid, tourNum, seqNum));
						tcSubTourTrip.setSubtripId(constructTripChainItemId(pid, tourNum, seqNum, "-1"));
						constructTrip(tcSubTourTrip, prevNode, nextNode, prevDeptTime);
						tripChain.push_back(tcSubTourTrip);

						// insert sub tour activity
						seqNum = seqNum + 1;
						TripChainItemParams tcSubTourActivity = TripChainItemParams(pid, "Activity", seqNum);
						tcSubTourActivity.setActivityId(constructTripChainItemId(pid, tourNum, seqNum));
						std::string subArrTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(subTourPrimaryStop->getArrivalTime()));
						std::string subDeptTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(subTourPrimaryStop->getDepartureTime()));
						constructActivity(tcSubTourActivity, subTourPrimaryStop, nextNode, subArrTimeStr, subDeptTimeStr);
						tripChain.push_back(tcSubTourActivity);
						prevNode = nextNode; //activity location
						prevDeptTime = subDeptTimeStr;

						// insert trip back to tour's primary activity location
						nextNode = tourActivityNode; // get back to tour's primary activity location
						seqNum = seqNum + 1;
						tcSubTourTrip = TripChainItemParams(pid, "Trip", seqNum);
						tcSubTourTrip.setTripId(constructTripChainItemId(pid, tourNum, seqNum));
						tcSubTourTrip.setSubtripId(constructTripChainItemId(pid, tourNum, seqNum, "-1"));
						constructTrip(tcSubTourTrip, prevNode, nextNode, prevDeptTime);
						tripChain.push_back(tcSubTourTrip);
						arrivalTime = subTour.getEndTime(); // for the next activity
					}
					if(nodeMappingFailed) { break; }// ignore remaining tours as well.
					else
					{
						// remainder of the primary activity
						seqNum = seqNum + 1;
						TripChainItemParams tcActivity = TripChainItemParams(pid, "Activity", seqNum);
						tcActivity.setActivityId(constructTripChainItemId(pid, tourNum, seqNum));
						arrTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(arrivalTime));
						deptTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(stop->getDepartureTime()));
						constructActivity(tcActivity, stop, nextNode, arrTimeStr, deptTimeStr);
						tripChain.push_back(tcActivity);
						prevNode = nextNode; //activity location
						prevDeptTime = deptTimeStr;
					}
				}
				else
				{
					seqNum = seqNum + 1;
					TripChainItemParams tcActivity = TripChainItemParams(pid, "Activity", seqNum);
					tcActivity.setActivityId(constructTripChainItemId(pid, tourNum, seqNum));
					arrTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(stop->getArrivalTime()));
					deptTimeStr = getRandomTimeInWindow(getTimeWindowFromIndex(stop->getDepartureTime()));
					constructActivity(tcActivity, stop, nextNode, arrTimeStr, deptTimeStr);
					tripChain.push_back(tcActivity);
				}
				prevNode = nextNode; //activity location
				prevDeptTime = deptTimeStr;
			}
			if(nodeMappingFailed) { break; } // ignore remaining tours as well.
			else
			{
				// insert last trip in tour
				seqNum = seqNum + 1;
				TripChainItemParams tcTrip = TripChainItemParams(pid, "Trip", seqNum);
				tcTrip.setTripId(constructTripChainItemId(pid, tourNum, seqNum));
				tcTrip.setSubtripId(constructTripChainItemId(pid, tourNum, seqNum, "-1"));
				constructTrip(tcTrip, prevNode, homeNode, prevDeptTime);
				tripChain.push_back(tcTrip);
				atHome = true;
			}
		}
	}
}

void sim_mob::medium::PredaySystem::outputTripChainsToPostgreSQL(const ZoneNodeMap& zoneNodeMap, TripChainSqlDao& tripChainDao)
{
	size_t numTours = tours.size();
	if (numTours == 0) { return; }
	long hhFactor = (long)std::ceil(personParams.getHouseholdFactor());
	std::list<TripChainItemParams> tripChain;
	constructTripChains(zoneNodeMap, hhFactor, tripChain);
	for(std::list<TripChainItemParams>::iterator tcIt=tripChain.begin(); tcIt!=tripChain.end();tcIt++)
	{
		tripChainDao.insert(*tcIt);
	}
}

void sim_mob::medium::PredaySystem::outputTripChainsToStream(const ZoneNodeMap& zoneNodeMap, std::stringstream& tripChainStream)
{
	size_t numTours = tours.size();
	if (numTours == 0) { return; }
	long hhFactor = (long)std::ceil(personParams.getHouseholdFactor());
	std::list<TripChainItemParams> tripChains;
	constructTripChains(zoneNodeMap, hhFactor, tripChains);
	for(std::list<TripChainItemParams>::const_iterator tcIt=tripChains.begin(); tcIt!=tripChains.end();tcIt++)
	{
		/*
		  ------------------ DATABASE preday_trip_chain_flat FIELDS for reference --------------------------------
		  person_id character varying NOT NULL,
		  tc_seq_no integer NOT NULL,
		  tc_item_type character varying,
		  trip_id character varying,
		  trip_origin integer,
		  trip_from_loc_type character varying DEFAULT 'node'::character varying,
		  trip_destination integer,
		  trip_to_loc_type character varying DEFAULT 'node'::character varying,
		  subtrip_id character varying,
		  subtrip_origin integer,
		  subtrip_from_loc_type character varying DEFAULT 'node'::character varying,
		  subtrip_destination integer,
		  subtrip_to_loc_type character varying DEFAULT 'node'::character varying,
		  subtrip_mode character varying,
		  is_primary_mode boolean,
		  start_time character varying,
		  pt_line_id character varying DEFAULT ''::character varying,
		  activity_id character varying,
		  activity_type character varying,
		  is_primary_activity boolean,
		  flexible_activity boolean DEFAULT false,
		  mandatory_activity boolean DEFAULT true,
		  activity_location integer,
		  activity_loc_type character varying DEFAULT 'node'::character varying,
		  activity_start_time character varying,
		  activity_end_time character varying
		*/
		const TripChainItemParams& data = (*tcIt);
		tripChainStream << data.getPersonId() << ",";
		tripChainStream << data.getTcSeqNum() << ",";
		tripChainStream << data.getTcItemType() << ",";
		tripChainStream << data.getTripId() << ",";
		tripChainStream << data.getTripOrigin() << "," << "node" << ",";
		tripChainStream << data.getTripDestination() << "," << "node" << ",";
		tripChainStream << data.getSubtripId() << ",";
		tripChainStream << data.getSubtripOrigin() << "," << "node" << ",";
		tripChainStream << data.getSubtripDestination() << "," << "node" << ",";
		tripChainStream << data.getSubtripMode() << ",";
		tripChainStream << (data.isPrimaryMode()? "True":"False") << ",";
		tripChainStream << data.getStartTime() << ",";
		tripChainStream << "\"\"" << ","; //public transit line id
		tripChainStream << data.getActivityId() << ",";
		tripChainStream << data.getActivityType() << ",";
		tripChainStream << (data.isPrimaryActivity()? "True":"False") << ",";
		tripChainStream << "False" << "," << "True" << ","; //flexible and mandatory activity
		tripChainStream << data.getActivityLocation() << "," << "node" << ",";
		tripChainStream << data.getActivityStartTime() << ",";
		tripChainStream << data.getActivityEndTime() << "\n";
	}
}

void sim_mob::medium::PredaySystem::printLogs()
{
	Print() << logStream.str();
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
