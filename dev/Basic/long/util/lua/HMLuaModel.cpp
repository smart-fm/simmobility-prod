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

HMLuaModel::HMLuaModel() : lua::LuaModel(){
}

HMLuaModel::HMLuaModel(const HMLuaModel& orig) :  lua::LuaModel(orig){
}

HMLuaModel::~HMLuaModel() {
}

void HMLuaModel::mapClasses() {
    getGlobalNamespace (state.get())
    .beginClass <ExpectationEntry> ("ExpectationEntry")
        .addConstructor <void (*) (void)> ()
        .addData ("price", &ExpectationEntry::price)
        .addData ("expectation", &ExpectationEntry::expectation)
    .endClass ();
}

void HMLuaModel::calulateSellerUnitExpectations(const Unit& unit, vector<ExpectationEntry>& outValues) {
    LuaRef funcRef = getGlobal(state.get(), "arrayTest");
    LuaRef retVal = funcRef();
    if (retVal.isTable()) {
        for (int i = 1; i <= retVal.length(); i++) {
            ExpectationEntry entry;
            entry.price = retVal[i].cast<ExpectationEntry>().price;
            entry.expectation = retVal[i].cast<ExpectationEntry>().expectation;
            outValues.push_back(entry);
        }
    }
}
