/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HMLuaModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 10, 2013, 2:39 PM
 */

#include "LT_LuaModels.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "core/DataManager.hpp"
#include "model/DeveloperModel.hpp"
#include "model/HM_Model.hpp"


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace luabridge;
using std::vector;

namespace {

    /**
     * Helper function to get the Postcode object by given id.
     * @param id of the postcode.
     * @return Postcode pointer or nullptr if post code does not exist.
     */
    inline const Postcode* getPostcode(const BigSerial id)
    {
        return DataManagerSingleton::getInstance().getPostcodeById(id);
    }

    /**
     * Helper function to get the Postcode amenities object by given id.
     * @param id of the postcode.
     * @return PostcodeAmenities pointer or nullptr if post code does not exist.
     */
    inline const PostcodeAmenities* getAmenities(const BigSerial id)
    {
        return DataManagerSingleton::getInstance().getAmenitiesById(id);
    }

    /**
     * Helper function to get the Building object by given id.
     * @param id of the building.
     * @return Building pointer or nullptr if post code does not exist.
     */
    inline const Building* getBuilding(const BigSerial id)
    {
        return DataManagerSingleton::getInstance().getBuildingById(id);
    }
    
    /**
     * Maps common classes for lua models 
     * @param state
     */
    inline void mapCommonClasses(lua_State* state)
    {
        getGlobalNamespace(state)
            .beginClass <Unit> ("Unit")
            .addProperty("fmUnitId", &Unit::getId)
        	.addProperty("fmBuildingId", &Unit::getBuildingId)
        	.addProperty("slaAddressId", &Unit::getSlaAddressId)
        	.addProperty("unitType", &Unit::getUnitType)
        	.addProperty("storeyRange", &Unit::getStoreyRange)
        	.addProperty("unitStatus", &Unit::getUnitStatus)
        	.addProperty("floorArea", &Unit::getFloorArea)
        	.addProperty("storey", &Unit::getStorey)
        	.addProperty("rent", &Unit::getRent)
        	.addProperty("saleFromDate", &Unit::getSaleFromDate)
        	.addProperty("physicalFromYear", &Unit::getPhysicalFromYear)
        	.addProperty("saleStatus", &Unit::getSaleStatus)
        	.addProperty("physicalStatus", &Unit::getPhysicalStatus)
            .endClass();
    getGlobalNamespace(state)
            .beginClass <Postcode> ("Postcode")
            .addProperty("address_id", &Postcode::getAddressId)
            .addProperty("sla_postcode", &Postcode::getSlaPostcode)
            .addProperty("taz_id", &Postcode::getTazId)
            .addProperty("longitude", &Postcode::getLongitude)
            .addProperty("latitude", &Postcode::getLatitude)
            .addProperty("primary_postcode", &Postcode::getPrimaryPostcode)
            .endClass();
    getGlobalNamespace(state)
            .beginClass <PostcodeAmenities> ("PostcodeAmenities")
            .addProperty("postcode", &PostcodeAmenities::getPostcode)
            .addProperty("buildingName", &PostcodeAmenities::getBuildingName)
            .addProperty("unitBlock", &PostcodeAmenities::getUnitBlock)
            .addProperty("roadName", &PostcodeAmenities::getRoadName)
            .addProperty("mtzNumber", &PostcodeAmenities::getMtzNumber)
            .addProperty("mrtStation", &PostcodeAmenities::getMrtStation)
            .addProperty("distanceToMRT", &PostcodeAmenities::getDistanceToMRT)
            .addProperty("distanceToBus", &PostcodeAmenities::getDistanceToBus)
            .addProperty("distanceToExpress", &PostcodeAmenities::getDistanceToExpress)
            .addProperty("distanceToPMS30", &PostcodeAmenities::getDistanceToPMS30)
            .addProperty("distanceToCBD", &PostcodeAmenities::getDistanceToCBD)
            .addProperty("distanceToMall", &PostcodeAmenities::getDistanceToMall)
            .addProperty("distanceToJob", &PostcodeAmenities::getDistanceToJob)
            .addProperty("mrt_200m", &PostcodeAmenities::hasMRT_200m)
            .addProperty("mrt_400m", &PostcodeAmenities::hasMRT_400m)
            .addProperty("express_200m", &PostcodeAmenities::hasExpress_200m)
            .addProperty("bus_200m", &PostcodeAmenities::hasBus_200m)
            .addProperty("bus_400m", &PostcodeAmenities::hasBus_400m)
            .addProperty("pms_1km", &PostcodeAmenities::hasPms_1km)
            .endClass();
    getGlobalNamespace(state)
            .beginClass <ParcelAmenities> ("ParcelAmenities")
            .addProperty("fmParcelId", &ParcelAmenities::getFmParcelId)
            .addProperty("nearestMRT", &ParcelAmenities::getNearestMRT)
            .addProperty("distanceToMRT", &ParcelAmenities::getDistanceToMRT)
            .addProperty("distanceToBus", &ParcelAmenities::getDistanceToBus)
            .addProperty("distanceToExpress", &ParcelAmenities::getDistanceToExpress)
            .addProperty("distanceToPMS30", &ParcelAmenities::getDistanceToPMS30)
            .addProperty("distanceToCBD", &ParcelAmenities::getDistanceToCBD)
            .addProperty("distanceToMall", &ParcelAmenities::getDistanceToMall)
            .addProperty("distanceToJob", &ParcelAmenities::getDistanceToJob)
            .addProperty("mrt_200m", &ParcelAmenities::hasMRT_200m)
            .addProperty("mrt_400m", &ParcelAmenities::hasMRT_400m)
            .addProperty("express_200m", &ParcelAmenities::hasExpress_200m)
            .addProperty("bus_200m", &ParcelAmenities::hasBus_200m)
            .addProperty("bus_400m", &ParcelAmenities::hasBus_400m)
            .addProperty("pms_1km", &ParcelAmenities::hasPms_1km)
            .endClass();
    getGlobalNamespace(state)
            .beginClass <Building> ("Building")
            .addProperty("fmBuildingId", &Building::getFmBuildingId)
            .addProperty("fmProjectId", &Building::getFmProjectId)
            .addProperty("fmParcelId", &Building::getFmParcelId)
            .addProperty("storeysAboveGround", &Building::getStoreysAboveGround)
            .addProperty("storeysBelowGround", &Building::getStoreysBelowGround)
            .addProperty("fromDate", &Building::getFromDate)
            .addProperty("toDate", &Building::getToDate)
            .addProperty("buildingStatus", &Building::getBuildingStatus)
            .addProperty("grossSqmRes", &Building::getGrossSqmRes)
            .addProperty("grossSqmOffice", &Building::getGrossSqmOffice)
            .addProperty("grossSqmRetail", &Building::getGrossSqmRetail)
            .addProperty("grossSqmOther", &Building::getGrossSqmOther)
            .endClass();
    }
}

/******************************************************************************
 *                         EXTERNAL EVENTS LUA
 ******************************************************************************/

ExternalEventsModel::ExternalEventsModel() : lua::LuaModel() {}

ExternalEventsModel::ExternalEventsModel(const ExternalEventsModel& orig): lua::LuaModel(orig) {}

ExternalEventsModel::~ExternalEventsModel() {}

void ExternalEventsModel::getExternalEvents(int day, vector<ExternalEvent>& outValues) const
{
    LuaRef funcRef = getGlobal(state.get(), "getExternalEvents");
    LuaRef retVal = funcRef(day);

    if (retVal.isTable())
    {
        for (int i = 1; i <= retVal.length(); i++)
        {
            outValues.push_back(retVal[i].cast<ExternalEvent>());
        }
    }
}

void ExternalEventsModel::mapClasses()
{
    getGlobalNamespace(state.get())
        .beginClass <ExternalEvent> ("ExternalEvent")
        .addConstructor <void (*) (void) > ()
        .addProperty("day", &ExternalEvent::getDay, &ExternalEvent::setDay)
        .addProperty("type", &ExternalEvent::getType, &ExternalEvent::setType)
        .addProperty("householdId", &ExternalEvent::getHouseholdId, &ExternalEvent::setHouseholdId)
        .addProperty("developerId", &ExternalEvent::getDeveloperId, &ExternalEvent::setDeveloperId)
    .endClass();
}

/******************************************************************************
 *                         HOUSING MARKET LUA
 ******************************************************************************/

HM_LuaModel::HM_LuaModel() : lua::LuaModel() {}

HM_LuaModel::HM_LuaModel(const HM_LuaModel& orig) : lua::LuaModel(orig) {}

HM_LuaModel::~HM_LuaModel() {}

void HM_LuaModel::mapClasses()
{
    getGlobalNamespace(state.get())
            .beginClass <HM_Model::TazStats> ("TazStats")
            .addProperty("hhNum", &HM_Model::TazStats::getHH_Num)
            .addProperty("hhTotalIncome", &HM_Model::TazStats::getHH_TotalIncome)
            .addProperty("hhAvgIncome", &HM_Model::TazStats::getHH_AvgIncome)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <ExpectationEntry> ("ExpectationEntry")
            .addConstructor <void (*) (void) > ()
            .addData("hedonicPrice", &ExpectationEntry::hedonicPrice)
            .addData("askingPrice", &ExpectationEntry::askingPrice)
            .addData("targetPrice", &ExpectationEntry::targetPrice)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <HousingMarket::Entry> ("UnitEntry")
            .addProperty("tazId", &HousingMarket::Entry::getTazId)
            .addProperty("postcodeId", &HousingMarket::Entry::getPostcodeId)
            .addProperty("hedonicPrice", &HousingMarket::Entry::getHedonicPrice)
            .addProperty("askingPrice", &HousingMarket::Entry::getAskingPrice)
            .addProperty("unitId", &HousingMarket::Entry::getUnitId)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Household> ("Household")
            .addProperty("id", &Household::getId)
            .addProperty("lifestyleId", &Household::getLifestyleId)
            .addProperty("unitId", &Household::getUnitId)
            .addProperty("ethnicityId", &Household::getEthnicityId)
            .addProperty("vehicleCategoryId", &Household::getVehicleCategoryId)
            .addProperty("size", &Household::getSize)
            .addProperty("childUnder4", &Household::getChildUnder4)
            .addProperty("childUnder15", &Household::getChildUnder15)
            .addProperty("income", &Household::getIncome)
            .addProperty("housingDuration", &Household::getHousingDuration)
            .addProperty("workers", &Household::getWorkers)
            .addProperty("ageOfHead", &Household::getAgeOfHead)
            .endClass();
    
    mapCommonClasses(state.get());
}

void HM_LuaModel::calulateUnitExpectations(const Unit& unit, int timeOnMarket, double logsum, double lagCoefficient, vector<ExpectationEntry>& outValues ) const
{
    const BigSerial pcId = unit.getSlaAddressId();
    LuaRef funcRef = getGlobal(state.get(), "calulateUnitExpectations");

	LuaRef retVal = funcRef(&unit, timeOnMarket, logsum, lagCoefficient, getBuilding(unit.getBuildingId()), getPostcode(pcId), getAmenities(pcId));

    if (retVal.isTable())
    {
        // Reverse the expectations (HM requirement).
        for (int i = retVal.length(); i >= 1; i--)
        {
            ExpectationEntry entry = retVal[i].cast<ExpectationEntry>();
            outValues.push_back(entry);
        }
    }

    if( retVal.length() == 0 )
    {
    	const Building* build = getBuilding(unit.getBuildingId());
    	const Postcode* postcode = getPostcode(pcId);
    	const PostcodeAmenities* amen = getAmenities(pcId);
    	std::string buildingName = amen == NULL? "<empty>": amen->getBuildingName();

    	PrintOutV("[ERROR] Unit Expectations is empty for unit " << unit.getId() << " from building ID: "  << build->getFmBuildingId() << " at addressId: " << postcode->getAddressId() << " with building name: " << buildingName << std::endl );

    }
}

double HM_LuaModel::calculateHedonicPrice(const Unit& unit) const
{
    const BigSerial pcId = unit.getSlaAddressId();
    LuaRef funcRef = getGlobal(state.get(), "calculateHedonicPrice");
    LuaRef retVal = funcRef(&unit, getBuilding(unit.getBuildingId()), getPostcode(pcId), getAmenities(pcId));
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HM_LuaModel::calculateSpeculation(const HousingMarket::Entry& entry, int unitBids) const
{
    LuaRef funcRef = getGlobal(state.get(), "calculateSpeculation");
    LuaRef retVal = funcRef(&entry, unitBids);

    if (retVal.isNumber())
    {
        return retVal.cast<double>();
    }

    return INVALID_DOUBLE;
}

double HM_LuaModel::calulateWP(const Household& hh, const Unit& unit, const HM_Model::TazStats& stats) const
{
    const BigSerial pcId = unit.getSlaAddressId();
    LuaRef funcRef = getGlobal(state.get(), "calculateWP");
    LuaRef retVal = funcRef(&hh, &unit, &stats, getAmenities(pcId));
    if (retVal.isNumber())
    {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

/******************************************************************************
 *                         Developer LUA
 ******************************************************************************/

DeveloperLuaModel::DeveloperLuaModel() : lua::LuaModel() {}

DeveloperLuaModel::~DeveloperLuaModel() {}

void DeveloperLuaModel::mapClasses()
{
    getGlobalNamespace(state.get())
            .beginClass <PotentialUnit> ("PotentialUnit")
            .addProperty("floorArea", &PotentialUnit::getFloorArea)
            .addProperty("unitTypeId", &PotentialUnit::getUnitTypeId)
            .addProperty("freehold", &PotentialUnit::isFreehold)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <LogsumForDevModel> ("LogsumForDevModel")
            .addProperty("fmParcelId", &LogsumForDevModel::gettAZ2012Id)
            .addProperty("tazId", &LogsumForDevModel::getTaz2008Id)
            .addProperty("accessibility", &LogsumForDevModel::getAccessibility)
            .endClass();
    mapCommonClasses(state.get());
}

double DeveloperLuaModel::calculateUnitRevenue(const PotentialUnit& unit,const ParcelAmenities& amenities, double logsum, int quarter, int futureYear, double HPIfromData,int age) const {

    LuaRef funcRef = getGlobal(state.get(), "calculateUnitRevenue");
    LuaRef retVal = funcRef(&unit, &amenities, logsum, quarter,futureYear,HPIfromData,age);

    if (retVal.isNumber())
    {
        return retVal.cast<double>();
    }

    return INVALID_DOUBLE;
}
