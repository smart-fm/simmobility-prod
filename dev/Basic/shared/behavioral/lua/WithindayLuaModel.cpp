//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WithindayLuaModel.hpp"

#include <memory>
#include "conf/ConfigManager.hpp"
#include "conf/RawConfigParams.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"

using namespace std;
using namespace sim_mob;
using namespace luabridge;

namespace
{

struct ModelContext
{
	WithindayLuaModel wdModel;
};

thread_local std::unique_ptr<ModelContext> threadContext;

void ensureContext()
{
	if (!threadContext)
	{
		try
		{
			const ModelScriptsMap& extScripts = ConfigManager::GetInstance().FullConfig().luaScriptsMap;
			const std::string& scriptsPath = extScripts.getPath();
			ModelContext* modelCtx = new ModelContext();
			modelCtx->wdModel.loadFile(scriptsPath + extScripts.getScriptFileName("logit"));
			modelCtx->wdModel.loadFile(scriptsPath + extScripts.getScriptFileName("wdmw"));
			modelCtx->wdModel.loadFile(scriptsPath + extScripts.getScriptFileName("wdme"));
			modelCtx->wdModel.loadFile(scriptsPath + extScripts.getScriptFileName("wdmso"));
			modelCtx->wdModel.initialize();
			threadContext.reset(modelCtx);
		}
		catch (const std::out_of_range& oorx)
		{
			throw std::runtime_error("missing or invalid generic property 'external_scripts'");
		}
	}
}
} //end anonymous namespace

sim_mob::WithindayLuaModel::WithindayLuaModel()
{}

sim_mob::WithindayLuaModel::~WithindayLuaModel()
{}

void sim_mob::WithindayLuaModel::mapClasses()
{
	getGlobalNamespace(state.get())
			.beginClass <PersonParams> ("WithindayPersonParams")
				.addProperty("person_id", &PersonParams::getPersonId)
				.addProperty("person_type_id", &PersonParams::getPersonTypeId)
				.addProperty("age_id", &PersonParams::getAgeId)
				.addProperty("universitystudent", &PersonParams::getIsUniversityStudent)
				.addProperty("female_dummy", &PersonParams::getIsFemale)
				.addProperty("student_dummy", &PersonParams::isStudent) //not used in lua
				.addProperty("worker_dummy", &PersonParams::isWorker) //not used in lua
				.addProperty("income_id", &PersonParams::getIncomeId)
				.addProperty("missing_income", &PersonParams::getMissingIncome)
				.addProperty("work_at_home_dummy", &PersonParams::getWorksAtHome)
				.addProperty("car_own", &PersonParams::getCarOwn)
				.addProperty("car_own_normal", &PersonParams::getCarOwnNormal)
				.addProperty("car_own_offpeak", &PersonParams::getCarOwnOffpeak)
				.addProperty("motor_own", &PersonParams::getMotorOwn)
				.addProperty("fixed_work_hour", &PersonParams::getHasFixedWorkTiming) //not used in lua
				.addProperty("homeLocation", &PersonParams::getHomeLocation) //not used in lua
				.addProperty("fixed_place", &PersonParams::hasFixedWorkPlace)
				.addProperty("fixedSchoolLocation", &PersonParams::getFixedSchoolLocation) //not used in lua
				.addProperty("only_adults", &PersonParams::getHH_OnlyAdults)
				.addProperty("only_workers", &PersonParams::getHH_OnlyWorkers)
				.addProperty("num_underfour", &PersonParams::getHH_NumUnder4)
				.addProperty("presence_of_under15", &PersonParams::getHH_HasUnder15)
				.addProperty("worklogsum", &PersonParams::getWorkLogSum)
				.addProperty("edulogsum", &PersonParams::getEduLogSum)
				.addProperty("shoplogsum", &PersonParams::getShopLogSum)
				.addProperty("otherlogsum", &PersonParams::getOtherLogSum)
				.addProperty("dptour_logsum", &PersonParams::getDptLogsum)
				.addProperty("dpstop_logsum", &PersonParams::getDpsLogsum)
				.addProperty("travel_probability", &PersonParams::getTravelProbability)
				.addProperty("num_expected_trips", &PersonParams::getTripsExpected)
			.endClass()

			.beginClass<WithindayModeParams>("WithindayModeParams")
				.addProperty("average_transfer_number",&WithindayModeParams::getAvgTransfer)
				.addProperty("central_dummy",&WithindayModeParams::isCentralZone)
				.addProperty("parking_rate",&WithindayModeParams::getCostCarParking)
				.addProperty("tt_ivt_car",&WithindayModeParams::getTtCarInVehicle)
				.addProperty("tt_public_ivt",&WithindayModeParams::getTtPublicInVehicle)
				.addProperty("tt_public_waiting",&WithindayModeParams::getTtPublicWaiting)
				.addProperty("tt_public_walk",&WithindayModeParams::getTtPublicWalk)
				.addProperty("destination_area",&WithindayModeParams::getDestinationArea)
				.addProperty("origin_area",&WithindayModeParams::getOriginArea)
				.addProperty("resident_size",&WithindayModeParams::getOriginResidentSize)
				.addProperty("work_op",&WithindayModeParams::getDestinationWorkerSize)
				.addProperty("education_op",&WithindayModeParams::getDestinationStudentsSize)
				.addProperty("shop", &WithindayModeParams::getDestinationShops)
				.addProperty("distance_remaining", &WithindayModeParams::getDistance)
				.addProperty("drive1_AV",&WithindayModeParams::isDrive1Available)
				.addProperty("motor_AV",&WithindayModeParams::isMotorAvailable)
				.addProperty("mrt_AV",&WithindayModeParams::isMrtAvailable)
				.addProperty("privatebus_AV",&WithindayModeParams::isPrivateBusAvailable)
				.addProperty("publicbus_AV",&WithindayModeParams::isPublicBusAvailable)
				.addProperty("share2_AV",&WithindayModeParams::isShare2Available)
				.addProperty("share3_AV",&WithindayModeParams::isShare3Available)
				.addProperty("taxi_AV",&WithindayModeParams::isTaxiAvailable)
				.addProperty("walk_AV",&WithindayModeParams::isWalkAvailable)
			.endClass();
}

int sim_mob::WithindayLuaModel::chooseMode(PersonParams& personParams, WithindayModeParams& wdModeParams) const
{
	switch(wdModeParams.getTripType())
	{
	case WORK:
	{
		LuaRef chooseWDMW = getGlobal(state.get(), "choose_wdmw");
		LuaRef retVal = chooseWDMW(&personParams, &wdModeParams);
		return retVal.cast<int>();
		break;
	}
	case EDUCATION:
	{
		LuaRef chooseWDME = getGlobal(state.get(), "choose_wdme");
		LuaRef retVal = chooseWDME(&personParams, &wdModeParams);
		return retVal.cast<int>();
		break;
	}
	case SHOP:
	case OTHER:
	case NULL_STOP:
	{
		LuaRef chooseWDMSO = getGlobal(state.get(), "choose_wdmso");
		LuaRef retVal = chooseWDMSO(&personParams, &wdModeParams);
		return retVal.cast<int>();
		break;
	}
	}
}

const WithindayLuaModel& sim_mob::WithindayLuaProvider::getWithindayModel()
{
	ensureContext();
	return threadContext.get()->wdModel;
}
