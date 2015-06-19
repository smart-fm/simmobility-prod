//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayLuaModel.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Harish Loganathan
 */

#include "PredayLogsumLuaModel.hpp"

#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;
using namespace luabridge;

sim_mob::PredayLogsumLuaModel::PredayLogsumLuaModel()
{}

sim_mob::PredayLogsumLuaModel::~PredayLogsumLuaModel()
{}

void sim_mob::PredayLogsumLuaModel::mapClasses() {
	getGlobalNamespace(state.get())
			.beginClass <PredayPersonParams> ("PredayPersonParams")
				.addProperty("person_id", &PredayPersonParams::getPersonId)
				.addProperty("person_type_id", &PredayPersonParams::getPersonTypeId)
				.addProperty("age_id", &PredayPersonParams::getAgeId)
				.addProperty("universitystudent", &PredayPersonParams::getIsUniversityStudent)
				.addProperty("female_dummy", &PredayPersonParams::getIsFemale)
				.addProperty("student_dummy", &PredayPersonParams::isStudent) //not used in lua
				.addProperty("worker_dummy", &PredayPersonParams::isWorker) //not used in lua
				.addProperty("income_id", &PredayPersonParams::getIncomeId)
				.addProperty("missing_income", &PredayPersonParams::getMissingIncome)
				.addProperty("work_at_home_dummy", &PredayPersonParams::getWorksAtHome)
				.addProperty("car_own", &PredayPersonParams::getCarOwn)
				.addProperty("car_own_normal", &PredayPersonParams::getCarOwnNormal)
				.addProperty("car_own_offpeak", &PredayPersonParams::getCarOwnOffpeak)
				.addProperty("motor_own", &PredayPersonParams::getMotorOwn)
				.addProperty("fixed_work_hour", &PredayPersonParams::getHasFixedWorkTiming) //not used in lua
				.addProperty("homeLocation", &PredayPersonParams::getHomeLocation) //not used in lua
				.addProperty("fixed_place", &PredayPersonParams::hasFixedWorkPlace)
				.addProperty("fixedSchoolLocation", &PredayPersonParams::getFixedSchoolLocation) //not used in lua
				.addProperty("only_adults", &PredayPersonParams::getHH_OnlyAdults)
				.addProperty("only_workers", &PredayPersonParams::getHH_OnlyWorkers)
				.addProperty("num_underfour", &PredayPersonParams::getHH_NumUnder4)
				.addProperty("presence_of_under15", &PredayPersonParams::getHH_HasUnder15)
				.addProperty("worklogsum", &PredayPersonParams::getWorkLogSum)
				.addProperty("edulogsum", &PredayPersonParams::getEduLogSum)
				.addProperty("shoplogsum", &PredayPersonParams::getShopLogSum)
				.addProperty("otherlogsum", &PredayPersonParams::getOtherLogSum)
				.addProperty("dptour_logsum", &PredayPersonParams::getDptLogsum)
				.addProperty("dpstop_logsum", &PredayPersonParams::getDpsLogsum)
			.endClass()

			.beginClass<LogsumTourModeDestinationParams>("LogsumTourModeDestinationParams")
				.addFunction("cost_public_first", &LogsumTourModeDestinationParams::getCostPublicFirst)
				.addFunction("cost_public_second", &LogsumTourModeDestinationParams::getCostPublicSecond)
				.addFunction("cost_car_ERP_first", &LogsumTourModeDestinationParams::getCostCarERPFirst)
				.addFunction("cost_car_ERP_second", &LogsumTourModeDestinationParams::getCostCarERPSecond)
				.addFunction("cost_car_OP_first", &LogsumTourModeDestinationParams::getCostCarOPFirst)
				.addFunction("cost_car_OP_second", &LogsumTourModeDestinationParams::getCostCarOPSecond)
				.addFunction("cost_car_parking", &LogsumTourModeDestinationParams::getCostCarParking)
				.addFunction("walk_distance1", &LogsumTourModeDestinationParams::getWalkDistance1)
				.addFunction("walk_distance2", &LogsumTourModeDestinationParams::getWalkDistance2)
				.addFunction("central_dummy", &LogsumTourModeDestinationParams::getCentralDummy)
				.addFunction("tt_public_ivt_first", &LogsumTourModeDestinationParams::getTT_PublicIvtFirst)
				.addFunction("tt_public_ivt_second", &LogsumTourModeDestinationParams::getTT_PublicIvtSecond)
				.addFunction("tt_public_out_first", &LogsumTourModeDestinationParams::getTT_PublicOutFirst)
				.addFunction("tt_public_out_second", &LogsumTourModeDestinationParams::getTT_PublicOutSecond)
				.addFunction("tt_car_ivt_first", &LogsumTourModeDestinationParams::getTT_CarIvtFirst)
				.addFunction("tt_car_ivt_second", &LogsumTourModeDestinationParams::getTT_CarIvtSecond)
				.addFunction("average_transfer_number", &LogsumTourModeDestinationParams::getAvgTransferNumber)
				.addFunction("employment", &LogsumTourModeDestinationParams::getEmployment)
				.addFunction("population", &LogsumTourModeDestinationParams::getPopulation)
				.addFunction("area", &LogsumTourModeDestinationParams::getArea)
				.addFunction("shop", &LogsumTourModeDestinationParams::getShop)
				.addFunction("availability",&LogsumTourModeDestinationParams::isAvailable_TMD)
			.endClass();
}

void sim_mob::PredayLogsumLuaModel::computeDayPatternLogsums(PredayPersonParams& personParams) const
{
	LuaRef computeLogsumDPT = getGlobal(state.get(), "compute_logsum_dpt");
	LuaRef dptLogsum = computeLogsumDPT(personParams);
	personParams.setDptLogsum(dptLogsum.cast<double>());

	LuaRef computeLogsumDPS = getGlobal(state.get(), "compute_logsum_dps");
	LuaRef dpsLogsum = computeLogsumDPS(personParams);
	personParams.setDpsLogsum(dpsLogsum.cast<double>());
}

void sim_mob::PredayLogsumLuaModel::computeDayPatternBinaryLogsums(PredayPersonParams& personParams) const
{
	LuaRef computeLogsumDPB = getGlobal(state.get(), "compute_logsum_dpb");
	LuaRef dpbLogsum = computeLogsumDPB(personParams);
	personParams.setDpbLogsum(dpbLogsum.cast<double>());
}

void sim_mob::PredayLogsumLuaModel::computeTourModeDestinationLogsum(PredayPersonParams& personParams, LogsumTourModeDestinationParams& tourModeDestinationParams) const
{
	LuaRef computeLogsumTMDW = getGlobal(state.get(), "compute_logsum_tmdw");
	LuaRef workLogSum = computeLogsumTMDW(&personParams, &tourModeDestinationParams);
	personParams.setWorkLogSum(workLogSum.cast<double>());

	LuaRef computeLogsumTMDS = getGlobal(state.get(), "compute_logsum_tmds");
	LuaRef shopLogSum = computeLogsumTMDS(&personParams, &tourModeDestinationParams);
	personParams.setShopLogSum(shopLogSum.cast<double>());

	LuaRef computeLogsumTMDO = getGlobal(state.get(), "compute_logsum_tmdo");
	LuaRef otherLogSum = computeLogsumTMDO(&personParams, &tourModeDestinationParams);
	personParams.setOtherLogSum(otherLogSum.cast<double>());
}
