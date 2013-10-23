/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   HMLuaModel.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 10, 2013, 2:39 PM
 */

#include "HMLuaModel.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace luabridge;
using std::vector;

HMLuaModel::HMLuaModel() : lua::LuaModel() {
}

HMLuaModel::HMLuaModel(const HMLuaModel& orig) : lua::LuaModel(orig) {
}

HMLuaModel::~HMLuaModel() {
}

void HMLuaModel::mapClasses() {
    getGlobalNamespace(state.get())
            .beginClass <ExpectationEntry> ("ExpectationEntry")
            .addConstructor <void (*) (void) > ()
            .addData("price", &ExpectationEntry::price)
            .addData("expectation", &ExpectationEntry::expectation)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Unit> ("Unit")
            .addProperty("id", &Unit::GetId)
            .addProperty("buildingId", &Unit::GetBuildingId)
            .addProperty("typeId", &Unit::GetTypeId)
            .addProperty("postcodeId", &Unit::GetPostcodeId)
            .addProperty("floorArea", &Unit::GetFloorArea)
            .addProperty("storey", &Unit::GetStorey)
            .addProperty("rent", &Unit::GetRent)
            .addProperty("hedonicPrice", &Unit::GetHedonicPrice)
            .addProperty("askingPrice", &Unit::GetAskingPrice)
            .endClass();
    getGlobalNamespace(state.get())
            .beginClass <Household> ("Household")
            .addProperty("id", &Household::GetId)
            .addProperty("lifestyleId", &Household::GetLifestyleId)
            .addProperty("unitId", &Household::GetUnitId)
            .addProperty("ethnicityId", &Household::GetEthnicityId)
            .addProperty("vehicleCategoryId", &Household::GetVehicleCategoryId)
            .addProperty("size", &Household::GetSize)
            .addProperty("children", &Household::GetChildren)
            .addProperty("income", &Household::GetIncome)
            .addProperty("housingDuration", &Household::GetHousingDuration)
            .addProperty("workers", &Household::GetWorkers)
            .addProperty("ageOfHead", &Household::GetAgeOfHead)
            .endClass();
}

void HMLuaModel::calulateUnitExpectations(const Unit& unit, int timeOnMarket,
        vector<ExpectationEntry>& outValues) {
    LuaRef funcRef = getGlobal(state.get(), "calulateUnitExpectations");
    LuaRef retVal = funcRef(unit, timeOnMarket);
    if (retVal.isTable()) {
        for (int i = 1; i <= retVal.length(); i++) {
            ExpectationEntry entry;
            entry.price = retVal[i].cast<ExpectationEntry>().price;
            entry.expectation = retVal[i].cast<ExpectationEntry>().expectation;
            outValues.push_back(entry);
        }
    }
}

double HMLuaModel::calculateHedonicPrice(const Unit& unit) {
    LuaRef funcRef = getGlobal(state.get(), "calculateHedonicPrice");
    LuaRef retVal = funcRef(unit);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HMLuaModel::calculateSurplus(const Unit& unit, int unitBids) {
    LuaRef funcRef = getGlobal(state.get(), "calculateSurplus");
    LuaRef retVal = funcRef(unit, unitBids);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}

double HMLuaModel::calulateWP(const Household& hh, const Unit& unit) {
    LuaRef funcRef = getGlobal(state.get(), "calculateWP");
    LuaRef retVal = funcRef(hh, unit);
    if (retVal.isNumber()) {
        return retVal.cast<double>();
    }
    return INVALID_DOUBLE;
}