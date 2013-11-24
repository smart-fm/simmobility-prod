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
#include <cstdlib>
#include "conf/ConfigManager.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "mongo/client/dbclient.h"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace luabridge;
using namespace mongo;

PredaySystem::PredaySystem(PersonParams& personParams) : personParams(personParams), usualWorkParams() {
	MongoCollectionsMap mongoColl = ConfigManager::GetInstance().FullConfig().constructs.mongoCollectionsMap.at("preday_mongo");
	Database db = ConfigManager::GetInstance().FullConfig().constructs.databases.at("fm_mongo");
	for(std::map<std::string, std::string>::const_iterator i=mongoColl.collectionName.begin(); i!=mongoColl.collectionName.end(); i++) {
		db::DB_Config dbConfig(db.host, db.port, db.dbName);
		db::DB_Connection dbConn(db::BackendType::MONGO_DB, dbConfig);
		mongoDao[i->first]=db::MongoDao(dbConn, db.dbName, i->second);
	}
	modeReferenceIndex[1] = "Public bus";
	modeReferenceIndex[2] = "MRT";
	modeReferenceIndex[3] = "Private Bus";
	modeReferenceIndex[4] = "Auto";
	modeReferenceIndex[5] = "Share 2+";
	modeReferenceIndex[6] = "Share 3+";
	modeReferenceIndex[7] = "Motor";
	modeReferenceIndex[8] = "Walk";
}

PredaySystem::~PredaySystem()
{}

void PredaySystem::mapClasses() {
	getGlobalNamespace(state.get())
			.beginClass <PersonParams> ("PersonParams")
			.addProperty("person_type_id", &PersonParams::getPersonTypeId)
			.addProperty("age_id", &PersonParams::getAgeId)
			.addProperty("universitystudent", &PersonParams::getIsUniversityStudent)
			.addProperty("female_dummy", &PersonParams::getIsFemale)
			.addProperty("income_id", &PersonParams::getIncomeId)
			.addProperty("work_from_home_dummy", &PersonParams::getWorksAtHome)
			.addProperty("car_own_normal", &PersonParams::getCarOwnNormal)
			.addProperty("car_own_offpeak", &PersonParams::getCarOwnOffpeak)
			.addProperty("motor_own", &PersonParams::getMotorOwn)
			.addProperty("fixed_work_hour", &PersonParams::getHasFixedWorkTiming)
			.addProperty("homeLocation", &PersonParams::getHomeLocation)
			.addProperty("fixed_place", &PersonParams::getFixedWorkLocation)
			.addProperty("fixedSchoolLocation", &PersonParams::getFixedSchoolLocation)
			.addProperty("only_adults", &PersonParams::getHH_OnlyAdults)
			.addProperty("only_workers", &PersonParams::getHH_OnlyWorkers)
			.addProperty("num_underfour", &PersonParams::getHH_NumUnder4)
			.addProperty("presence_of_under15", &PersonParams::getHH_NumUnder15)
			.addProperty("worklogsum", &PersonParams::getWorkLogSum)
			.addProperty("edulogsum", &PersonParams::getEduLogSum)
			.addProperty("shoplogsum", &PersonParams::getShopLogSum)
			.addProperty("otherlogsum", &PersonParams::getOtherLogSum)
			.addFunction("getTimeWindowAvailability", &PersonParams::getTimeWindowAvailability)
			.endClass();
}

void PredaySystem::predictDayPattern() {
	LuaRef chooseDP = getGlobal(state.get(), "choose_dp");
	LuaRef retVal = chooseDP(personParams);
	if (retVal.isTable()) {
		dayPattern["WorkT"] = retVal[1].cast<bool>();
		dayPattern["EduT"] = retVal[2].cast<bool>();
		dayPattern["ShopT"] = retVal[3].cast<bool>();
		dayPattern["OthersT"] = retVal[4].cast<bool>();
		dayPattern["WorkI"] = retVal[5].cast<bool>();
		dayPattern["EduI"] = retVal[6].cast<bool>();
		dayPattern["ShopI"] = retVal[7].cast<bool>();
		dayPattern["OthersI"] = retVal[8].cast<bool>();
	}
	else {
		throw std::runtime_error("Error in day pattern prediction. Unexpected return value");
	}
}

void PredaySystem::predictNumTours() {
	if(dayPattern.size() <= 0) {
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
	}
	numTours["WorkT"] = numTours["EduT"] = numTours["ShopT"] = numTours["OthersT"] = 0;

	if(dayPattern["WorkT"]) {
		LuaRef chooseNTW = getGlobal(state.get(), "choose_ntw"); // choose Num. of Work Tours
		LuaRef retVal = chooseNTW(personParams);
	    if (retVal.isNumber()) {
	    	numTours["WorkT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["EduT"]) {
		LuaRef chooseNTE = getGlobal(state.get(), "choose_nte");// choose Num. of Education Tours
		LuaRef retVal = chooseNTE(personParams);
	    if (retVal.isNumber()) {
	    	numTours["EduT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["ShopT"]) {
		LuaRef chooseNTS = getGlobal(state.get(), "choose_nts");// choose Num. of Shopping Tours
		LuaRef retVal = chooseNTS(personParams);
	    if (retVal.isNumber()) {
	    	numTours["ShopT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["OthersT"]) {
		LuaRef chooseNTO = getGlobal(state.get(), "choose_nto");// choose Num. of Other Tours
		LuaRef retVal = chooseNTO(personParams);
	    if (retVal.isNumber()) {
	    	numTours["OthersT"] = retVal.cast<int>();
	    }
	}
}

bool PredaySystem::predictUsualWorkLocation() {
	LuaRef chooseUW = getGlobal(state.get(), "choose_uw"); // choose usual work location
	LuaRef retVal = chooseUW(personParams, usualWorkParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	return retVal.cast<bool>();
}

void PredaySystem::predictTourMode(Tour& tour) {
	LuaRef chooseMode = getGlobal(state.get(), "chooseMode"); // choose usual work location
	LuaRef retVal = chooseMode(personParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	std::string& mode = modeReferenceIndex.at(retVal.cast<int>());
	tour.setTourMode(mode);
}

void PredaySystem::predictTourModeDestination(Tour& tour) {

	LuaRef chooseTMD;
	switch (tour.getTourType()) {
	case WORK:
		personParams.setStopType(1);
		chooseTMD = getGlobal(state.get(), "chooseTMDW"); // tour mode destination for work tour
		break;
	case EDUCATION:
		personParams.setStopType(2);
		chooseTMD = getGlobal(state.get(), "chooseTMDE"); // tour mode destination for work tour
		break;
	case SHOP:
		personParams.setStopType(3);
		chooseTMD = getGlobal(state.get(), "chooseTMDS"); // tour mode destination for work tour
		break;
	case OTHER:
		personParams.setStopType(4);
		chooseTMD = getGlobal(state.get(), "chooseTMDO"); // tour mode destination for work tour
		break;
	}
	LuaRef retVal = chooseTMD(personParams);
	if (!retVal.isString()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	std::string& modeDest = retVal.cast<std::string>();
	std::vector<std::string> strs;
	boost::split(strs, modeDest, boost::is_any_of(","));
	tour.setTourMode(strs.front());
	tour.setPrimaryActivityLocation(atol(strs.back().c_str()));
}

std::string& PredaySystem::predictTourTimeOfDay(Tour& tour) {
	std::string& timeWndw;
	if(!tour.isSubTour()) {
		long origin = personParams.getHomeLocation();
		long destination = tour.getPrimaryActivityLocation();
		sim_mob::medium::TimeOfDayParams todParams;
		if(origin != destination) {
			BSONObjBuilder bsonObjBldr_ttAM;
			bsonObjBldr_ttAM << "origin" << origin << "destination" << destination;
			BSONObj bsonObj_tAM = bsonObjBldr_ttAM.obj();

			BSONObjBuilder bsonObjBldr_ttPM;
			bsonObjBldr_ttPM << "origin" << origin << "destination" << destination;
			BSONObj bsonObj_tPM = bsonObjBldr_ttPM.obj();

			BSONObjBuilder bsonObjBldr_AMCosts;
			bsonObjBldr_AMCosts << "origin" << origin << "destin" << destination; // origin is home and destination is primary activity location
			BSONObj bsonObj_AMCosts = bsonObjBldr_AMCosts.obj();

			BSONObjBuilder bsonObjBldr_PMCosts;
			bsonObjBldr_PMCosts << "origin" << destination << "destin" << origin; // origin is primary activity location and destination is home
			BSONObj bsonObj_PMCosts = bsonObjBldr_PMCosts.obj();

			try {
				for (uint32_t i=1; i<=48; i++) {
					switch(tour.getTourMode()) {
					case 1: // Fall through
					case 2:
					case 3:
					{
						auto_ptr<mongo::DBClientCursor> tCostBusAM = mongoDao["tcost_bus"].queryDocument(bsonObj_tAM);
						auto_ptr<mongo::DBClientCursor> tCostBusPM = mongoDao["tcost_bus"].queryDocument(bsonObj_tPM);
						BSONObj tCostBusDocAM = tCostBusAM->next();
						BSONObj tCostBusDocPM = tCostBusPM->next();
						todParams.travelTimesFirstHalfTour.push_back((int) tCostBusDocAM.getField("TT_bus_arrival_" + i));
						todParams.travelTimesSecondHalfTour.push_back((int) tCostBusDocPM.getField("TT_bus_arrival_" + i));
						break;
					}
					case 4: // Fall through
					case 5:
					case 6:
					case 7:
					{
						auto_ptr<mongo::DBClientCursor> tCostCarAM = mongoDao["tcost_car"].queryDocument(bsonObj_tAM);
						auto_ptr<mongo::DBClientCursor> tCostCarPM = mongoDao["tcost_car"].queryDocument(bsonObj_tPM);
						BSONObj tCostCarDocAM = tCostCarAM->next();
						BSONObj tCostCarDocPM = tCostCarPM->next();
						todParams.travelTimesFirstHalfTour.push_back((int) tCostCarDocAM.getField("TT_car_arrival_" + i));
						todParams.travelTimesSecondHalfTour.push_back((int) tCostCarDocPM.getField("TT_car_arrival_" + i));
						break;
					}
					case 8:
					{
						auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"].queryDocument(bsonObj_AMCosts);
						auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"].queryDocument(bsonObj_PMCosts);
						BSONObj amDistanceObj = amDistance->next();
						BSONObj pmDistanceObj = pmDistance->next();
						double distanceMin = amDistanceObj.getField("distance").Double() - pmDistanceObj.getField("distance").Double();
						todParams.travelTimesSecondHalfTour.push_back((int) (distanceMin/5));
						todParams.travelTimesSecondHalfTour.push_back((int) (distanceMin/5));
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

	}
	return timeWndw;
}

void PredaySystem::generateIntermediateStops(Tour& tour) {
	deque<Stop*>& stops = tour.getStops();
	if(stops.size() != 1) {
		throw runtime_error("generateIntermediateStops()|tour object contains " + stops.size() + " stops. 1 stop (primary activity) was expected.");
	}
	Stop* primaryStop = tour.getStops().front(); // The only stop at this point is the primary activity stop
	Stop* generatedStop = nullptr;

	if ((dayPattern.at("WorkI") + dayPattern.at("EduI") + dayPattern.at("ShopI") + dayPattern.at("OtherI")) > 0 ) {
		//if any stop type was predicted in the day pattern
		StopGenerationParams isgParams(personParams, tour, primaryStop);
		long origin = personParams.getHomeLocation();
		long destination = primaryStop->getStopLocation();
		isgParams.setFirstTour(*(tours.front()) == tour);
		std::deque<Tour*>::iterator tourIt = std::find(tours.begin(), tours.end(), &tour);
		size_t index = std::distance(tours.begin(), tourIt);
		isgParams.setNumRemainingTours(tours.size() - index + 1);

		//First half tour
		BSONObjBuilder bsonObjBldr_AMCosts;
		bsonObjBldr_AMCosts << "origin" << origin << "destin" << destination; // origin is home and destination is primary activity location
		BSONObj bsonObj_AMCosts = bsonObjBldr_AMCosts.obj();
		auto_ptr<mongo::DBClientCursor> amDistance = mongoDao["AMCosts"].queryDocument(bsonObj_AMCosts);
		BSONObj amDistanceObj = amDistance->next();
		if(origin != destination) {
			isgParams.setDistance(amDistanceObj.getField("distance").Double());
		}
		else {
			isgParams.setDistance(0.0);
		}
		isgParams.setFirstHalfTour(true);

		double prevDepartureTime = 3.25; // first window; start of day
		double nextArrivalTime = primaryStop->getArrivalTime();
		if (*(tours.front()) != tour) { // if this tour is not the first tour of the day
			Tour* previousTour = *(std::find(tours.begin(), tours.end(), &tour)-1);
			prevDepartureTime = previousTour->getEndTime(); // departure time id taken as the end time of the previous tour
		}

		int stopCounter = 0;
		isgParams.setStopCounter(stopCounter);
		std::string& choice;
		int insertIdx = 0;
		Stop& nextStop = primaryStop;
		while(choice != "Quit" && stopCounter<3){
			LuaRef chooseISG = getGlobal(state.get(), "choose_isg");
			LuaRef retVal = chooseISG(personParams, isgParams);
			choice = retVal.cast<std::string>();
			if(choice != "Quit") {
				StopType stopType;
				if(choice == "Work") { stopType = WORK; }
				else if (choice == "Education") { stopType = EDUCATION; }
				else if (choice == "Shopping") {StopType = SHOP; }
				else if (choice == "Others") {stopType = OTHER; }
				Stop generatedStop = Stop(stopType, tour, false /*not primary*/, true /*in first half tour*/);
				tour.addStop(&generatedStop);
				predictStopModeDestination(generatedStop);
				calculateDepartureTime(generatedStop, nextStop);
				if(generatedStop.getDepartureTime() <= 3.25)
				{
					tour.removeStop(&generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				predictStopTimeOfDay(generatedStop);
				nextStop = generatedStop;
				personParams.blockTime(generatedStop.getArrivalTime(), generatedStop.getDepartureTime());
				nextArrivalTime = generatedStop.getArrivalTime();
				stopCounter = stopCounter + 1;
			}
		}

		// Second half tour
		BSONObjBuilder bsonObjBldr_PMCosts;
		bsonObjBldr_PMCosts << "origin" << destination << "destin" << origin; // origin is home and destination is primary activity location
		BSONObj bsonObj_PMCosts = bsonObjBldr_PMCosts.obj();
		auto_ptr<mongo::DBClientCursor> pmDistance = mongoDao["PMCosts"].queryDocument(bsonObj_PMCosts);
		BSONObj pmDistanceObj = pmDistance->next();
		if(origin != destination) {
			isgParams.setDistance(pmDistanceObj.getField("distance").Double());
		}
		else {
			isgParams.setDistance(0.0);
		}
		isgParams.setFirstHalfTour(false);

		double prevDepartureTime = primaryStop->getDepartureTime();
		double nextArrivalTime = 26.75; // end of day

		int stopCounter = 0;
		isgParams.setStopCounter(stopCounter);
		std::string& choice;
		int insertIdx = 0;
		Stop& prevStop = primaryStop;
		while(choice != "Quit" && stopCounter<3){
			LuaRef chooseISG = getGlobal(state.get(), "choose_isg");
			LuaRef retVal = chooseISG(personParams, isgParams);
			choice = retVal.cast<std::string>();
			if(choice != "Quit") {
				StopType stopType;
				if(choice == "Work") { stopType = WORK; }
				else if (choice == "Education") { stopType = EDUCATION; }
				else if (choice == "Shopping") {StopType = SHOP; }
				else if (choice == "Others") {stopType = OTHER; }
				Stop generatedStop = Stop(stopType, tour, false /*not primary*/, false /* not in first half tour*/);
				tour.addStop(&generatedStop);
				predictStopModeDestination(generatedStop);
				calculateArrivalTime(generatedStop, prevStop);
				if(generatedStop.getDepartureTime() >=  26.75)
				{
					tour.removeStop(&generatedStop);
					stopCounter = stopCounter + 1;
					continue;
				}
				predictStopTimeOfDay(generatedStop);
				prevStop = generatedStop;
				personParams.blockTime(generatedStop.getArrivalTime(), generatedStop.getDepartureTime());
				prevDepartureTime = generatedStop.getDepartureTime();
				stopCounter = stopCounter + 1;
			}
		}
	}
}

void PredaySystem::predictStopModeDestination(Stop& stop) {
}

void PredaySystem::predictStopTimeOfDay(Stop& stop) { // this must set the arrivaland departure time for the stop
}

void PredaySystem::calculateArrivalTime(Stop& currStop,  Stop& prevStop) { // this must set the arrival time for currStop
}

void PredaySystem::calculateDepartureTime(Stop& currStop,  Stop& nextStop) { // this must set the departure time for the currStop
}

void PredaySystem::calculateTourStartTime(Tour& tour) {
}


void PredaySystem::calculateTourEndTime(Tour& tour) {
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
		if(!personParams.isStudent() && personParams.getFixedWorkLocation() != 0) {
			//if person not a student and has a fixed work location
			usualWorkParams.setFirstOfMultiple((int)firstOfMultiple);
			usualWorkParams.setSubsequentOfMultiple((int) !firstOfMultiple);
			firstOfMultiple = false;
			attendsUsualWorkLocation = predictUsualWorkLocation(); // Predict if this tour is to a usual work location
		}
		Tour* workTour = new Tour(WORK);
		workTour->setUsualLocation(attendsUsualWorkLocation);
		tours.push_back(workTour);
	}

	// Construct education tours
	for(int i=0; i<numTours["EduT"]; i++) {
		Tour* eduTour = new Tour(EDUCATION);
		eduTour->setUsualLocation(true); // Education tours are always to usual locations
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
	predictDayPattern();

	//Predict number of Tours
	predictNumTours();

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
		Stop* primaryActivity = new Stop(tour.getTourType(), tour, true);
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
