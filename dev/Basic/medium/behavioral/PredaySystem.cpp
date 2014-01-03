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

#include <boost/algorithm/string.hpp>
#include <string>
#include "behavioral/lua/PredayLuaProvider.hpp"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"
#include "behavioral/params/TourModeParams.hpp"
#include "behavioral/params/ModeDestinationParams.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "database/DB_Connection.hpp"
#include "logging/Log.hpp"
#include "mongo/client/dbclient.h"
#include "PredayClasses.hpp"


using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace mongo;

PredaySystem::PredaySystem(PersonParams& personParams, const ZoneMap& zoneMap, const boost::unordered_map<int,int>& zoneIdLookup, const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap)
: personParams(personParams), zoneMap(zoneMap), zoneIdLookup(zoneIdLookup), amCostMap(amCostMap), pmCostMap(pmCostMap), opCostMap(opCostMap)
{
	MongoCollectionsMap mongoColl = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo");
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	string emptyString;
	for(std::map<std::string, std::string>::const_iterator i=mongoColl.collectionName.begin(); i!=mongoColl.collectionName.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName, emptyString, emptyString);
		mongoDao[i->first]= new db::MongoDao(dbConfig, db.dbName, i->second);
	}
}

PredaySystem::~PredaySystem()
{
	for(boost::unordered_map<std::string, db::MongoDao*>::iterator i=mongoDao.begin(); i!=mongoDao.end(); i++) {
		safe_delete_item(i->second);
	}
	mongoDao.clear();
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
	ZoneParams* znDesObj = zoneMap.at(tour.getPrimaryActivityLocationId());
	tmParams.setCostCarParking(znDesObj->getParkingRate());
	tmParams.setCentralZone(znDesObj->getCentralDummy());
	tmParams.setResidentSize(znOrgObj->getResidentWorkers());
	tmParams.setWorkOp(znDesObj->getEmployment());
	tmParams.setEducationOp(znDesObj->getTotalEnrollment());
	tmParams.setOriginArea(znOrgObj->getArea());
	tmParams.setDestinationArea(znDesObj->getArea());
	if(personParams.getHomeLocation() != tour.getPrimaryActivityLocation()) {
		CostParams* amObj = amCostMap.at(personParams.getHomeLocation()).at(tour.getPrimaryActivityLocation());
		CostParams* pmObj = pmCostMap.at(tour.getPrimaryActivityLocation()).at(personParams.getHomeLocation());
		tmParams.setCostPublicFirst(amObj->getPubCost());
		tmParams.setCostPublicSecond(pmObj->getPubCost());
		tmParams.setCostCarErpFirst(amObj->getCarCostErp());
		tmParams.setCostCarErpSecond(pmObj->getCarCostErp());
		tmParams.setCostCarOpFirst(amObj->getDistance() * 0.147);
		tmParams.setCostCarOpSecond(pmObj->getDistance() * 0.147);
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
			tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
			tmParams.setShare2Available(1);
			tmParams.setShare3Available(1);
			tmParams.setPublicBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setMrtAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setPrivateBusAvailable(amObj->getPubIvt() > 0 && pmObj->getPubIvt() > 0);
			tmParams.setWalkAvailable(amObj->getPubIvt() <= 3 && pmObj->getPubIvt() <= 3);
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
			tmParams.setWalkAvailable(amObj->getPubIvt() <= 3 && pmObj->getPubIvt() <= 3);
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

void PredaySystem::predictTourModeDestination(Tour& tour) {
	TourModeDestinationParams tmdParams(zoneMap, amCostMap, pmCostMap, personParams, tour.getTourType());
	int modeDest = PredayLuaProvider::getPredayModel().predictTourModeDestination(personParams, tmdParams);
	tour.setTourMode(tmdParams.getMode(modeDest));
	int zone_id = tmdParams.getDestination(modeDest);
	tour.setPrimaryActivityLocationId(zone_id);
	tour.setPrimaryActivityLocation(zoneMap.at(zone_id)->getZoneCode());
}

TimeWindowAvailability PredaySystem::predictTourTimeOfDay(Tour& tour) {
	int timeWndw;
	if(!tour.isSubTour()) {
		int origin = personParams.getHomeLocation();
		int destination = tour.getPrimaryActivityLocation();
		TourTimeOfDayParams todParams;
		timeWndw = PredayLuaProvider::getPredayModel().predictTourTimeOfDay(personParams, todParams, tour.getTourType());
	}
	return TimeWindowAvailability::timeWindowsLookup[timeWndw];
}

void PredaySystem::generateIntermediateStops(Tour& tour) {
	if(tour.stops.size() < 1) {
		stringstream ss;
		ss << "generateIntermediateStops()|tour object contains no stops. 1 stop (primary activity) was expected.";
		throw runtime_error(ss.str());
	}
	Stop* primaryStop = tour.stops.front(); // The only stop at this point is the primary activity stop
	Stop* generatedStop = nullptr;

	if ((dayPattern.at("WorkI") + dayPattern.at("EduI") + dayPattern.at("ShopI") + dayPattern.at("OthersI")) > 0 ) {
		//if any stop type was predicted in the day pattern
		StopGenerationParams isgParams(tour, primaryStop, dayPattern);
		int origin = personParams.getHomeLocation();
		int destination = primaryStop->getStopLocation();
		isgParams.setFirstTour(tours.front() == &tour);
		std::deque<Tour*>::iterator tourIt = std::find(tours.begin(), tours.end(), &tour);
		size_t index = std::distance(tours.begin(), tourIt);
		isgParams.setNumRemainingTours(tours.size() - index + 1);

		//First half tour
		if(origin != destination) {
			CostParams* amDistanceObj = amCostMap.at(origin).at(destination);
			isgParams.setDistance(amDistanceObj->getDistance());
		}
		else {
			isgParams.setDistance(0.0);
		}
		isgParams.setFirstHalfTour(true);

		double prevDepartureTime = 3.25; // first window; start of day
		double nextArrivalTime = primaryStop->getArrivalTime();
		if (tours.front() != &tour) { // if this tour is not the first tour of the day
			Tour* previousTour = *(std::find(tours.begin(), tours.end(), &tour)-1);
			prevDepartureTime = previousTour->getEndTime(); // departure time id taken as the end time of the previous tour
		}

		int stopCounter = 0;
		isgParams.setStopCounter(stopCounter);
		int choice;
		Stop* nextStop = primaryStop;
		while(choice != 5 && stopCounter<3){
			Print() << "generateIntermediateStop()" << std::endl;
			choice = PredayLuaProvider::getPredayModel().generateIntermediateStop(personParams, isgParams);
			if(choice != 5) {
				StopType stopType;
				if(choice == 1) { stopType = WORK; }
				else if (choice == 2) { stopType = EDUCATION; }
				else if (choice == 3) { stopType = SHOP; }
				else if (choice == 4) { stopType = OTHER; }
				Stop* generatedStop = new Stop(stopType, tour, false /*not primary*/, true /*in first half tour*/);
				tour.addStop(generatedStop);
				Print() << "predictStopModeDestination()" << std::endl;
				predictStopModeDestination(generatedStop, nextStop->getStopLocation());
				calculateDepartureTime(generatedStop, nextStop);
				if(generatedStop->getDepartureTime() <= 3.25)
				{
					tour.removeStop(generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				Print() << "predictStopTimeOfDay()" << std::endl;
				predictStopTimeOfDay(generatedStop, true);
				nextStop = generatedStop;
				personParams.blockTime(generatedStop->getArrivalTime(), generatedStop->getDepartureTime());
				nextArrivalTime = generatedStop->getArrivalTime();
				stopCounter = stopCounter + 1;
			}
		}

		// Second half tour
		if(origin != destination) {
			CostParams* pmDistanceObj = pmCostMap.at(origin).at(destination);
			isgParams.setDistance(pmDistanceObj->getDistance());
		}
		else {
			isgParams.setDistance(0.0);
		}
		isgParams.setFirstHalfTour(false);

		prevDepartureTime = primaryStop->getDepartureTime();
		nextArrivalTime = 26.75; // end of day

		stopCounter = 0;
		isgParams.setStopCounter(stopCounter);
		choice = 0;
		Stop* prevStop = primaryStop;
		while(choice != 5 && stopCounter<3){
			Print() << "generateIntermediateStop()" << std::endl;
			choice = PredayLuaProvider::getPredayModel().generateIntermediateStop(personParams, isgParams);
			if(choice != 5) {
				StopType stopType;
				if(choice == 1) { stopType = WORK; }
				else if (choice == 2) { stopType = EDUCATION; }
				else if (choice == 3) { stopType = SHOP; }
				else if (choice == 4) { stopType = OTHER; }
				Stop* generatedStop = new Stop(stopType, tour, false /*not primary*/, false  /*not in first half tour*/);
				tour.addStop(generatedStop);
				Print() << "predictStopModeDestination()" << std::endl;
				predictStopModeDestination(generatedStop, prevStop->getStopLocation());
				calculateArrivalTime(generatedStop, prevStop);
				if(generatedStop->getDepartureTime() >=  26.75)
				{
					tour.removeStop(generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				Print() << "predictStopTimeOfDay()" << std::endl;
				predictStopTimeOfDay(generatedStop, false);
				prevStop = generatedStop;
				personParams.blockTime(generatedStop->getArrivalTime(), generatedStop->getDepartureTime());
				prevDepartureTime = generatedStop->getDepartureTime();
				stopCounter = stopCounter + 1;
			}
		}
	}
}

void PredaySystem::predictStopModeDestination(Stop* stop, int origin)
{
	StopModeDestinationParams imdParams(zoneMap, amCostMap, pmCostMap, personParams, stop->getStopType(), origin, stop->getParentTour().getTourMode());
	int modeDest = PredayLuaProvider::getPredayModel().predictStopModeDestination(personParams, imdParams);
	stop->setStopMode(imdParams.getMode(modeDest));
	int zone_id = imdParams.getDestination(modeDest);
	stop->setStopLocationId(zone_id);
	stop->setStopLocation(zoneMap.at(zone_id)->getZoneCode());
}

void PredaySystem::predictStopTimeOfDay(Stop* stop, bool isBeforePrimary) {
	StopTimeOfDayParams stodParams(stop->getStopTypeID(), isBeforePrimary);
	double origin = stop->getStopLocation();
	double destination = personParams.getHomeLocation();

	if(origin != destination) {
		BSONObj bsonObjTT = BSON("origin" << origin << "destination" << destination);
		BSONObj bsonObjTC = BSON("origin" << destination << "destin" << origin);
		BSONObj tCostBusDoc;
		mongoDao["tcost_bus"]->getOne(bsonObjTT, tCostBusDoc);
		BSONObj tCostCarDoc;
		mongoDao["tcost_car"]->getOne(bsonObjTT, tCostCarDoc);

		CostParams* amDistanceObj = amCostMap.at(destination).at(origin);
		CostParams* pmDistanceObj = pmCostMap.at(destination).at(origin);

		for (uint32_t i = 1; i <= 48; i++) {
			try {
				switch (stop->getStopMode()) {
				case 1: // Fall through
				case 2:
				case 3:
				{
					std::stringstream fieldName;
					if(stodParams.getFirstBound()) {
						fieldName << "TT_bus_arrival_" << i;
					}
					else {
						fieldName << "TT_bus_departure_" << i;
					}
					try {
						stodParams.travelTimes.push_back(tCostBusDoc.getField(fieldName.str()).Double());
					}
					catch (exception& e) {
						//not a double... could be 999
						stodParams.travelTimes.push_back(tCostBusDoc.getField(fieldName.str()).Int());
					}
					break;
				}
				case 4: // Fall through
				case 5:
				case 6:
				case 7:
				case 9:
				{
					std::stringstream fieldName;
					if(stodParams.getFirstBound()) {
						fieldName << "TT_car_arrival_" << i;
					}
					else {
						fieldName << "TT_car_departure_" << i;
					}
					try {
						stodParams.travelTimes.push_back(tCostCarDoc.getField(fieldName.str()).Double());
					}
					catch (exception& e) {
						//not a double... could be an integer
						stodParams.travelTimes.push_back(tCostCarDoc.getField(fieldName.str()).Int());
					}
					break;
				}
				case 8: {
					double distanceMin = amDistanceObj->getDistance() - pmDistanceObj->getDistance();
					stodParams.travelTimes.push_back(distanceMin/5);
					break;
				}
				}
			}
			catch (exception& e) {
				// something unexpected happened while getting data
				// retrieved values could've been NULL
				// therefore, setting the travel times to a high value - 999
				stodParams.travelTimes.push_back(999);
			}
		}
	}
	else {
		for(int i=1; i<=48; i++) {
			stodParams.travelTimes.push_back(0.0);
		}
	}

	// high and low tod
	if(isBeforePrimary) {
		stodParams.setTodHigh(stop->getDepartureTime());
		if(&stop->getParentTour() == tours.front()) {
			stodParams.setTodLow(3.25);
		}
		else {
			Tour* prevTour = *(std::find(tours.begin(), tours.end(), &stop->getParentTour()) - 1);
			stodParams.setTodLow(prevTour->getEndTime());
		}
	}
	else {
		stodParams.setTodLow(stop->getArrivalTime());
		stodParams.setTodHigh(26.75);
	}

	ZoneParams* zoneDoc = zoneMap.at(zoneIdLookup.at(origin));
	if(origin != destination) {
		// calculate costs
		CostParams* amDoc = amCostMap.at(origin).at(destination);
		CostParams* pmDoc = pmCostMap.at(origin).at(destination);
		CostParams* opDoc = opCostMap.at(origin).at(destination);
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=1; i<=48; i++) {

			if(stodParams.getFirstBound()) {
				duration = stodParams.getTodHigh() - i + 1;
			}
			else { //if(stodParams.getSecondBound())
				duration = i - stodParams.getTodLow() + 1;
			}
			duration = 0.25+(duration-1)*0.5;
			parkingRate = zoneDoc->getParkingRate();
			costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

			if(i >= 10 && i <= 14) { // time window indexes 10 to 14 are AM Peak windows
				costCarERP = amDoc->getCarCostErp();
				costCarOP = amDoc->getDistance() * 0.147;
				walkDistance = amDoc->getDistance();
			}
			else if (i >= 30 && i <= 34) { // time window indexes 30 to 34 are PM Peak indexes
				costCarERP = pmDoc->getCarCostErp();
				costCarOP = pmDoc->getDistance() * 0.147;
				walkDistance = pmDoc->getDistance();
			}
			else { // other time window indexes are Off Peak indexes
				costCarERP = opDoc->getCarCostErp();
				costCarOP = opDoc->getDistance() * 0.147;
				walkDistance = opDoc->getDistance();
			}

			switch (stop->getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				if(i >= 10 && i <= 14) { // time window indexes 10 to 14 are AM Peak windows
					stodParams.travelCost.push_back(amDoc->getPubCost());
				}
				else if (i >= 30 && i <= 34) { // time window indexes 30 to 34 are PM Peak indexes
					stodParams.travelCost.push_back(pmDoc->getPubCost());
				}
				else { // other time window indexes are Off Peak indexes
					stodParams.travelCost.push_back(opDoc->getPubCost());
				}
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
				stodParams.travelCost.push_back(3.4+costCarERP
						+3*personParams.getIsFemale()
						+((walkDistance*(walkDistance>10)-10*(walkDistance>10))/0.35 + (walkDistance*(walkDistance<=10)+10*(walkDistance>10))/0.4)*0.22);
				break;
			}
			case 8: {
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}
	else { // if origin and destination are same
		double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
		for(int i=1; i<=48; i++) {

			if(stodParams.getFirstBound()) {
				duration = stodParams.getTodHigh() - i + 1;
			}
			else { //if(stodParams.getSecondBound())
				duration = i - stodParams.getTodLow() + 1;
			}
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
				if(i >= 10 && i <= 14) { // time window indexes 10 to 14 are AM Peak windows
					stodParams.travelCost.push_back(0);
				}
				else if (i >= 30 && i <= 34) { // time window indexes 30 to 34 are PM Peak indexes
					stodParams.travelCost.push_back(0);
				}
				else { // other time window indexes are Off Peak indexes
					stodParams.travelCost.push_back(0);
				}
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
				stodParams.travelCost.push_back(3.4+costCarERP
						+3*personParams.getIsFemale()
						+((walkDistance*(walkDistance>10)-10*(walkDistance>10))/0.35 + (walkDistance*(walkDistance<=10)+10*(walkDistance>10))/0.4)*0.22);
				break;
			}
			case 8: {
				stodParams.travelCost.push_back(0);
				break;
			}
			}
		}
	}

	for(double i=1; i<=48; i++) {
		double timeWndw = (i-1)*0.5 + 3.25;
		if(timeWndw <= stodParams.getTodLow() || timeWndw >= stodParams.getTodHigh()) {
			stodParams.availability[i] = false;
		}
	}

	int timeWindow_idx = PredayLuaProvider::getPredayModel().predictStopTimeOfDay(personParams, stodParams);
	double timeWndw = stodParams.getTimeWindow(timeWindow_idx);
	if(isBeforePrimary) {
		stop->setArrivalTime(timeWndw);
	}
	else {
		stop->setDepartureTime(timeWndw);
	}
}

void PredaySystem::calculateArrivalTime(Stop* currStop,  Stop* prevStop) { // this must set the arrival time for currStop
	/*
	 * There are 48 half-hour time windows in a day from 3.25 to 26.75.
	 * Given a time window x, its choice index can be determined by ((x - 3.25) / 0.5) + 1
	 */
	/*uint32_t prevActivityDepartureIndex = ((prevStop.getDepartureTime() - 3.25)/0.5) +1;
	double travelTime;
	try {
		if(currStop.getStopLocation() != prevStop.getStopLocation()) {
			BSONObjBuilder bsonObjBldr;
			bsonObjBldr << "origin" << currStop.getStopLocation() << "destination" << prevStop.getStopLocation();
			BSONObj bsonObj = bsonObjBldr.obj();

			switch(prevStop.getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				auto_ptr<mongo::DBClientCursor> tCostBus = mongoDao["tcost_bus"]->queryDocument(bsonObj);
				BSONObj tCostBusDoc = tCostBus->next();
				travelTime = tCostBusDoc.getField("TT_bus_departure_" + prevActivityDepartureIndex).Double();
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				auto_ptr<mongo::DBClientCursor> tCostCar = mongoDao["tcost_car"]->queryDocument(bsonObj);
				BSONObj tCostCarDoc = tCostCar->next();
				travelTime = tCostCarDoc.getField("TT_car_departure_" + prevActivityDepartureIndex).Double();
				break;
			}
			case 8:
			{
				BSONObjBuilder bsonObjBldr_Costs;
				bsonObjBldr_Costs << "origin" << currStop.getStopLocation() << "destin" << prevStop.getStopLocation();
				bsonObj = bsonObjBldr_Costs.obj();
				auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObj);
				auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObj);
				BSONObj amDistanceObj = amDistance->next();
				BSONObj pmDistanceObj = pmDistance->next();
				double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
				travelTime = distanceMin/5;
				break;
			}
			}
		}
		else {
			travelTime = 0.0;
		}
	}
	catch (exception& e) {
		// something went wrong in fetching data. Set travel time to a high value.
		travelTime = 999;
	}
	double currStopArrTime = prevStop.getDepartureTime() + travelTime;
	if((currStopArrTime - std::floor(currStopArrTime)) < 0.5) {
		currStopArrTime = std::floor(currStopArrTime) + 0.25;
	}
	else {
		currStopArrTime = std::floor(currStopArrTime) + 0.75;
	}
	currStop.setArrivalTime(currStopArrTime);*/
}

void PredaySystem::calculateDepartureTime(Stop* currStop,  Stop* nextStop) { // this must set the departure time for the currStop
	/*
	 * There are 48 half-hour time windows in a day from 3.25 to 26.75.
	 * Given a time window x, its choice index can be determined by ((x - 3.25) / 0.5) + 1
	 */
	/*uint32_t nextActivityArrivalIndex = ((nextStop.getArrivalTime() - 3.25)/0.5) +1;
	double travelTime;
	try {
		if(currStop.getStopLocation() != nextStop.getStopLocation()) {
			BSONObjBuilder bsonObjBldr;
			bsonObjBldr << "origin" << currStop.getStopLocation() << "destination" << nextStop.getStopLocation();
			BSONObj bsonObj = bsonObjBldr.obj();

			switch(nextStop.getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				auto_ptr<mongo::DBClientCursor> tCostBus = mongoDao["tcost_bus"]->queryDocument(bsonObj);
				BSONObj tCostBusDoc = tCostBus->next();
				travelTime = tCostBusDoc.getField("TT_bus_arrival_" + nextActivityArrivalIndex).Double();
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				auto_ptr<mongo::DBClientCursor> tCostCar = mongoDao["tcost_car"]->queryDocument(bsonObj);
				BSONObj tCostCarDoc = tCostCar->next();
				travelTime = tCostCarDoc.getField("TT_car_arrival_" + nextActivityArrivalIndex).Double();
				break;
			}
			case 8:
			{
				BSONObjBuilder bsonObjBldr_Costs;
				bsonObjBldr_Costs << "origin" << currStop.getStopLocation() << "destin" << nextStop.getStopLocation();
				bsonObj = bsonObjBldr_Costs.obj();
				auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObj);
				auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObj);
				BSONObj amDistanceObj = amDistance->next();
				BSONObj pmDistanceObj = pmDistance->next();
				double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
				travelTime = distanceMin/5;
				break;
			}
			}
		}
		else {
			travelTime = 0.0;
		}
	}
	catch (exception& e) {
		// something went wrong in fetching data. Set travel time to a high value.
		travelTime = 999;
	}
	double currStopDepTime = nextStop.getArrivalTime() - travelTime;
	if((currStopDepTime - std::floor(currStopDepTime)) < 0.5) {
		currStopDepTime = std::floor(currStopDepTime) + 0.25;
	}
	else {
		currStopDepTime = std::floor(currStopDepTime) + 0.75;
	}
	currStop.setDepartureTime(currStopDepTime);*/
}

void PredaySystem::calculateTourStartTime(Tour& tour) {
	/*
	 * There are 48 half-hour time windows in a day from 3.25 to 26.75.
	 * Given a time window x, its choice index can be determined by ((x - 3.25) / 0.5) + 1
	 */
	/*Stop& firstStop = *(tour.getStops().front());
	uint32_t firstActivityArrivalIndex = ((firstStop.getArrivalTime() - 3.25)/0.5) +1;
	double travelTime;
	try {
		if(personParams.getHomeLocation() != firstStop.getStopLocation()) {
			BSONObjBuilder bsonObjBldr;
			bsonObjBldr << "origin" << personParams.getHomeLocation() << "destination" << firstStop.getStopLocation();
			BSONObj bsonObj = bsonObjBldr.obj();
			switch(firstStop.getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				auto_ptr<mongo::DBClientCursor> tCostBus = mongoDao["tcost_bus"]->queryDocument(bsonObj);
				BSONObj tCostBusDoc = tCostBus->next();
				travelTime = tCostBusDoc.getField("TT_bus_arrival_" + firstActivityArrivalIndex).Double();
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				auto_ptr<mongo::DBClientCursor> tCostCar = mongoDao["tcost_car"]->queryDocument(bsonObj);
				BSONObj tCostCarDoc = tCostCar->next();
				travelTime = tCostCarDoc.getField("TT_car_arrival_" + firstActivityArrivalIndex).Double();
				break;
			}
			case 8:
			{
				BSONObjBuilder bsonObjBldr_Costs;
				bsonObjBldr << "origin" << personParams.getHomeLocation() << "destin" << firstStop.getStopLocation();
				bsonObj = bsonObjBldr_Costs.obj();
				auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObj);
				auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObj);
				BSONObj amDistanceObj = amDistance->next();
				BSONObj pmDistanceObj = pmDistance->next();
				double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
				travelTime = distanceMin/5;
				break;
			}
			}
		}
		else {
			travelTime = 0.0;
		}

	}
	catch (exception& e) {
		// something went wrong in fetching data. Set travel time to a high value.
		travelTime = 999;
	}
	double tourStartTime = firstStop.getArrivalTime() - travelTime;
	if((tourStartTime - std::floor(tourStartTime)) < 0.5) {
		tourStartTime = std::floor(tourStartTime) + 0.25;
	}
	else {
		tourStartTime = std::floor(tourStartTime) + 0.75;
	}
	tour.setStartTime(tourStartTime);*/
}

void PredaySystem::calculateTourEndTime(Tour& tour) {
	/*
	 * There are 48 half-hour time windows in a day from 3.25 to 26.75.
	 * Given a time window x, its choice index can be determined by ((x - 3.25) / 0.5) + 1
	 */
	/*Stop& lastStop = *(tour.getStops().back());
	uint32_t prevActivityDepartureIndex = ((lastStop.getDepartureTime() - 3.25)/0.5) +1;
	double travelTime;
	try {
		if(personParams.getHomeLocation() != lastStop.getStopLocation()) {
			BSONObjBuilder bsonObjBldr;
			bsonObjBldr << "origin" << lastStop.getStopLocation() << "destination" << personParams.getHomeLocation();
			BSONObj bsonObj = bsonObjBldr.obj();

			switch(lastStop.getStopMode()) {
			case 1: // Fall through
			case 2:
			case 3:
			{
				auto_ptr<mongo::DBClientCursor> tCostBus = mongoDao["tcost_bus"]->queryDocument(bsonObj);
				BSONObj tCostBusDoc = tCostBus->next();
				travelTime = tCostBusDoc.getField("TT_bus_departure_" + prevActivityDepartureIndex).Double();
				break;
			}
			case 4: // Fall through
			case 5:
			case 6:
			case 7:
			case 9:
			{
				auto_ptr<mongo::DBClientCursor> tCostCar = mongoDao["tcost_car"]->queryDocument(bsonObj);
				BSONObj tCostCarDoc = tCostCar->next();
				travelTime = tCostCarDoc.getField("TT_car_departure_" + prevActivityDepartureIndex).Double();
				break;
			}
			case 8:
			{
				BSONObjBuilder bsonObjBldr_Costs;
				bsonObjBldr << "origin" << lastStop.getStopLocation() << "destin" << personParams.getHomeLocation();
				bsonObj = bsonObjBldr_Costs.obj();
				auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObj);
				auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObj);
				BSONObj amDistanceObj = amDistance->next();
				BSONObj pmDistanceObj = pmDistance->next();
				double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
				travelTime = distanceMin/5;
				break;
			}
			}
		}
		else {
			travelTime = 0.0;
		}
	}
	catch (exception& e) {
		// something went wrong in fetching data. Set travel time to a high value.
		travelTime = 999;
	}
	double tourEndTime = lastStop.getDepartureTime() + travelTime;
	if((tourEndTime - std::floor(tourEndTime)) < 0.5) {
		tourEndTime = std::floor(tourEndTime) + 0.25;
	}
	else {
		tourEndTime = std::floor(tourEndTime) + 0.75;
	}
	tour.setEndTime(tourEndTime);*/
}

void PredaySystem::constructTours(PersonParams& personParams) {
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
			Print() << "predictUsualWorkLocation()" << std::endl;
			attendsUsualWorkLocation = predictUsualWorkLocation(firstOfMultiple); // Predict if this tour is to a usual work location
			firstOfMultiple = false;
		}
		Tour* workTour = new Tour(WORK);
		workTour->setUsualLocation(attendsUsualWorkLocation);
		if(attendsUsualWorkLocation) {
			workTour->setPrimaryActivityLocation(personParams.getFixedWorkLocation());
			workTour->setPrimaryActivityLocationId(zoneIdLookup.at(personParams.getFixedWorkLocation()));
		}
		tours.push_back(workTour);
	}

	// Construct education tours
	for(int i=0; i<numTours["EduT"]; i++) {
		Tour* eduTour = new Tour(EDUCATION);
		eduTour->setUsualLocation(true); // Education tours are always to usual locations
		eduTour->setPrimaryActivityLocation(personParams.getFixedSchoolLocation());
		eduTour->setPrimaryActivityLocationId(zoneIdLookup.at(personParams.getFixedSchoolLocation()));
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
		tours.push_back(new Tour(SHOP));
	}

	// Construct other tours
	for(int i=0; i<numTours["OthersT"]; i++) {
		tours.push_back(new Tour(OTHER));
	}
}

void PredaySystem::planDay() {
	//Predict day pattern
	Print() << "predictDayPattern(" << personParams.getPersonId() << "): " ;
	PredayLuaProvider::getPredayModel().predictDayPattern(personParams, dayPattern);

	//Predict number of Tours
	if(dayPattern.size() <= 0) {
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
	}
	Print() << "predictNumTours(" << personParams.getPersonId() << "): ";
	PredayLuaProvider::getPredayModel().predictNumTours(personParams, dayPattern, numTours);

	//Construct tours
	constructTours(personParams);

	//Process each tour
	Print() << "Tours: " << tours.size() << std::endl;
	for(std::deque<Tour*>::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		Tour& tour = *(*tourIt);
		if(tour.isUsualLocation()) {
			// Predict just the mode for tours to usual location
			Print() << "predictTourMode()" << std::endl;
			predictTourMode(tour);
		}
		else {
			// Predict mode and destination for tours to not-usual locations
			Print() << "predictTourModeDestination()" << std::endl;
			predictTourModeDestination(tour);
		}

		// Predict the time of day for this tour
		Print() << "predictTourTimeOfDay()" << std::endl;
		TimeWindowAvailability timeWindow = predictTourTimeOfDay(tour);
		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true, true);
		primaryActivity->setStopMode(tour.getTourMode());
		primaryActivity->setStopLocation(tour.getPrimaryActivityLocation());
		primaryActivity->setStopLocationId(tour.getPrimaryActivityLocationId());
		primaryActivity->allotTime(timeWindow.getStartTime(), timeWindow.getEndTime());
		tour.addStop(primaryActivity);
		personParams.blockTime(timeWindow.getStartTime(), timeWindow.getEndTime());

		//Generate stops for this tour
		generateIntermediateStops(tour);

		calculateTourStartTime(tour);
		calculateTourEndTime(tour);
		personParams.blockTime(tour.getStartTime(), tour.getEndTime());
	}
}
