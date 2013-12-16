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
#include "behavioral/params/TourModeDestinationParams.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "database/DB_Connection.hpp"
#include "logging/Log.hpp"
#include "mongo/client/dbclient.h"


using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace mongo;

PredaySystem::PredaySystem(PersonParams& personParams, boost::unordered_map<int, ZoneParams>& zoneMap)
: personParams(personParams), zoneMap(zoneMap), predayLuaModel(PredayLuaProvider::getPredayModel()) // get the thread-local lua model object
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

bool sim_mob::medium::PredaySystem::predictUsualWorkLocation(PersonParams& personParams, bool firstOfMultiple) {
    UsualWorkParams usualWorkParams;
	usualWorkParams.setFirstOfMultiple((int) firstOfMultiple);
	usualWorkParams.setSubsequentOfMultiple((int) !firstOfMultiple);
	BSONObj queryObj = BSON("origin" << personParams.getHomeLocation() << "destin" << personParams.getFixedWorkLocation());
	BSONObj resultObjAM, resultObjPM, resultObjZone;
	mongoDao["AMCosts"]->getOne(queryObj, resultObjAM);
	mongoDao["PMCosts"]->getOne(queryObj, resultObjPM);
	usualWorkParams.setWalkDistanceAm(resultObjAM.getField("distance").Double());
	usualWorkParams.setWalkDistanceAm(resultObjPM.getField("distance").Double());
	queryObj = BSON("zone_code" << personParams.getFixedWorkLocation());
	mongoDao["Zone"]->getOne(queryObj, resultObjZone);
	usualWorkParams.setZoneEmployment(resultObjZone.getField("employment").Double());
	return predayLuaModel.predictUsualWorkLocation(personParams, usualWorkParams);
}

void PredaySystem::predictTourMode(Tour& tour) {
	TourModeParams tmParams;
	tmParams.setStopType(tour.getTourType());
	BSONObj znOrgObj, znDesObj;
	BSONObj znOrgQuery = BSON("zone_code" << personParams.getHomeLocation());
	BSONObj znDesQuery = BSON("zone_code" << tour.getPrimaryActivityLocation());
	Print() << "zone_code" << tour.getPrimaryActivityLocation() << std::endl;
	mongoDao["Zone"]->getOne(znOrgQuery, znOrgObj);
	mongoDao["Zone"]->getOne(znDesQuery, znDesObj);
	tmParams.setCostCarParking(znDesObj.getField("parking_rate").Double());
	tmParams.setCentralZone(znDesObj.getField("central_dummy").Double());
	tmParams.setResidentSize(znOrgObj.getField("resident_workers").Double());
	tmParams.setWorkOp(znDesObj.getField("employment").Double());
	tmParams.setEducationOp(znDesObj.getField("total_enrollment").Double());
	tmParams.setOriginArea(znOrgObj.getField("area").Double());
	tmParams.setDestinationArea(znDesObj.getField("area").Double());
	if(personParams.getHomeLocation() != tour.getPrimaryActivityLocation()) {
		BSONObj amObj, pmObj;
		BSONObj amQuery = BSON("origin" << personParams.getHomeLocation() << "destin" << tour.getPrimaryActivityLocation());
		BSONObj pmQuery = BSON("origin" << tour.getPrimaryActivityLocation()  << "destin" << personParams.getHomeLocation());
		mongoDao["AMCosts"]->getOne(amQuery, amObj);
		mongoDao["PMCosts"]->getOne(pmQuery, pmObj);
		tmParams.setCostPublicFirst(amObj.getField("pub_cost").Double());
		tmParams.setCostPublicSecond(pmObj.getField("pub_cost").Double());
		tmParams.setCostCarErpFirst(amObj.getField("car_cost_erp").Double());
		tmParams.setCostCarErpSecond(pmObj.getField("car_cost_erp").Double());
		tmParams.setCostCarOpFirst(amObj.getField("distance").Double() * 0.147);
		tmParams.setCostCarOpSecond(pmObj.getField("distance").Double() * 0.147);
		tmParams.setWalkDistance1(amObj.getField("distance").Double());
		tmParams.setWalkDistance2(pmObj.getField("distance").Double());
		tmParams.setTtPublicIvtFirst(amObj.getField("pub_ivt").Double());
		tmParams.setTtPublicIvtSecond(pmObj.getField("pub_ivt").Double());
		tmParams.setTtPublicWaitingFirst(amObj.getField("pub_wtt").Double());
		tmParams.setTtPublicWaitingSecond(pmObj.getField("pub_wtt").Double());
		tmParams.setTtPublicWalkFirst(amObj.getField("pub_walkt").Double());
		tmParams.setTtPublicWalkSecond(pmObj.getField("pub_walkt").Double());
		tmParams.setTtCarIvtFirst(amObj.getField("car_ivt").Double());
		tmParams.setTtCarIvtSecond(pmObj.getField("car_ivt").Double());
		tmParams.setAvgTransfer((amObj.getField("avg_transfer").Double() + pmObj.getField("avg_transfer").Double())/2);
		switch(tmParams.getStopType()){
		case WORK:
			tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
			tmParams.setShare2Available(1);
			tmParams.setShare3Available(1);
			tmParams.setPublicBusAvailable(amObj.getField("pub_ivt").Double() > 0 && pmObj.getField("pub_ivt").Double() > 0);
			tmParams.setMrtAvailable(amObj.getField("pub_ivt").Double() > 0 && pmObj.getField("pub_ivt").Double() > 0);
			tmParams.setPrivateBusAvailable(amObj.getField("pub_ivt").Double() > 0 && pmObj.getField("pub_ivt").Double() > 0);
			tmParams.setWalkAvailable(amObj.getField("pub_ivt").Double() <= 3 && pmObj.getField("pub_ivt").Double() <= 3);
			tmParams.setTaxiAvailable(1);
			tmParams.setMotorAvailable(1);
			break;
		case EDUCATION:
			tmParams.setDrive1Available(personParams.hasDrivingLicence() * personParams.getCarOwnNormal());
			tmParams.setShare2Available(1);
			tmParams.setShare3Available(1);
			tmParams.setPublicBusAvailable(1);
			tmParams.setMrtAvailable(1);
			tmParams.setPrivateBusAvailable(1);
			tmParams.setWalkAvailable(1);
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

	tour.setTourMode(predayLuaModel.predictTourMode(personParams, tmParams));
}

void PredaySystem::predictTourModeDestination(Tour& tour) {
	TourModeDestinationParams tmdParams(zoneMap, personParams, tour.getTourType());
	int modeDest = predayLuaModel.predictTourModeDestination(personParams, tmdParams);
	tour.setTourMode(tmdParams.getMode_TMD(modeDest));
	tour.setPrimaryActivityLocation(tmdParams.getDestination_TMD(modeDest));
}

std::string& PredaySystem::predictTourTimeOfDay(Tour& tour) {
	string dummy = "";
	std::string& timeWndw = dummy;
/*	if(!tour.isSubTour()) {
		long origin = personParams.getHomeLocation();
		long destination = tour.getPrimaryActivityLocation();
		TourTimeOfDayParams todParams;
		if(origin != destination) {
			BSONObjBuilder bsonObjBldr_ttAM;
			bsonObjBldr_ttAM << "origin" << origin << "destination" << destination;
			BSONObj bsonObj_tAM = bsonObjBldr_ttAM.obj();

			BSONObjBuilder bsonObjBldr_ttPM;
			bsonObjBldr_ttPM << "origin" << destination << "destination" << origin;
			BSONObj bsonObj_tPM = bsonObjBldr_ttPM.obj();

			BSONObjBuilder bsonObjBldr_AMCosts;
			bsonObjBldr_AMCosts << "origin" << origin << "destin" << destination; // origin is home and destination is primary activity location
			BSONObj bsonObj_AMCosts = bsonObjBldr_AMCosts.obj();

			BSONObjBuilder bsonObjBldr_PMCosts;
			bsonObjBldr_PMCosts << "origin" << destination << "destin" << origin; // origin is primary activity location and destination is home
			BSONObj bsonObj_PMCosts = bsonObjBldr_PMCosts.obj();

			auto_ptr<mongo::DBClientCursor> tCostBusAM = mongoDao["tcost_bus"]->queryDocument(bsonObj_tAM);
			auto_ptr<mongo::DBClientCursor> tCostBusPM = mongoDao["tcost_bus"]->queryDocument(bsonObj_tPM);
			BSONObj tCostBusDocAM = tCostBusAM->next();
			BSONObj tCostBusDocPM = tCostBusPM->next();

			auto_ptr<mongo::DBClientCursor> tCostCarAM = mongoDao["tcost_car"]->queryDocument(bsonObj_tAM);
			auto_ptr<mongo::DBClientCursor> tCostCarPM = mongoDao["tcost_car"]->queryDocument(bsonObj_tPM);
			BSONObj tCostCarDocAM = tCostCarAM->next();
			BSONObj tCostCarDocPM = tCostCarPM->next();

			auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObj_AMCosts);
			auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObj_PMCosts);
			BSONObj amDistanceObj = amDistance->next();
			BSONObj pmDistanceObj = pmDistance->next();

			try {
				for (uint32_t i=1; i<=48; i++) {
					switch(tour.getTourMode()) {
					case 1: // Fall through
					case 2:
					case 3:
					{
						todParams.travelTimesFirstHalfTour.push_back(tCostBusDocAM.getField("TT_bus_arrival_" + i).Double());
						todParams.travelTimesSecondHalfTour.push_back(tCostBusDocPM.getField("TT_bus_arrival_" + i).Double());
						break;
					}
					case 4: // Fall through
					case 5:
					case 6:
					case 7:
					case 9:
					{
						todParams.travelTimesFirstHalfTour.push_back(tCostCarDocAM.getField("TT_car_arrival_" + i).Double());
						todParams.travelTimesSecondHalfTour.push_back(tCostCarDocPM.getField("TT_car_arrival_" + i).Double());
						break;
					}
					case 8:
					{
						double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
						todParams.travelTimesSecondHalfTour.push_back(distanceMin/5);
						todParams.travelTimesSecondHalfTour.push_back(distanceMin/5);
						break;
					}
					}
				}
			}
			catch(exception& e) {
				// something unexpected happened while getting data
				// retrieved values could've been NULL
				// therefore, setting the travel times to a high value - 999
				Print() << "Error occurred in predictTourTimeOfDay(): " + e.what() << std::endl;
				todParams.travelTimesSecondHalfTour.push_back(999);
				todParams.travelTimesSecondHalfTour.push_back(999);
			}
		}
		LuaRef chooseTTOD = getGlobal(state.get(), "chooseTTOD");
		LuaRef retVal = chooseTTOD(personParams, todParams);
		timeWndw = retVal.cast<std::string>();
		personParams.blockTime(timeWndw);

	}*/
	return timeWndw;
}

void PredaySystem::generateIntermediateStops(Tour& tour) {
	if(tour.stops.size() != 1) {
		stringstream ss;
		ss << "generateIntermediateStops()|tour object contains " << tour.stops.size() << " stops. 1 stop (primary activity) was expected.";
		throw runtime_error(ss.str());
	}
	Stop* primaryStop = tour.stops.front(); // The only stop at this point is the primary activity stop
	Stop* generatedStop = nullptr;

	if ((dayPattern.at("WorkI") + dayPattern.at("EduI") + dayPattern.at("ShopI") + dayPattern.at("OthersI")) > 0 ) {
		//if any stop type was predicted in the day pattern
		StopGenerationParams isgParams(tour, primaryStop);
		int origin = personParams.getHomeLocation();
		int destination = primaryStop->getStopLocation();
		isgParams.setFirstTour(tours.front() == &tour);
		std::deque<Tour*>::iterator tourIt = std::find(tours.begin(), tours.end(), &tour);
		size_t index = std::distance(tours.begin(), tourIt);
		isgParams.setNumRemainingTours(tours.size() - index + 1);

		//First half tour
		if(origin != destination) {
			mongo::BSONObj bsonObj_AMCosts = BSON("origin" << origin << "destin" << destination); // origin is home and destination is primary activity location
			mongo::BSONObj amDistanceObj;
			mongoDao["AMCosts"]->getOne(bsonObj_AMCosts, amDistanceObj);
			isgParams.setDistance(amDistanceObj.getField("distance").Double());
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
			choice = predayLuaModel.generateIntermediateStop(personParams, isgParams);
			if(choice != 5) {
				StopType stopType;
				if(choice == 1) { stopType = WORK; }
				else if (choice == 2) { stopType = EDUCATION; }
				else if (choice == 3) { stopType = SHOP; }
				else if (choice == 4) { stopType = OTHER; }
				Stop* generatedStop = new Stop(stopType, tour, false /*not primary*/, true /*in first half tour*/);
				tour.addStop(generatedStop);
				predictStopModeDestination(generatedStop);
				calculateDepartureTime(generatedStop, nextStop);
				if(generatedStop->getDepartureTime() <= 3.25)
				{
					tour.removeStop(generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				predictStopTimeOfDay(generatedStop, true);
				nextStop = generatedStop;
				personParams.blockTime(generatedStop->getArrivalTime(), generatedStop->getDepartureTime());
				nextArrivalTime = generatedStop->getArrivalTime();
				stopCounter = stopCounter + 1;
			}
		}

		// Second half tour
		if(origin != destination) {
			mongo::BSONObj bsonObj_PMCosts = BSON("origin" << destination << "destin" << origin); // origin is home and destination is primary activity location)
			mongo::BSONObj pmDistanceObj;
			mongoDao["PMCosts"]->getOne(bsonObj_PMCosts, pmDistanceObj);
			isgParams.setDistance(pmDistanceObj.getField("distance").Double());
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
			choice = predayLuaModel.generateIntermediateStop(personParams, isgParams);
			if(choice != 5) {
				StopType stopType;
				if(choice == 1) { stopType = WORK; }
				else if (choice == 2) { stopType = EDUCATION; }
				else if (choice == 3) { stopType = SHOP; }
				else if (choice == 4) { stopType = OTHER; }
				Stop* generatedStop = new Stop(stopType, tour, false /*not primary*/, false  /*not in first half tour*/);
				tour.addStop(generatedStop);
				predictStopModeDestination(generatedStop);
				calculateArrivalTime(generatedStop, prevStop);
				if(generatedStop->getDepartureTime() >=  26.75)
				{
					tour.removeStop(generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				predictStopTimeOfDay(generatedStop, false);
				prevStop = generatedStop;
				personParams.blockTime(generatedStop->getArrivalTime(), generatedStop->getDepartureTime());
				prevDepartureTime = generatedStop->getDepartureTime();
				stopCounter = stopCounter + 1;
			}
		}
	}
}

void PredaySystem::predictStopModeDestination(Stop* stop)
{}

void PredaySystem::predictStopTimeOfDay(Stop* stop, bool isBeforePrimary) {
	/*StopTimeOfDayParams stodParams(stop.getStopTypeID(), isBeforePrimary);
	double origin = stop.getStopLocation();
	double destination = personParams.getHomeLocation();

	if(origin != destination) {
		BSONObjBuilder bsonObjBldrTT, bsonObjBldrTC;
		bsonObjBldrTT << "origin" << origin << "destination" << destination;
		BSONObj bsonObjTT = bsonObjBldrTT.obj();

		bsonObjBldrTC << "origin" << destination << "destin" << origin;
		BSONObj bsonObjTC = bsonObjBldrTC.obj();

		try {
			auto_ptr<mongo::DBClientCursor> tCostBusCursor = mongoDao["tcost_bus"]->queryDocument(bsonObjTT);
			BSONObj tCostBusDoc = tCostBusCursor->next();

			auto_ptr<mongo::DBClientCursor> tCostCarCursor = mongoDao["tcost_car"]->queryDocument(bsonObjTT);
			BSONObj tCostCarDoc = tCostCarCursor->next();

			auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"]->queryDocument(bsonObjTC);
			auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"]->queryDocument(bsonObjTC);
			BSONObj amDistanceObj = amDistance->next();
			BSONObj pmDistanceObj = pmDistance->next();

			for (uint32_t i = 1; i <= 48; i++) {
				switch (stop.getStopMode()) {
				case 1: // Fall through
				case 2:
				case 3:
				{
					if(stodParams.getFirstBound()) {
						stodParams.travelTimes.push_back(tCostBusDoc.getField("TT_bus_arrival_" + i).Double());
					}
					else {
						stodParams.travelTimes.push_back(tCostBusDoc.getField("TT_bus_departure_" + i).Double());
					}
					break;
				}
				case 4: // Fall through
				case 5:
				case 6:
				case 7:
				case 9:
				{
					if(stodParams.getFirstBound()) {
						stodParams.travelTimes.push_back(tCostCarDoc.getField("TT_car_arrival_" + i).Double());
					}
					else {
						stodParams.travelTimes.push_back(tCostCarDoc.getField("TT_car_departure_" + i).Double());
					}
					break;
				}
				case 8: {
					double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
					stodParams.travelTimes.push_back(distanceMin/5);
					break;
				}
				}
			}
		}
		catch (exception& e) {
			// something unexpected happened while getting data
			// retrieved values could've been NULL
			// therefore, setting the travel times to a high value - 999
			Print() << "Error occurred in predictStopTimeOfDay(): " + e.what() << std::endl;
			stodParams.travelTimes.push_back(999);
			stodParams.travelTimes.push_back(999);
		}
	}
	else {
		for(int i=1; i<=48; i++) {
			stodParams.travelTimes.push_back(0.0);
		}
	}

	// high and low tod
	if(isBeforePrimary) {
		stodParams.setTodHigh(stop.getDepartureTime());
		if(stop.getParentTour() == tours.front()) {
			stodParams.setTodLow(3.25);
		}
		else {
			Tour* prevTour = *(std::find(tours.begin(), tours.end(), stop.getParentTour()) - 1);
			stodParams.setTodLow(prevTour->getEndTime());

		}
	}
	else {
		stodParams.setTodLow(stop.getArrivalTime());
		stodParams.setTodHigh(26.75);
	}

	// calculate costs
	BSONObjBuilder bsonObjBldr_costs;
	bsonObjBldr_costs << "origin" << origin << "destin" << destination;
	BSONObj costObj = bsonObjBldr_costs.obj();

	auto_ptr<mongo::DBClientCursor> amCursor = mongoDao["AMCosts"]->queryDocument(costObj);
	auto_ptr<mongo::DBClientCursor> pmCursor = mongoDao["PMCosts"]->queryDocument(costObj);
	auto_ptr<mongo::DBClientCursor> opCursor = mongoDao["OPCosts"]->queryDocument(costObj);
	BSONObj amDoc = amCursor->next();
	BSONObj pmDoc = pmCursor->next();
	BSONObj opDoc = opCursor->next();

	BSONObjBuilder bsonObjBldr_zone;
	bsonObjBldr_zone << "zone_code" << origin;
	BSONObj zoneObj = bsonObjBldr_zone.obj();

	auto_ptr<mongo::DBClientCursor> zoneCursor = mongoDao["Zone"]->queryDocument(zoneObj);
	BSONObj zoneDoc = zoneCursor->next();

	double duration, parkingRate, costCarParking, costCarERP, costCarOP, walkDistance;
	for(int i=1; i<=48; i++) {

		if(stodParams.getFirstBound()) {
			duration = stodParams.getTodHigh() - i + 1;
		}
		else { //if(stodParams.getSecondBound())
			duration = i - stodParams.getTodLow() + 1;
		}
		duration = 0.25+(duration-1)*0.5;
		parkingRate = zoneDoc.getField("parking_rate").Double();
		costCarParking = (8*(duration>8)+duration*(duration<=8))*parkingRate;

		if(i >= 10 && i <= 14) { // time window indexes 10 to 14 are AM Peak windows
			costCarERP = amDoc.getField("car_cost_erp").Double();
			costCarOP = amDoc.getField("distance").Double() * 0.147;
			walkDistance = amDoc.getField("distance").Double();
		}
		else if (i >= 30 && i <= 34) { // time window indexes 30 to 34 are PM Peak indexes
			costCarERP = pmDoc.getField("car_cost_erp").Double();
			costCarOP = pmDoc.getField("distance").Double() * 0.147;
			walkDistance = pmDoc.getField("distance").Double();
		}
		else { // other time window indexes are Off Peak indexes
			costCarERP = opDoc.getField("car_cost_erp").Double();
			costCarOP = opDoc.getField("distance").Double() * 0.147;
			walkDistance = opDoc.getField("distance").Double();
		}

		switch (stop.getStopMode()) {
		case 1: // Fall through
		case 2:
		case 3:
		{
			if(i >= 10 && i <= 14) { // time window indexes 10 to 14 are AM Peak windows
				stodParams.travelCost.push_back(amDoc.getField("pub_cost").Double());
			}
			else if (i >= 30 && i <= 34) { // time window indexes 30 to 34 are PM Peak indexes
				stodParams.travelCost.push_back(pmDoc.getField("pub_cost").Double());
			}
			else { // other time window indexes are Off Peak indexes
				stodParams.travelCost.push_back(opDoc.getField("pub_cost").Double());
			}
			break;
		}
		case 4: // Fall through
		case 5:
		case 6:
		{
			stodParams.travelCost.push_back((costCarParking+costCarOP+costCarERP)/(stop.getStopMode()-3.0));
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

	for(double i=1; i<=48; i++) {
		double timeWndw = (i-1)*0.5 + 3.25;
		if(timeWndw <= stodParams.getTodLow() || timeWndw >= stodParams.getTodHigh()) {
			stodParams.availability[i] = false;
		}
	}

	LuaRef chooseITOD = getGlobal(state.get(), "choose_itd");
	LuaRef retVal = chooseITOD(personParams, stodParams);
	double timeWndw = retVal.cast<double>();
	if(isBeforePrimary) {
		stop.setArrivalTime(timeWndw);
	}
	else {
		stop.setDepartureTime(timeWndw);
	}*/
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
			attendsUsualWorkLocation = predictUsualWorkLocation(personParams, firstOfMultiple); // Predict if this tour is to a usual work location
			firstOfMultiple = false;
		}
		Tour* workTour = new Tour(WORK);
		workTour->setUsualLocation(attendsUsualWorkLocation);
		if(attendsUsualWorkLocation) {
			workTour->setPrimaryActivityLocation(personParams.getFixedWorkLocation());
		}
		tours.push_back(workTour);
	}

	// Construct education tours
	for(int i=0; i<numTours["EduT"]; i++) {
		Tour* eduTour = new Tour(EDUCATION);
		eduTour->setUsualLocation(true); // Education tours are always to usual locations
		eduTour->setPrimaryActivityLocation(personParams.getFixedSchoolLocation());
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
	personParams.setWorkLogSum(0.0);
	personParams.setEduLogSum(0.0);
	personParams.setShopLogSum(0.0);
	personParams.setOtherLogSum(0.0);

	Print() << "predictDayPattern(" << personParams.getPersonId() << "): " ;
	predayLuaModel.predictDayPattern(personParams, dayPattern);

	//Predict number of Tours
	if(dayPattern.size() <= 0) {
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
	}
	Print() << "predictNumTours(" << personParams.getPersonId() << "): ";
	predayLuaModel.predictNumTours(personParams, dayPattern, numTours);

	//Construct tours
	constructTours();

	//Process each tour
	for(std::deque<Tour*>::iterator tourIt=tours.begin(); tourIt!=tours.end(); tourIt++) {
		Tour& tour = *(*tourIt);
		if(tour.isUsualLocation()) {
			// Predict just the mode for tours to usual location
			predictTourMode(tour);
		}
		else {
			// Predict mode and destination for tours to not-usual locations
			predictTourModeDestination(tour);
		}

		// Predict the time of day for this tour
		std::string timeWindow = predictTourTimeOfDay(tour);
		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true, true);
		primaryActivity->setStopMode(tour.getTourMode());
		primaryActivity->setStopLocation(tour.getPrimaryActivityLocation());
		primaryActivity->allotTime(timeWindow);
		tour.addStop(primaryActivity);
		personParams.blockTime(timeWindow);

		//Generate stops for this tour
		generateIntermediateStops(tour);

		calculateTourStartTime(tour);
		calculateTourEndTime(tour);
		personParams.blockTime(tour.getStartTime(), tour.getEndTime());
	}
}
