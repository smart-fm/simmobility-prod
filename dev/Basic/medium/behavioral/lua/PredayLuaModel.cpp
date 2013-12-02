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
				.addProperty("student_dummy", &PersonParams::isStudent)
				.addProperty("worker_dummy", &PersonParams::isWorker)
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
				.addFunction("cost_public_first", &TourModeDestinationParams::getCostPublicFirst_TMD)
				.addFunction("cost_public_second", &TourModeDestinationParams::getCostPublicSecond_TMD)
				.addFunction("cost_car_ERP_first", &TourModeDestinationParams::getCostCarERPFirst_TMD)
				.addFunction("cost_car_ERP_second", &TourModeDestinationParams::getCostCarERPSecond_TMD)
				.addFunction("cost_car_OP_first", &TourModeDestinationParams::getCostCarOPFirst_TMD)
				.addFunction("cost_car_OP_second", &TourModeDestinationParams::getCostCarOPSecond_TMD)
				.addFunction("cost_car_parking", &TourModeDestinationParams::getCostCarParking_TMD)
				.addFunction("walk_distance1", &TourModeDestinationParams::getWalkDistance1_TMD)
				.addFunction("walk_distance2", &TourModeDestinationParams::getWalkDistance2_TMD)
				.addFunction("central_dummy", &TourModeDestinationParams::getCentralDummy_TMD)
				.addFunction("tt_public_ivt_first", &TourModeDestinationParams::getTT_PublicIvtFirst_TMD)
				.addFunction("tt_public_ivt_second", &TourModeDestinationParams::getTT_PublicIvtSecond_TMD)
				.addFunction("tt_public_out_first", &TourModeDestinationParams::getTT_PublicOutFirst_TMD)
				.addFunction("tt_public_out_second", &TourModeDestinationParams::getTT_PublicOutSecond_TMD)
				.addFunction("tt_car_ivt_first", &TourModeDestinationParams::getTT_CarIvtFirst_TMD)
				.addFunction("tt_car_ivt_second", &TourModeDestinationParams::getTT_CarIvtSecond_TMD)
				.addFunction("average_transfer_number", &TourModeDestinationParams::getAvgTransferNumber_TMD)
				.addFunction("employment", &TourModeDestinationParams::getEmployment_TMD)
				.addFunction("population", &TourModeDestinationParams::getArea_TMD)
				.addFunction("area", &TourModeDestinationParams::getPopulation_TMD)
				.addFunction("shop", &TourModeDestinationParams::getShop_TMD)
				.addFunction("availability",&TourModeDestinationParams::isAvailable_TMD)
			.endClass();

	getGlobalNamespace(state.get())
			.beginClass<StopGenerationParams>("StopGenerationParams")
				.addProperty("tour_type", &StopGenerationParams::getTourType)
				.addProperty("driver_dummy", &StopGenerationParams::isDriver)
				.addProperty("passenger_dummy", &StopGenerationParams::isPassenger)
				.addProperty("public_dummy", &StopGenerationParams::isPublicTransitCommuter)
				.addProperty("first_tour_dummy", &StopGenerationParams::isFirstTour)
				.addProperty("tour_remain", &StopGenerationParams::getNumRemainingTours)
				.addProperty("distance", &StopGenerationParams::getDistance)
				.addProperty("p_700a_930a", &StopGenerationParams::getP_700a_930a)
				.addProperty("p_930a_1200a", &StopGenerationParams::getP_930a_1200a)
				.addProperty("p_300p_530p", &StopGenerationParams::getP_300p_530p)
				.addProperty("p_530p_730p", &StopGenerationParams::getP_530p_730p)
				.addProperty("p_730p_1000p", &StopGenerationParams::getP_730p_1000p)
				.addProperty("p_1000p_700a", &StopGenerationParams::getP_1000p_700a)
				.addProperty("first_bound", &StopGenerationParams::getFirstBound)
				.addProperty("second_bound", &StopGenerationParams::getSecondBound)
				.addProperty("first_stop", &StopGenerationParams::getFirstStop)
				.addProperty("second_stop", &StopGenerationParams::getSecondStop)
				.addProperty("three_plus_stop", &StopGenerationParams::getThreePlusStop)
				.addFunction("availability", &StopGenerationParams::isAvailable)
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
	switch (tourModeDestinationParams.getTourType_TMD()) {
	case WORK:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmdw");
		LuaRef retVal = chooseTMD(personParams, tourModeDestinationParams);
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

int sim_mob::medium::PredayLuaModel::generateIntermediateStop(PersonParams& personParams, StopGenerationParams& isgParams) const {
	LuaRef chooseISG = getGlobal(state.get(), "choose_isg");
	LuaRef retVal = chooseISG(personParams, isgParams);
	return retVal.cast<int>();
}




