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
    inline const Postcode* getPostcode(const BigSerial id) {
        return DataManagerSingleton::getInstance().getPostcodeById(id);
    }

    /**
     * Helper function to get the Postcode amenities object by given id.
     * @param id of the postcode.
     * @return PostcodeAmenities pointer or nullptr if post code does not exist.
     */
    inline const PostcodeAmenities* getAmenities(const BigSerial id) {
        return DataManagerSingleton::getInstance().getAmenitiesById(id);
    }

    /**
     * Helper function to get the Building object by given id.
     * @param id of the building.
     * @return Building pointer or nullptr if post code does not exist.
     */
    inline const Building* getBuilding(const BigSerial id) {
        return DataManagerSingleton::getInstance().getBuildingById(id);
    }
}

/******************************************************************************
 *                         EXTERNAL EVENTS LUA
 ******************************************************************************/

ExternalEventsModel::ExternalEventsModel() : lua::LuaModel() {
}

ExternalEventsModel::ExternalEventsModel(const ExternalEventsModel& orig)
: lua::LuaModel(orig) {
}

ExternalEventsModel::~ExternalEventsModel() {
}

void ExternalEventsModel::getExternalEvents(int day,
        vector<ExternalEvent>& outValues) const {
    LuaRef funcRef = getGlobal(state.get(), "getExternalEvents");
    LuaRef retVal = funcRef(day);
    if (retVal.isTable()) {
        for (int i = 1; i <= retVal.length(); i++) {
            outValues.push_back(retVal[i].cast<ExternalEvent>());
        }
    }
}

void ExternalEventsModel::mapClasses() {
    getGlobalNamespace(state.get())
            .beginClass <ExternalEvent> ("ExternalEvent")
            .addConstructor <void (*) (void) > ()
            .addProperty("day", &ExternalEvent::getDay,
            &ExternalEvent::setDay)
            .addProperty("type", &ExternalEvent::getType,
            &ExternalEvent::setType)
            .addProperty("householdId", &ExternalEvent::getHouseholdId,
            &ExternalEvent::setHouseholdId)
            .endClass();
}

/******************************************************************************
 *                         HOUSING MARKET LUA
 ******************************************************************************/

HM_LuaModel::HM_LuaModel() : lua::LuaModel() {
}

HM_LuaModel::HM_LuaModel(const HM_LuaModel& orig) : lua::LuaModel(orig) {
}

HM_LuaModel::~HM_LuaModel() {
}

void HM_LuaModel::mapClasses() {
    getGlobalNamespace(state.get())
            .beginClass <ExpectationEntry> ("ExpectationEntry")
            .addConstructor <void (*) (void) > ()
            .addData("price", &ExpectationEntry::price)
            .addData("expectation", &ExpectationEntry::expectation)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Unit> ("Unit")
            .addProperty("id", &Unit::getId)
            .addProperty("buildingId", &Unit::getBuildingId)
            .addProperty("typeId", &Unit::getTypeId)
            .addProperty("postcodeId", &Unit::getPostcodeId)
            .addProperty("floorArea", &Unit::getFloorArea)
            .addProperty("storey", &Unit::getStorey)
            .addProperty("rent", &Unit::getRent)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <HousingMarket::Entry> ("UnitEntry")
            .addProperty("unit", &HousingMarket::Entry::getUnit)
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
            .addProperty("children", &Household::getChildren)
            .addProperty("income", &Household::getIncome)
            .addProperty("housingDuration", &Household::getHousingDuration)
            .addProperty("workers", &Household::getWorkers)
            .addProperty("ageOfHead", &Household::getAgeOfHead)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Postcode> ("Postcode")
            .addProperty("id", &Postcode::getId)
            .addProperty("code", &Postcode::getCode)
            .addProperty("location", &Postcode::getLocation)
            .addProperty("tazId", &Postcode::getTazId)
            .endClass();
    getGlobalNamespace(state.get())
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
            .addProperty("apartment", &PostcodeAmenities::isApartment)
            .addProperty("condo", &PostcodeAmenities::isCondo)
            .addProperty("terrace", &PostcodeAmenities::isTerrace)
            .addProperty("semi", &PostcodeAmenities::isSemi)
            .addProperty("detached", &PostcodeAmenities::isDetached)
            .addProperty("ec", &PostcodeAmenities::isEc)
            .addProperty("_private", &PostcodeAmenities::isPrivate)
            .addProperty("hdb", &PostcodeAmenities::isHdb)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Building> ("Building")
            .addProperty("id", &Building::getId)
            .addProperty("builtYear", &Building::getBuiltYear)
            .addProperty("landedArea", &Building::getLandedArea)
            .addProperty("parcelId", &Building::getParcelId)
            .addProperty("parkingSpaces", &Building::getParkingSpaces)
            .addProperty("tenureId", &Building::getTenureId)
            .addProperty("typeId", &Building::getTypeId)
            .endClass();
}

void HM_LuaModel::calulateUnitExpectations(const Unit& unit, int timeOnMarket,
        vector<ExpectationEntry>& outValues) const {
    LuaRef funcRef = getGlobal(state.get(), "calulateUnitExpectations");
    LuaRef retVal = funcRef(&unit, timeOnMarket);
    if (retVal.isTable()) {
        for (int i = 1; i <= retVal.length(); i++) {
            ExpectationEntry entry;
            entry.price = retVal[i].cast<ExpectationEntry>().price;
            entry.expectation = retVal[i].cast<ExpectationEntry>().expectation;
            outValues.push_back(entry);
        }
    }
}

double HM_LuaModel::calculateHedonicPrice(const Unit& unit) const {
    const BigSerial pcId = unit.getPostcodeId();
    LuaRef funcRef = getGlobal(state.get(), "calculateHedonicPrice");
    LuaRef retVal = funcRef(&unit, getBuilding(unit.getBuildingId()),
            getPostcode(pcId), getAmenities(pcId));
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HM_LuaModel::calculateSurplus(const HousingMarket::Entry& entry, int unitBids) const {
    LuaRef funcRef = getGlobal(state.get(), "calculateSurplus");
    LuaRef retVal = funcRef(&entry, unitBids);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HM_LuaModel::calulateWP(const Household& hh, const Unit& unit) const {
    LuaRef funcRef = getGlobal(state.get(), "calculateWP");
    LuaRef retVal = funcRef(&hh, &unit);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}