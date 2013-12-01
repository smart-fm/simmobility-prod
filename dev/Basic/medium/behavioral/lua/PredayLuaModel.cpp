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
			.addProperty("fixed_place", &PersonParams::getFixedWorkPlace)
			.addProperty("fixedSchoolLocation", &PersonParams::getFixedSchoolLocation)
			.addProperty("only_adults", &PersonParams::getHH_OnlyAdults)
			.addProperty("only_workers", &PersonParams::getHH_OnlyWorkers)
			.addProperty("num_underfour", &PersonParams::getHH_NumUnder4)
			.addProperty("presence_of_under15", &PersonParams::getHH_HasUnder15)
			.addProperty("worklogsum", &PersonParams::getWorkLogSum)
			.addProperty("edulogsum", &PersonParams::getEduLogSum)
			.addProperty("shoplogsum", &PersonParams::getShopLogSum)
			.addProperty("otherlogsum", &PersonParams::getOtherLogSum)
			.addFunction("getTimeWindowAvailabilityTour", &PersonParams::getTimeWindowAvailability)
			.endClass();

	getGlobalNamespace(state.get())
			.beginClass <UsualWorkParams> ("UsualWorkParams")
			.addProperty("first_of_multiple", &UsualWorkParams::getFirstOfMultiple)
			.addProperty("subsequent_of_multiple", &UsualWorkParams::getSubsequentOfMultiple)
			.addProperty("distance1", &UsualWorkParams::getWalkDistanceAm)
			.addProperty("distance2", &UsualWorkParams::getWalkDistancePm)
			.addProperty("work_op", &UsualWorkParams::getZoneEmployment)
			.endClass();

	getGlobalNamespace(state.get())
			.beginClass<TourModeParams>("TourModeParams")
			.addProperty("average_transfer_number",&TourModeParams::getAvgTransfer)
			.addProperty("central_dummy",&TourModeParams::isCentralZone)
			.addProperty("cost_car_ERP_first",&TourModeParams::getCostCarErpFirst)
			.addProperty("cost_car_ERP_second",&TourModeParams::getCostCarErpSecond)
			.addProperty("cost_car_OP_first",&TourModeParams::getCostCarOpFirst)
			.addProperty("cost_car_OP_second",&TourModeParams::getCostCarOpSecond)
			.addProperty("cost_car_parking",&TourModeParams::getCostCarParking)
			.addProperty("cost_public_first",&TourModeParams::getCostPublicFirst)
			.addProperty("cost_public_second",&TourModeParams::getCostPublicSecond)
			.addProperty("drive1_AV",&TourModeParams::isDrive1Available)
			.addProperty("motor_AV",&TourModeParams::isMotorAvailable)
			.addProperty("mrt_AV",&TourModeParams::isMrtAvailable)
			.addProperty("privatebus_AV",&TourModeParams::isPrivateBusAvailable)
			.addProperty("publicbus_AV",&TourModeParams::isPublicBusAvailable)
			.addProperty("share2_AV",&TourModeParams::isShare2Available)
			.addProperty("share3_AV",&TourModeParams::isShare3Available)
			.addProperty("taxi_AV",&TourModeParams::isTaxiAvailable)
			.addProperty("tt_ivt_car_first",&TourModeParams::getTtCarIvtFirst)
			.addProperty("tt_ivt_car_second",&TourModeParams::getTtCarIvtSecond)
			.addProperty("tt_public_ivt_first",&TourModeParams::getTtPublicIvtFirst)
			.addProperty("tt_public_ivt_second",&TourModeParams::getTtPublicIvtSecond)
			.addProperty("tt_public_waiting_first",&TourModeParams::getTtPublicWaitingFirst)
			.addProperty("tt_public_waiting_second",&TourModeParams::getTtPublicWaitingSecond)
			.addProperty("tt_public_walk_first",&TourModeParams::getTtPublicWalkFirst)
			.addProperty("tt_public_walk_second",&TourModeParams::getTtPublicWalkSecond)
			.addProperty("tmw_walk_AV",&TourModeParams::isWalkAvailable)
			.addProperty("walk_distance1",&TourModeParams::getWalkDistance1)
			.addProperty("walk_distance2",&TourModeParams::getWalkDistance2)
			.addProperty("destination_area",&TourModeParams::getDestinationArea)
			.addProperty("origin_area",&TourModeParams::getOriginArea)
			.addProperty("resident_size",&TourModeParams::getResidentSize)
			.addProperty("work_op",&TourModeParams::getWorkOp)
			.endClass();

	getGlobalNamespace(state.get())
			.beginClass<TourModeDestinationParams>("TourModeDestinationParams")
			.addFunction("cost_public_first", &TourModeDestinationParams::getCostPublicFirst)
			.addFunction("cost_public_second", &TourModeDestinationParams::getCostPublicSecond)
			.addFunction("cost_car_ERP_first", &TourModeDestinationParams::getCostCarERPFirst)
			.addFunction("cost_car_ERP_second", &TourModeDestinationParams::getCostCarERPSecond)
			.addFunction("cost_car_OP_first", &TourModeDestinationParams::getCostCarOPFirst)
			.addFunction("cost_car_OP_second", &TourModeDestinationParams::getCostCarOPSecond)
			.addFunction("cost_car_parking", &TourModeDestinationParams::getCostCarParking)
			.addFunction("walk_distance1", &TourModeDestinationParams::getWalkDistance1)
			.addFunction("walk_distance2", &TourModeDestinationParams::getWalkDistance2)
			.addFunction("central_dummy", &TourModeDestinationParams::getCentralDummy)
			.addFunction("tt_public_ivt_first", &TourModeDestinationParams::getTT_PublicIvtFirst)
			.addFunction("tt_public_ivt_second", &TourModeDestinationParams::getTT_PublicIvtSecond)
			.addFunction("tt_public_out_first", &TourModeDestinationParams::getTT_PublicOutFirst)
			.addFunction("tt_public_out_second", &TourModeDestinationParams::getTT_PublicOutSecond)
			.addFunction("tt_car_ivt_first", &TourModeDestinationParams::getTT_CarIvtFirst)
			.addFunction("tt_car_ivt_second", &TourModeDestinationParams::getTT_CarIvtSecond)
			.addFunction("average_transfer_number", &TourModeDestinationParams::getAvgTransferNumber)
			.addFunction("employment", &TourModeDestinationParams::getEmployment)
			.addFunction("population", &TourModeDestinationParams::getArea)
			.addFunction("area", &TourModeDestinationParams::getPopulation)
			.addFunction("shop", &TourModeDestinationParams::getShop)
			.addFunction("availability",&TourModeDestinationParams::isAvailable)
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
		Print() << dayPattern["WorkT"] << dayPattern["EduT"] << dayPattern["ShopT"] << dayPattern["OthersT"] << std::endl;
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
	Print() << numTours["WorkT"] << numTours["EduT"] << numTours["ShopT"] << numTours["OthersT"] << std::endl;
}

bool sim_mob::medium::PredayLuaModel::predictUsualWorkLocation(PersonParams& personParams, UsualWorkParams& usualWorkParams) const {
	LuaRef chooseUW = getGlobal(state.get(), "choose_uw"); // choose usual work location
	LuaRef retVal = chooseUW(personParams, usualWorkParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	return retVal.cast<bool>();
}

int sim_mob::medium::PredayLuaModel::predictTourMode(PersonParams& personParams, TourModeParams& tourModeParams) const {
	switch (tourModeParams.getStopType()) {
	case WORK:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmw");
		LuaRef retVal = chooseTMD(personParams, tourModeParams);
		return retVal.cast<int>();
		break;
	}
	case EDUCATION:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tme");
		LuaRef retVal = chooseTMD(personParams, tourModeParams);
		return retVal.cast<int>();
		break;
	}
	default:
	{
		throw std::runtime_error("Tour mode model cannot be invoked for Shopping and Other tour types");
	}
	}
}

int sim_mob::medium::PredayLuaModel::predictTourModeDestination(PersonParams& personParams, TourModeDestinationParams& tourModeDestinationParams) const {
	switch (tourModeDestinationParams.getTourType()) {
	case WORK:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmdw");
		LuaRef retVal = chooseTMD(personParams, tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	case EDUCATION:
	{
		LuaRef chooseTM = getGlobal(state.get(), "choose_tmde");
		LuaRef retVal = chooseTM(personParams, tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	case SHOP:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmds");
		LuaRef retVal = chooseTMD(personParams, tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	case OTHER:
	{
		LuaRef chooseTM = getGlobal(state.get(), "choose_tmdo");
		LuaRef retVal = chooseTM(personParams, tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	default:
	{
		throw std::runtime_error("Tour mode model cannot be invoked for Shopping and Other tour types");
	}
	}
}




