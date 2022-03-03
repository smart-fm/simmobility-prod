//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayLuaModel.hpp"

#include "behavioral/StopType.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "logging/Log.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;
using namespace luabridge;

namespace
{
const int NUM_ZONES = 1169;
}

sim_mob::medium::PredayLuaModel::PredayLuaModel()
{}

sim_mob::medium::PredayLuaModel::~PredayLuaModel()
{}

void sim_mob::medium::PredayLuaModel::mapClasses()
{
    getGlobalNamespace(state.get())
            .beginClass <PersonParams> ("PersonParams")
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
                .addProperty("vehicle_ownership_category", &PersonParams::getVehicleOwnershipCategory)
                .addProperty("has_driving_licence", &PersonParams::hasDrivingLicence)
                .addProperty("fixed_work_hour", &PersonParams::getHasFixedWorkTiming) //not used in lua
                .addProperty("homeLocation", &PersonParams::getHomeLocation) //not used in lua
                .addProperty("fixed_place", &PersonParams::hasFixedWorkPlace)
                .addProperty("fixedSchoolLocation", &PersonParams::getFixedSchoolLocation) //not used in lua
                .addProperty("only_adults", &PersonParams::getHH_OnlyAdults)
                .addProperty("only_workers", &PersonParams::getHH_OnlyWorkers)
                .addProperty("num_underfour", &PersonParams::getHH_NumUnder4)
                .addProperty("presence_of_under15", &PersonParams::getHH_HasUnder15)
                .addFunction("activity_logsum", &PersonParams::getActivityLogsum)
                .addProperty("dptour_logsum", &PersonParams::getDptLogsum)
                .addProperty("dpstop_logsum", &PersonParams::getDpsLogsum)
                .addProperty("studentTypeId", &PersonParams::getStudentTypeId)
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
                .addFunction("getModeAvailability",&TourModeParams::isModeAvailable)
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
                .addProperty("cbd_dummy",&TourModeParams::isCbdDestZone)
                .addProperty("cbd_dummy_origin",&TourModeParams::isCbdOrgZone)
                .addProperty("cost_increase", &TourModeParams::getCostIncrease)
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
                .addProperty("mode_to_work", &TourModeDestinationParams::getModeForParentWorkTour)
                .addFunction("availability",&TourModeDestinationParams::isAvailable_TMD)
                .addProperty("cbd_dummy_origin",&TourModeDestinationParams::isCbdOrgZone)
                .addFunction("cbd_dummy",&TourModeDestinationParams::getCbdDummy)
                .addProperty("cost_increase", &TourModeDestinationParams::getCostIncrease)
            .endClass()

            .beginClass<StopModeDestinationParams>("StopModeDestinationParams")
                .addProperty("stop_type", &StopModeDestinationParams::getTourPurpose)
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
                .addProperty("first_bound", &StopModeDestinationParams::isFirstBound)
                .addProperty("second_bound", &StopModeDestinationParams::isSecondBound)
                .addFunction("availability",&StopModeDestinationParams::isAvailable_IMD)
                .addProperty("cbd_dummy_origin",&StopModeDestinationParams::isCbdOrgZone)
                .addFunction("cbd_dummy",&StopModeDestinationParams::getCbdDummy)
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
                .addProperty("cbd_dummy",&TourTimeOfDayParams::isCbdDestZone)
                .addProperty("cbd_dummy_origin",&TourTimeOfDayParams::isCbdOrgZone)
                .addProperty("mode",&TourTimeOfDayParams::getTourMode)
            .endClass()

            .beginClass<StopTimeOfDayParams>("StopTimeOfDayParams")
                .addProperty("stop_type", &StopTimeOfDayParams::getStopType)
                .addFunction("TT", &StopTimeOfDayParams::getTravelTime)
                .addFunction("cost", &StopTimeOfDayParams::getTravelCost)
                .addProperty("high_tod", &StopTimeOfDayParams::getTodHigh)
                .addProperty("low_tod", &StopTimeOfDayParams::getTodLow)
                .addProperty("first_bound", &StopTimeOfDayParams::getFirstBound)
                .addProperty("second_bound", &StopTimeOfDayParams::getSecondBound)
                .addFunction("availability", &StopTimeOfDayParams::getAvailability)
                .addProperty("cbd_dummy",&StopTimeOfDayParams::isCbdDestZone)
                .addProperty("cbd_dummy_origin",&StopTimeOfDayParams::isCbdOrgZone)
                .addProperty("mode",&StopTimeOfDayParams::getStopMode)
            .endClass()

            .beginClass<SubTourParams>("SubTourParams")
                .addProperty("activity_type", &SubTourParams::getSubTourPurpose)
                .addProperty("first_of_multiple", &SubTourParams::isFirstOfMultipleTours)
                .addProperty("subsequent_of_multiple", &SubTourParams::isSubsequentOfMultipleTours)
                .addProperty("mode_choice",  &SubTourParams::getTourMode)
                .addProperty("usual_location", &SubTourParams::isUsualLocation)
                .addFunction("time_window_availability", &SubTourParams::getTimeWindowAvailability)
                .addProperty("cbd_dummy",&SubTourParams::isCbdDestZone)
                .addProperty("cbd_dummy_origin",&SubTourParams::isCbdOrgZone)
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
                .addProperty("time_window_first_bound", &StopGenerationParams::getTimeWindowFirstBound)
                .addProperty("time_window_second_bound", &StopGenerationParams::getTimeWindowSecondBound)
                .addFunction("availability", &StopGenerationParams::isAvailable)
            .endClass()

            .beginClass<ZoneAddressParams>("ZoneAddressParams")
                .addProperty("num_addresses", &ZoneAddressParams::getNumAddresses)
                .addFunction("distance_mrt", &ZoneAddressParams::getDistanceMRT)
                .addFunction("distance_bus", &ZoneAddressParams::getDistanceBus)
            .endClass();

}

void sim_mob::medium::PredayLuaModel::computeDayPatternLogsums(PersonParams& personParams) const
{
    LuaRef computeLogsumDPT = getGlobal(state.get(), "compute_logsum_dpt");
    LuaRef dptRetVal = computeLogsumDPT(personParams);
    if(dptRetVal.isTable())
    {
        personParams.setDptLogsum(dptRetVal[1].cast<double>());
    }
    else
    {
        throw std::runtime_error("compute_logsum_dpt function does not return a table as expected");
    }

    LuaRef computeLogsumDPS = getGlobal(state.get(), "compute_logsum_dps");
    LuaRef dpsLogsum = computeLogsumDPS(personParams);
    personParams.setDpsLogsum(dpsLogsum.cast<double>());
}

void sim_mob::medium::PredayLuaModel::computeDayPatternBinaryLogsums(PersonParams& personParams) const
{
    LuaRef computeLogsumDPB = getGlobal(state.get(), "compute_logsum_dpb");
    LuaRef dpbRetVal = computeLogsumDPB(personParams);
    if(dpbRetVal.isTable())
    {
        personParams.setDpbLogsum(dpbRetVal[1].cast<double>());
    }
    else
    {
        throw std::runtime_error("compute_logsum_dpb function does not return a table as expected");
    }
}

void sim_mob::medium::PredayLuaModel::predictDayPattern(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                        std::unordered_map<int, bool> &dayPatternTours, std::unordered_map<int, bool> &dayPatternStops) const
{
    LuaRef chooseDPB = getGlobal(state.get(), "choose_dpb");
    LuaRef retValB = chooseDPB(&personParams);
    if(retValB.cast<int>() == 1) // no travel
    {
        for (const auto& activityType : activityTypes)
        {
            dayPatternTours[activityType.first] = 0;
            dayPatternStops[activityType.first] = 0;
        }
    }
    else
    {
        //Day pattern tours
        LuaRef chooseDPT = getGlobal(state.get(), "choose_dpt");
        LuaRef retValT = chooseDPT(&personParams);
        if (retValT.isTable())
        {
            for (const auto& activityType : activityTypes)
            {
                dayPatternTours[activityType.first] = retValT[activityType.first].cast<int>();
            }
        }
        else
        {
            throw std::runtime_error("Error in day pattern tours prediction. Unexpected return value");
        }

        //Day pattern stops
        LuaRef chooseDPS = getGlobal(state.get(), "choose_dps");
        LuaRef retValS = chooseDPS(&personParams);
        if (retValS.isTable())
        {
            for (const auto& activityType : activityTypes)
            {
                dayPatternStops[activityType.first] = retValS[activityType.first].cast<int>();
            }
        }
        /*else
        {
            throw std::runtime_error("Error in day pattern stops prediction. Unexpected return value");
        }*/
    }
}

void sim_mob::medium::PredayLuaModel::predictNumTours(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                      std::unordered_map<int, bool> &dayPatternTours, std::unordered_map<int, int>& numTours) const
{
    for (const auto& dayPatternTour : dayPatternTours)
    {
        numTours[dayPatternTour.first] = 0;

        if (dayPatternTour.second)
        {
            std::string luaFunc = "choose_" + activityTypes.at(dayPatternTour.first).numToursModel;
            LuaRef chooseNT = getGlobal(state.get(), luaFunc.c_str());
            LuaRef retVal = chooseNT(&personParams);
            if (retVal.isNumber())
            {
                numTours[dayPatternTour.first] = retVal.cast<int>();
            }
        }
    }
}

bool sim_mob::medium::PredayLuaModel::predictUsualWorkLocation(PersonParams& personParams, UsualWorkParams& usualWorkParams) const
{
    LuaRef chooseUW = getGlobal(state.get(), "choose_uw"); // choose usual work location
    LuaRef retVal = chooseUW(&personParams, &usualWorkParams);
    if (!retVal.isNumber())
    {
        throw std::runtime_error("Error in usual work location model. Unexpected return value");
    }
    return retVal.cast<bool>();
}

int sim_mob::medium::PredayLuaModel::predictTourMode(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                     TourModeParams& tourModeParams) const
{
    const std::string& tmModel = activityTypes.at(tourModeParams.getStopType()).tourModeModel;

    if (!tmModel.empty())
    {
        std::string luaFunc = "choose_" + tmModel;
        LuaRef chooseTM = getGlobal(state.get(), luaFunc.c_str());
        LuaRef retVal = chooseTM(&personParams, &tourModeParams);
        return retVal.cast<int>();
    }
    else
        {
        throw std::runtime_error("Tour mode model cannot be invoked for " + activityTypes.at(tourModeParams.getStopType()).name + " tour type");
        return -1;
    }
}

void sim_mob::medium::PredayLuaModel::computeTourModeLogsumWork(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                            TourModeParams& tourModeParams) const
{
    for (const auto& activity : activityTypes)
    {
        const ActivityTypeConfig& actConfig = activity.second;
        if (actConfig.type == WORK_ACTIVITY_TYPE && personParams.hasFixedWorkPlace())
        {
            if (!actConfig.tourModeModel.empty())
            {
                std::string luaFunc = "compute_logsum_" + actConfig.tourModeModel;
                LuaRef computeLogsumTMW = getGlobal(state.get(), luaFunc.c_str());
                LuaRef workLogSum = computeLogsumTMW(&personParams, &tourModeParams);
                personParams.setActivityLogsum(activity.first, workLogSum.cast<double>());
            }
        }
    }
}

void sim_mob::medium::PredayLuaModel::computeTourModeLogsumEducation(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                            TourModeParams& tourModeParams) const
{
    for (const auto& activity : activityTypes)
    {
        const ActivityTypeConfig& actConfig = activity.second;
      
        if (actConfig.type == EDUCATION_ACTIVITY_TYPE && personParams.isStudent())
        {
            if (!actConfig.tourModeModel.empty())
            {
                std::string luaFunc = "compute_logsum_" + actConfig.tourModeModel;
                LuaRef computeLogsumTME = getGlobal(state.get(), luaFunc.c_str());
                LuaRef eduLogSum = computeLogsumTME(&personParams, &tourModeParams);
                personParams.setActivityLogsum(activity.first, eduLogSum.cast<double>());
            }
        }
    }
}

void sim_mob::medium::PredayLuaModel::computeTourModeDestinationLogsum(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                                       TourModeDestinationParams& tourModeDestinationParams, int zoneSize) const
{
    for (const auto& activity : activityTypes)
    {
        const ActivityTypeConfig& actConfig = activity.second;
        if (actConfig.type == EDUCATION_ACTIVITY_TYPE)
        {
                continue;
        }

        if (actConfig.type == WORK_ACTIVITY_TYPE && !personParams.hasFixedWorkPlace())
        {
            if (!actConfig.tourModeDestModel.empty())
            {
                std::string luaFunc = "compute_logsum_" + actConfig.tourModeDestModel;
                LuaRef computeLogsumTMDW = getGlobal(state.get(), luaFunc.c_str());
                LuaRef workLogSum = computeLogsumTMDW(&personParams, &tourModeDestinationParams, zoneSize);
                personParams.setActivityLogsum(activity.first, workLogSum.cast<double>());
            }
        }
        else
        {
            if (!actConfig.tourModeDestModel.empty())
            {
                 std::string luaFunc = "compute_logsum_" + actConfig.tourModeDestModel;
                LuaRef computeLogsumTMD = getGlobal(state.get(), luaFunc.c_str());
                LuaRef logsum = computeLogsumTMD(&personParams, &tourModeDestinationParams, zoneSize);
                personParams.setActivityLogsum(activity.first, logsum.cast<double>());
            }
        }
    }
}

int sim_mob::medium::PredayLuaModel::predictTourModeDestination(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                                TourModeDestinationParams& tourModeDestinationParams) const
{
    const std::string& tmdModel = activityTypes.at(tourModeDestinationParams.getTourPurpose()).tourModeDestModel;

    if(!tmdModel.empty())
    {
        std::string luaFunc = "choose_" + tmdModel;
        LuaRef chooseTMD = getGlobal(state.get(), luaFunc.c_str());
        LuaRef retVal = chooseTMD(&personParams, &tourModeDestinationParams);
        return retVal.cast<int>();
    }
    else
        {
            throw std::runtime_error("Tour mode model cannot be invoked for Shopping and Other tour types");
        return -1;
    }
}

void PredayLuaModel::initializeLogsums(PersonParams &personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes) const
{
    for (const auto& activity : activityTypes)
    {
        personParams.setActivityLogsum(activity.first, 0.0);
    }
}

int sim_mob::medium::PredayLuaModel::predictTourTimeOfDay(PersonParams& personParams, const std::unordered_map<int, ActivityTypeConfig> &activityTypes,
                                                          TourTimeOfDayParams& tourTimeOfDayParams, StopType tourType) const
{
    const std::string& modelName = activityTypes.at(tourType).tourTimeOfDayModel;
    if (!modelName.empty())
    {
        std::string luaFunc = "choose_" + modelName;
        LuaRef chooseTTD = getGlobal(state.get(), luaFunc.c_str());
        LuaRef retVal = chooseTTD(&personParams, &tourTimeOfDayParams);
        return retVal.cast<int>();
    }
    throw std::runtime_error("Error. Empty model name in PredayLuaModel::predictTourTimeOfDay.");
}

int sim_mob::medium::PredayLuaModel::generateIntermediateStop(PersonParams& personParams, StopGenerationParams& isgParams) const
{
    LuaRef chooseISG = getGlobal(state.get(), "choose_isg");
    LuaRef retVal = chooseISG(&personParams, &isgParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictStopModeDestination(PersonParams& personParams, StopModeDestinationParams& imdParams) const
{
    LuaRef chooseIMD = getGlobal(state.get(), "choose_imd");
    LuaRef retVal = chooseIMD(&personParams, &imdParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictStopTimeOfDay(PersonParams& personParams, StopTimeOfDayParams& stopTimeOfDayParams) const
{
    LuaRef chooseITD = getGlobal(state.get(), "choose_itd");
    LuaRef retVal = chooseITD(&personParams, &stopTimeOfDayParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictWorkBasedSubTour(PersonParams& personParams, SubTourParams& subTourParams) const
{
    LuaRef chooseTWS = getGlobal(state.get(), "choose_tws");
    LuaRef retVal = chooseTWS(&personParams, &subTourParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictSubTourModeDestination(PersonParams& personParams, TourModeDestinationParams& tourModeDestinationParams) const
{
    LuaRef chooseSTMD = getGlobal(state.get(), "choose_stmd");
    LuaRef retVal = chooseSTMD(&personParams, &tourModeDestinationParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictSubTourTimeOfDay(PersonParams& personParams, SubTourParams& subTourParams) const
{
    LuaRef chooseSTTD = getGlobal(state.get(), "choose_sttd");
    LuaRef retVal = chooseSTTD(&personParams, &subTourParams);
    return retVal.cast<int>();
}

int sim_mob::medium::PredayLuaModel::predictAddress(ZoneAddressParams& znAddressParams) const
{
    LuaRef chooseAddress = getGlobal(state.get(), "choose_address");
    LuaRef retVal = chooseAddress(&znAddressParams);
    return znAddressParams.getAddressId(retVal.cast<int>());
}
