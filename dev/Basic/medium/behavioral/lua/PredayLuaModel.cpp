//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayLuaModel.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Harish Loganathan
 */

#include "PredayLuaModel.hpp"


#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "logging/Log.hpp"
#include "mongo/client/dbclient.h"
#include "behavioral/PredayClasses.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace luabridge;
using namespace mongo;

sim_mob::medium::PredayLuaModel::PredayLuaModel()
{}

sim_mob::medium::PredayLuaModel::~PredayLuaModel()
{}

void sim_mob::medium::PredayLuaModel::mapClasses() {
	getGlobalNamespace(state.get())
			.beginClass <PersonParams> ("PersonParams")
			.addProperty("person_type_id", &PersonParams::getPersonTypeId)
			.addProperty("age_id", &PersonParams::getAgeId)
			.addProperty("universitystudent", &PersonParams::getIsUniversityStudent)
			.addProperty("female_dummy", &PersonParams::getIsFemale)
			.addProperty("income_id", &PersonParams::getIncomeId)
			.addProperty("work_at_home_dummy", &PersonParams::getWorksAtHome)
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
			.addFunction("getTimeWindowAvailabilityTour", &PersonParams::getTimeWindowAvailability)
			.endClass();
}

void sim_mob::medium::PredayLuaModel::predictDayPattern(PersonParams& personParams, boost::unordered_map<std::string, bool>& dayPattern) const {
	LuaRef chooseDP = getGlobal(state.get(), "choose_dp");
	LuaRef retVal = chooseDP(personParams);
	if (retVal.isTable()) {
		dayPattern["WorkT"] = retVal[1].cast<int>();
		dayPattern["EduT"] = retVal[2].cast<int>();
		dayPattern["ShopT"] = retVal[3].cast<int>();
		dayPattern["OthersT"] = retVal[4].cast<int>();
		dayPattern["WorkI"] = retVal[5].cast<int>();
		dayPattern["EduI"] = retVal[6].cast<int>();
		dayPattern["ShopI"] = retVal[7].cast<int>();
		dayPattern["OthersI"] = retVal[8].cast<int>();
	}
	else {
		throw std::runtime_error("Error in day pattern prediction. Unexpected return value");
	}
}

void sim_mob::medium::PredayLuaModel::predictNumTours(PersonParams& personParams, boost::unordered_map<std::string, bool>& dayPattern, boost::unordered_map<std::string, int>& numTours) const {
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

bool sim_mob::medium::PredayLuaModel::predictUsualWorkLocation(PersonParams& personParams, bool firstOfMultiple) const {
	LuaRef chooseUW = getGlobal(state.get(), "choose_uw"); // choose usual work location
    ModelParamsUsualWork usualWorkParams;
	usualWorkParams.setFirstOfMultiple((int) firstOfMultiple);
	usualWorkParams.setSubsequentOfMultiple((int) !firstOfMultiple);
	LuaRef retVal = chooseUW(personParams, usualWorkParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	return retVal.cast<bool>();
}




