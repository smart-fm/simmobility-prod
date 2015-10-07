//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

void sim_mob::PredayLogsumLuaModel::mapClasses()
{
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
				.addProperty("travel_probability", &PredayPersonParams::getTravelProbability)
				.addProperty("num_expected_trips", &PredayPersonParams::getTripsExpected)
			.endClass()

			.beginClass<LogsumTourModeParams>("TourModeParams")
				.addProperty("average_transfer_number",&LogsumTourModeParams::getAvgTransfer)
				.addProperty("central_dummy",&LogsumTourModeParams::isCentralZone)
				.addProperty("cost_car_ERP_first",&LogsumTourModeParams::getCostCarErpFirst)
				.addProperty("cost_car_ERP_second",&LogsumTourModeParams::getCostCarErpSecond)
				.addProperty("cost_car_OP_first",&LogsumTourModeParams::getCostCarOpFirst)
				.addProperty("cost_car_OP_second",&LogsumTourModeParams::getCostCarOpSecond)
				.addProperty("cost_car_parking",&LogsumTourModeParams::getCostCarParking)
				.addProperty("cost_public_first",&LogsumTourModeParams::getCostPublicFirst)
				.addProperty("cost_public_second",&LogsumTourModeParams::getCostPublicSecond)
				.addProperty("drive1_AV",&LogsumTourModeParams::isDrive1Available)
				.addProperty("motor_AV",&LogsumTourModeParams::isMotorAvailable)
				.addProperty("mrt_AV",&LogsumTourModeParams::isMrtAvailable)
				.addProperty("privatebus_AV",&LogsumTourModeParams::isPrivateBusAvailable)
				.addProperty("publicbus_AV",&LogsumTourModeParams::isPublicBusAvailable)
				.addProperty("share2_AV",&LogsumTourModeParams::isShare2Available)
				.addProperty("share3_AV",&LogsumTourModeParams::isShare3Available)
				.addProperty("taxi_AV",&LogsumTourModeParams::isTaxiAvailable)
				.addProperty("walk_AV",&LogsumTourModeParams::isWalkAvailable)
				.addProperty("tt_ivt_car_first",&LogsumTourModeParams::getTtCarIvtFirst)
				.addProperty("tt_ivt_car_second",&LogsumTourModeParams::getTtCarIvtSecond)
				.addProperty("tt_public_ivt_first",&LogsumTourModeParams::getTtPublicIvtFirst)
				.addProperty("tt_public_ivt_second",&LogsumTourModeParams::getTtPublicIvtSecond)
				.addProperty("tt_public_waiting_first",&LogsumTourModeParams::getTtPublicWaitingFirst)
				.addProperty("tt_public_waiting_second",&LogsumTourModeParams::getTtPublicWaitingSecond)
				.addProperty("tt_public_walk_first",&LogsumTourModeParams::getTtPublicWalkFirst)
				.addProperty("tt_public_walk_second",&LogsumTourModeParams::getTtPublicWalkSecond)
				.addProperty("walk_distance1",&LogsumTourModeParams::getWalkDistance1)
				.addProperty("walk_distance2",&LogsumTourModeParams::getWalkDistance2)
				.addProperty("destination_area",&LogsumTourModeParams::getDestinationArea)
				.addProperty("origin_area",&LogsumTourModeParams::getOriginArea)
				.addProperty("resident_size",&LogsumTourModeParams::getResidentSize)
				.addProperty("work_op",&LogsumTourModeParams::getWorkOp)
				.addProperty("education_op",&LogsumTourModeParams::getEducationOp)
				.addProperty("cbd_dummy",&LogsumTourModeParams::isCbdDestZone)
				.addProperty("cbd_dummy_origin",&LogsumTourModeParams::isCbdOrgZone)
				.addProperty("cost_increase", &LogsumTourModeParams::getCostIncrease)
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
				.addProperty("cbd_dummy_origin",&LogsumTourModeDestinationParams::isCbdOrgZone)
				.addFunction("cbd_dummy",&LogsumTourModeDestinationParams::getCbdDummy)
				.addProperty("cost_increase", &LogsumTourModeDestinationParams::getCostIncrease)
			.endClass();
}

void sim_mob::PredayLogsumLuaModel::computeDayPatternLogsums(PredayPersonParams& personParams) const
{
	LuaRef computeLogsumDPT = getGlobal(state.get(), "compute_logsum_dpt");
	LuaRef dptRetVal = computeLogsumDPT(personParams);
	if(dptRetVal.isTable())
	{
		personParams.setDptLogsum(dptRetVal[1].cast<double>());
		personParams.setTripsExpected(dptRetVal[2].cast<double>());
	}
	else
	{
		throw std::runtime_error("compute_logsum_dpt function does not return a table as expected");
	}

	LuaRef computeLogsumDPS = getGlobal(state.get(), "compute_logsum_dps");
	LuaRef dpsLogsum = computeLogsumDPS(personParams);
	personParams.setDpsLogsum(dpsLogsum.cast<double>());
}

void sim_mob::PredayLogsumLuaModel::computeDayPatternBinaryLogsums(PredayPersonParams& personParams) const
{
	LuaRef computeLogsumDPB = getGlobal(state.get(), "compute_logsum_dpb");
	LuaRef dpbRetVal = computeLogsumDPB(personParams);
	if(dpbRetVal.isTable())
	{
		personParams.setDpbLogsum(dpbRetVal[1].cast<double>());
		personParams.setTravelProbability(dpbRetVal[2].cast<double>());
	}
	else
	{
		throw std::runtime_error("compute_logsum_dpb function does not return a table as expected");
	}
}

void sim_mob::PredayLogsumLuaModel::computeTourModeLogsum(PredayPersonParams& personParams, LogsumTourModeParams& tourModeParams) const
{
	if(personParams.hasFixedWorkPlace())
	{
		LuaRef computeLogsumTMW = getGlobal(state.get(), "compute_logsum_tmw");
		LuaRef workLogSum = computeLogsumTMW(&personParams, &tourModeParams);
		personParams.setWorkLogSum(workLogSum.cast<double>());
	}
}

void sim_mob::PredayLogsumLuaModel::computeTourModeDestinationLogsum(PredayPersonParams& personParams, LogsumTourModeDestinationParams& tourModeDestinationParams) const
{
	if(!personParams.hasFixedWorkPlace())
	{
		LuaRef computeLogsumTMDW = getGlobal(state.get(), "compute_logsum_tmdw");
		LuaRef workLogSum = computeLogsumTMDW(&personParams, &tourModeDestinationParams);
		personParams.setWorkLogSum(workLogSum.cast<double>());
	}

	LuaRef computeLogsumTMDS = getGlobal(state.get(), "compute_logsum_tmds");
	LuaRef shopLogSum = computeLogsumTMDS(&personParams, &tourModeDestinationParams);
	personParams.setShopLogSum(shopLogSum.cast<double>());

	LuaRef computeLogsumTMDO = getGlobal(state.get(), "compute_logsum_tmdo");
	LuaRef otherLogSum = computeLogsumTMDO(&personParams, &tourModeDestinationParams);
	personParams.setOtherLogSum(otherLogSum.cast<double>());
}
