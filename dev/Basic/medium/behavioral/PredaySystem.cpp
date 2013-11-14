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

#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"

using namespace sim_mob;
using namespace sim_mob::medium;
using namespace luabridge;

sim_mob::medium::PredaySystem::PredaySystem(PersonParams& personParams) : personParams(personParams) {
	modeReferenceIndex[1] = "Public bus";
	modeReferenceIndex[2] = "MRT";
	modeReferenceIndex[3] = "Private Bus";
	modeReferenceIndex[4] = "Auto";
	modeReferenceIndex[5] = "Share 2+";
	modeReferenceIndex[6] = "Share 3+";
	modeReferenceIndex[7] = "Motor";
	modeReferenceIndex[8] = "Walk";
}

void PredaySystem::mapClasses() {
	getGlobalNamespace(state.get())
			.beginClass <PersonParams> ("PersonParams")
			.addProperty("personTypeId", &PersonParams::getPersonTypeId)
			.addProperty("ageId", &PersonParams::getAgeId)
			.addProperty("isUniversityStudent", &PersonParams::getIsUniversityStudent)
			.addProperty("isFemale", &PersonParams::getIsFemale)
			.addProperty("incomeId", &PersonParams::getIncomeId)
			.addProperty("worksAtHome", &PersonParams::getWorksAtHome)
			.addProperty("carOwnNormal", &PersonParams::getCarOwnNormal)
			.addProperty("carOwnOffpeak", &PersonParams::getCarOwnOffpeak)
			.addProperty("motorOwn", &PersonParams::getMotorOwn)
			.addProperty("hasFlexibleWorkTiming", &PersonParams::getHasFlexibleWorkTiming)
			.addProperty("homeLocation", &PersonParams::getHomeLocation)
			.addProperty("fixedWorkLocation", &PersonParams::getFixedWorkLocation)
			.addProperty("fixedSchoolLocation", &PersonParams::getFixedSchoolLocation)
			.addFunction("getTimeWindowAvailability", &PersonParams::getTimeWindowAvailability)
			.endClass();
}

void sim_mob::medium::PredaySystem::planDay() {

}

void sim_mob::medium::PredaySystem::predictDayPattern() {
	LuaRef chooseDP = getGlobal(state.get(), "chooseDP");
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

void sim_mob::medium::PredaySystem::predictNumTours() {
	if(dayPattern.size() <= 0) {
		throw std::runtime_error("Cannot invoke number of tours model without a day pattern");
	}
	numTours["WorkT"] = numTours["EduT"] = numTours["ShopT"] = numTours["OthersT"] = 0;

	if(dayPattern["WorkT"]) {
		LuaRef chooseNTW = getGlobal(state.get(), "chooseNTW"); // choose Num. of Work Tours
		LuaRef retVal = chooseNTW(personParams);
	    if (retVal.isNumber()) {
	    	numTours["WorkT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["EduT"]) {
		LuaRef chooseNTE = getGlobal(state.get(), "chooseNTE");// choose Num. of Education Tours
		LuaRef retVal = chooseNTE(personParams);
	    if (retVal.isNumber()) {
	    	numTours["EduT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["ShopT"]) {
		LuaRef chooseNTS = getGlobal(state.get(), "chooseNTS");// choose Num. of Shopping Tours
		LuaRef retVal = chooseNTS(personParams);
	    if (retVal.isNumber()) {
	    	numTours["ShopT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["OthersT"]) {
		LuaRef chooseNTO = getGlobal(state.get(), "chooseNTO");// choose Num. of Other Tours
		LuaRef retVal = chooseNTO(personParams);
	    if (retVal.isNumber()) {
	    	numTours["OthersT"] = retVal.cast<int>();
	    }
	}
}

bool sim_mob::medium::PredaySystem::predictUsualWorkLocation() {
	LuaRef chooseUW = getGlobal(state.get(), "chooseUW"); // choose usual work location
	LuaRef retVal = chooseUW(personParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	return retVal.cast<bool>();
}

void sim_mob::medium::PredaySystem::predictTourMode(Tour& tour) {
	LuaRef chooseMode = getGlobal(state.get(), "chooseMode"); // choose usual work location
	LuaRef retVal = chooseMode(personParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	std::string& mode = modeReferenceIndex.at(retVal.cast<int>());
	tour.setTourMode(mode);
}

void sim_mob::medium::PredaySystem::predictTourModeDestination(Tour& tour) {

	LuaRef chooseTMD;
	switch (tour.getTourType()) {
	case WORK_TOUR:
		personParams.setStopType(1);
		chooseTMD = getGlobal(state.get(), "chooseTMDW"); // tour mode destination for work tour
		break;
	case EDUCATION_TOUR:
		personParams.setStopType(2);
		chooseTMD = getGlobal(state.get(), "chooseTMDE"); // tour mode destination for work tour
		break;
	case SHOP_TOUR:
		personParams.setStopType(3);
		chooseTMD = getGlobal(state.get(), "chooseTMDS"); // tour mode destination for work tour
		break;
	case OTHER_TOUR:
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

void sim_mob::medium::PredaySystem::predictTourTimeOfDay(Tour& tour) {
	if(!tour.isSubTour()) {
		long origin = tour.getPrimaryActivityLocation();
		long destination = personParams.getHomeLocation();
		if(origin != destination) {

		}
	}
}

void sim_mob::medium::PredaySystem::generateIntermediateStops() {
}

void sim_mob::medium::PredaySystem::predictStopTimeOfDay() {
}

void sim_mob::medium::PredaySystem::calculateArrivalTime() {
}

void sim_mob::medium::PredaySystem::calculateDepartureTime() {
}

void sim_mob::medium::PredaySystem::calculateTourStartTime() {
}

void sim_mob::medium::PredaySystem::predictStopModeDestination() {
}

void sim_mob::medium::PredaySystem::calculateTourEndTime() {
}


