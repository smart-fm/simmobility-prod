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
			modelCtx->wdModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmw"));
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
			.endClass();

//			.beginClass<LogsumTourModeParams>("TourModeParams")
//				.addProperty("average_transfer_number",&LogsumTourModeParams::getAvgTransfer)
//				.addProperty("central_dummy",&LogsumTourModeParams::isCentralZone)
//				.addProperty("cost_car_ERP_first",&LogsumTourModeParams::getCostCarErpFirst)
//				.addProperty("cost_car_ERP_second",&LogsumTourModeParams::getCostCarErpSecond)
//				.addProperty("cost_car_OP_first",&LogsumTourModeParams::getCostCarOpFirst)
//				.addProperty("cost_car_OP_second",&LogsumTourModeParams::getCostCarOpSecond)
//				.addProperty("cost_car_parking",&LogsumTourModeParams::getCostCarParking)
//				.addProperty("cost_public_first",&LogsumTourModeParams::getCostPublicFirst)
//				.addProperty("cost_public_second",&LogsumTourModeParams::getCostPublicSecond)
//				.addProperty("drive1_AV",&LogsumTourModeParams::isDrive1Available)
//				.addProperty("motor_AV",&LogsumTourModeParams::isMotorAvailable)
//				.addProperty("mrt_AV",&LogsumTourModeParams::isMrtAvailable)
//				.addProperty("privatebus_AV",&LogsumTourModeParams::isPrivateBusAvailable)
//				.addProperty("publicbus_AV",&LogsumTourModeParams::isPublicBusAvailable)
//				.addProperty("share2_AV",&LogsumTourModeParams::isShare2Available)
//				.addProperty("share3_AV",&LogsumTourModeParams::isShare3Available)
//				.addProperty("taxi_AV",&LogsumTourModeParams::isTaxiAvailable)
//				.addProperty("walk_AV",&LogsumTourModeParams::isWalkAvailable)
//				.addProperty("tt_ivt_car_first",&LogsumTourModeParams::getTtCarIvtFirst)
//				.addProperty("tt_ivt_car_second",&LogsumTourModeParams::getTtCarIvtSecond)
//				.addProperty("tt_public_ivt_first",&LogsumTourModeParams::getTtPublicIvtFirst)
//				.addProperty("tt_public_ivt_second",&LogsumTourModeParams::getTtPublicIvtSecond)
//				.addProperty("tt_public_waiting_first",&LogsumTourModeParams::getTtPublicWaitingFirst)
//				.addProperty("tt_public_waiting_second",&LogsumTourModeParams::getTtPublicWaitingSecond)
//				.addProperty("tt_public_walk_first",&LogsumTourModeParams::getTtPublicWalkFirst)
//				.addProperty("tt_public_walk_second",&LogsumTourModeParams::getTtPublicWalkSecond)
//				.addProperty("walk_distance1",&LogsumTourModeParams::getWalkDistance1)
//				.addProperty("walk_distance2",&LogsumTourModeParams::getWalkDistance2)
//				.addProperty("destination_area",&LogsumTourModeParams::getDestinationArea)
//				.addProperty("origin_area",&LogsumTourModeParams::getOriginArea)
//				.addProperty("resident_size",&LogsumTourModeParams::getResidentSize)
//				.addProperty("work_op",&LogsumTourModeParams::getWorkOp)
//				.addProperty("education_op",&LogsumTourModeParams::getEducationOp)
//				.addProperty("cbd_dummy",&LogsumTourModeParams::isCbdDestZone)
//				.addProperty("cbd_dummy_origin",&LogsumTourModeParams::isCbdOrgZone)
//				.addProperty("cost_increase", &LogsumTourModeParams::getCostIncrease)
//			.endClass();
}

void sim_mob::WithindayLuaModel::chooseMode(PersonParams& personParams/*, LogsumTourModeParams& tourModeParams*/) const
{
}

const WithindayLuaModel& sim_mob::WithindayLuaProvider::getWithindayModel()
{
	ensureContext();
	return threadContext.get()->wdModel;
}
