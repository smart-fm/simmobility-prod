/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 9, 2013, 4:39 PM
 */

#include "LuaProxy.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <vector>
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"

using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace luabridge;
using std::vector;

namespace {

    void deleteContext(lua_State* context) {
        if (context) {
            lua_close(context);
        }
    }

    boost::thread_specific_ptr<lua_State> threadContext(deleteContext);

    void ensureContext() {
        if (!threadContext.get()) {
            lua_State* state = luaL_newstate();
            luaL_openlibs(state);
            luaL_dofile(state,"/home/gandola/Desktop/sim_mob.lua");
            threadContext.reset(state);
        }
    }
}

double LuaProxy::HM_SellerExpectation(double price, double expectation, double theta, double alpha) {
    ensureContext();
    LuaRef funcRef = getGlobal(threadContext.get(), "sellerExpectationFunction");
    LuaRef retVal = funcRef(price, expectation, theta, alpha);
    if (retVal.isNumber()) {
        return retVal;
    }
}

 void LuaProxy::arrayTest(vector<double>& out) {
    ensureContext();
    LuaRef funcRef = getGlobal(threadContext.get(), "arrayTest");
    LuaRef retVal = funcRef();
    if (retVal.isTable()) {
        for (int i=1; i <= retVal.length(); i++){
            double x = retVal[i];
            out.push_back(x);
        }      
    }
}