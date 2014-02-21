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
				.addProperty("missing_income", &PersonParams::getMissingIncome)
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
			.endClass()

			.beginClass <UsualWorkParams> ("UsualWorkParams")
				.addProperty("first_of_multiple", &UsualWorkParams::getFirstOfMultiple)
				.addProperty("subsequent_of_multiple", &UsualWorkParams::getSubsequentOfMultiple)
				.addProperty("distance1", &UsualWorkParams::getWalkDistanceAm)
				.addProperty("distance2", &UsualWorkParams::getWalkDistancePm)
				.addProperty("work_op", &UsualWorkParams::getZoneEmployment)
			.endClass()

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
				.addProperty("walk_AV",&TourModeParams::isWalkAvailable)
				.addProperty("tt_ivt_car_first",&TourModeParams::getTtCarIvtFirst)
				.addProperty("tt_ivt_car_second",&TourModeParams::getTtCarIvtSecond)
				.addProperty("tt_public_ivt_first",&TourModeParams::getTtPublicIvtFirst)
				.addProperty("tt_public_ivt_second",&TourModeParams::getTtPublicIvtSecond)
				.addProperty("tt_public_waiting_first",&TourModeParams::getTtPublicWaitingFirst)
				.addProperty("tt_public_waiting_second",&TourModeParams::getTtPublicWaitingSecond)
				.addProperty("tt_public_walk_first",&TourModeParams::getTtPublicWalkFirst)
				.addProperty("tt_public_walk_second",&TourModeParams::getTtPublicWalkSecond)
				.addProperty("walk_distance1",&TourModeParams::getWalkDistance1)
				.addProperty("walk_distance2",&TourModeParams::getWalkDistance2)
				.addProperty("destination_area",&TourModeParams::getDestinationArea)
				.addProperty("origin_area",&TourModeParams::getOriginArea)
				.addProperty("resident_size",&TourModeParams::getResidentSize)
				.addProperty("work_op",&TourModeParams::getWorkOp)
				.addProperty("education_op",&TourModeParams::getEducationOp)
			.endClass()

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
				.addFunction("population", &TourModeDestinationParams::getPopulation)
				.addFunction("area", &TourModeDestinationParams::getArea)
				.addFunction("shop", &TourModeDestinationParams::getShop)
				.addFunction("availability",&TourModeDestinationParams::isAvailable_TMD)
			.endClass()

			.beginClass<StopModeDestinationParams>("StopModeDestinationParams")
				.addFunction("stop_type", &StopModeDestinationParams::getTourPurpose)
				.addFunction("cost_public", &StopModeDestinationParams::getCostPublic)
				.addFunction("cost_car_ERP", &StopModeDestinationParams::getCarCostERP)
				.addFunction("cost_car_OP", &StopModeDestinationParams::getCostCarOP)
				.addFunction("cost_car_parking", &StopModeDestinationParams::getCostCarParking)
				.addFunction("walk_distance1", &StopModeDestinationParams::getWalkDistanceFirst)
				.addFunction("walk_distance2", &StopModeDestinationParams::getWalkDistanceSecond)
				.addFunction("central_dummy", &StopModeDestinationParams::getCentralDummy)
				.addFunction("tt_public_ivt", &StopModeDestinationParams::getTT_PubIvt)
				.addFunction("tt_public_out", &StopModeDestinationParams::getTT_PubOut)
				.addFunction("tt_car_ivt", &StopModeDestinationParams::getTT_CarIvt)
				.addFunction("employment", &StopModeDestinationParams::getEmployment)
				.addFunction("population", &StopModeDestinationParams::getPopulation)
				.addFunction("area", &StopModeDestinationParams::getArea)
				.addFunction("shop", &StopModeDestinationParams::getShop)
				.addFunction("availability",&StopModeDestinationParams::isAvailable_IMD)
			.endClass()

			.beginClass<TourTimeOfDayParams>("TourTimeOfDayParams")
				.addFunction("TT_HT1", &TourTimeOfDayParams::getTT_FirstHalfTour)
				.addFunction("TT_HT2", &TourTimeOfDayParams::getTT_SecondHalfTour)
				.addProperty("cost_HT1_am", &TourTimeOfDayParams::getCostHt1Am)
				.addProperty("cost_HT1_pm", &TourTimeOfDayParams::getCostHt1Pm)
				.addProperty("cost_HT1_op", &TourTimeOfDayParams::getCostHt1Op)
				.addProperty("cost_HT2_am", &TourTimeOfDayParams::getCostHt2Am)
				.addProperty("cost_HT2_pm", &TourTimeOfDayParams::getCostHt2Pm)
				.addProperty("cost_HT2_op", &TourTimeOfDayParams::getCostHt2Op)
			.endClass()

			.beginClass<StopTimeOfDayParams>("StopTimeOfDayParams")
				.addFunction("TT", &StopTimeOfDayParams::getTravelTime)
				.addFunction("cost", &StopTimeOfDayParams::getTravelCost)
				.addProperty("high_tod", &StopTimeOfDayParams::getTodHigh)
				.addProperty("low_tod", &StopTimeOfDayParams::getTodLow)
				.addProperty("first_bound", &StopTimeOfDayParams::getFirstBound)
				.addProperty("second_bound", &StopTimeOfDayParams::getSecondBound)
				.addFunction("availability", &StopTimeOfDayParams::getAvailability)
			.endClass()

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
				.addProperty("has_subtour", &StopGenerationParams::getHasSubtour)
				.addFunction("availability", &StopGenerationParams::isAvailable)
			.endClass();

}

void sim_mob::medium::PredayLuaModel::predictDayPattern(PersonParams& personParams, boost::unordered_map<std::string, bool>& dayPattern) const {
	LuaRef chooseDP = getGlobal(state.get(), "choose_dp");
	LuaRef retVal = chooseDP(&personParams);
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
		LuaRef retVal = chooseNTW(&personParams);
	    if (retVal.isNumber()) {
	    	numTours["WorkT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["EduT"]) {
		LuaRef chooseNTE = getGlobal(state.get(), "choose_nte"); // choose Num. of Education Tours
		LuaRef retVal = chooseNTE(&personParams);
	    if (retVal.isNumber()) {
	    	numTours["EduT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["ShopT"]) {
		LuaRef chooseNTS = getGlobal(state.get(), "choose_nts"); // choose Num. of Shopping Tours
		LuaRef retVal = chooseNTS(&personParams);
	    if (retVal.isNumber()) {
	    	numTours["ShopT"] = retVal.cast<int>();
	    }
	}
	if(dayPattern["OthersT"]) {
		LuaRef chooseNTO = getGlobal(state.get(), "choose_nto"); // choose Num. of Other Tours
		LuaRef retVal = chooseNTO(&personParams);
	    if (retVal.isNumber()) {
	    	numTours["OthersT"] = retVal.cast<int>();
	    }
	}
}

bool sim_mob::medium::PredayLuaModel::predictUsualWorkLocation(PersonParams& personParams, UsualWorkParams& usualWorkParams) const {
	LuaRef chooseUW = getGlobal(state.get(), "choose_uw"); // choose usual work location
	LuaRef retVal = chooseUW(&personParams, &usualWorkParams);
	if (!retVal.isNumber()) {
		throw std::runtime_error("Error in usual work location model. Unexpected return value");
	}
	return retVal.cast<bool>();
}

int sim_mob::medium::PredayLuaModel::predictTourMode(PersonParams& personParams, TourModeParams& tourModeParams) const {
	switch (tourModeParams.getStopType()) {
	case WORK:
	{
		LuaRef chooseTMW = getGlobal(state.get(), "choose_tmw");
		LuaRef retVal = chooseTMW(&personParams, &tourModeParams);
		return retVal.cast<int>();
		break;
	}
	case EDUCATION:
	{
		LuaRef chooseTME = getGlobal(state.get(), "choose_tme");
		LuaRef retVal = chooseTME(&personParams, &tourModeParams);
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
	switch (tourModeDestinationParams.getTourPurpose()) {
	case WORK:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmdw");
		LuaRef retVal = chooseTMD(&personParams, &tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	case SHOP:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmds");
		LuaRef retVal = chooseTMD(&personParams, &tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	case OTHER:
	{
		LuaRef chooseTMD = getGlobal(state.get(), "choose_tmdo");
		LuaRef retVal = chooseTMD(&personParams, &tourModeDestinationParams);
		return retVal.cast<int>();
		break;
	}
	default:
	{
		throw std::runtime_error("Tour mode model cannot be invoked for Shopping and Other tour types");
	}
	}
}

int sim_mob::medium::PredayLuaModel::predictTourTimeOfDay(PersonParams& personParams, TourTimeOfDayParams& tourTimeOfDayParams, StopType tourType) const {
	switch (tourType) {
	case WORK:
	{
		LuaRef chooseTTDW = getGlobal(state.get(), "choose_ttdw");
		LuaRef retVal = chooseTTDW(&personParams, &tourTimeOfDayParams);
		return retVal.cast<int>();
		break;
	}
	case EDUCATION:
	{
		LuaRef chooseTTDE = getGlobal(state.get(), "choose_ttde");
		LuaRef retVal = chooseTTDE(&personParams, &tourTimeOfDayParams);
		return retVal.cast<int>();
		break;
	}
	case SHOP: // Fall through
	case OTHER:
	{
		LuaRef chooseTTDO = getGlobal(state.get(), "choose_ttdo");
		LuaRef retVal = chooseTTDO(&personParams, &tourTimeOfDayParams);
		return retVal.cast<int>();
		break;
	}
	}
}

int sim_mob::medium::PredayLuaModel::generateIntermediateStop(PersonParams& personParams, StopGenerationParams& isgParams) const {
	LuaRef chooseISG = getGlobal(state.get(), "choose_isg");
	LuaRef retVal = chooseISG(&personParams, &isgParams);
	return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictStopModeDestination(PersonParams& personParams, StopModeDestinationParams& imdParams) const {
	LuaRef chooseIMD = getGlobal(state.get(), "choose_imd");
	LuaRef retVal = chooseIMD(&personParams, &imdParams);
	return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictStopTimeOfDay(PersonParams& personParams, StopTimeOfDayParams& stopTimeOfDayParams) const {
	LuaRef chooseITD = getGlobal(state.get(), "choose_itd");
	LuaRef retVal = chooseITD(&personParams, &stopTimeOfDayParams);
	return retVal.cast<int>();
}
