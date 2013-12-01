/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 9, 2013, 4:39 PM
 */

#include "PredayLuaProvider.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using namespace sim_mob::lua;

namespace {

    struct ModelContext {
        PredayLuaModel predayModel;
    };

    boost::thread_specific_ptr<ModelContext> threadContext;

    void ensureContext() {
        if (!threadContext.get()) {
        	std::string externalScriptsId = ConfigManager::GetInstance().FullConfig().system.genericProps.at("external_scripts");
        	ExternalScriptsMap extScripts = ConfigManager::GetInstance().FullConfig().constructs.externalScriptsMap.at(externalScriptsId);
            ModelContext* modelCtx = new ModelContext();
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("logit"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("dp"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("ntw"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("nte"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("nts"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("nto"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("uw"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("tme"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("tmw"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("tmdw"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("tmds"));
            modelCtx->predayModel.loadFile(extScripts.path + extScripts.scriptFileName.at("tmdo"));
            modelCtx->predayModel.initialize();
            threadContext.reset(modelCtx);
        }
    }
}

const PredayLuaModel& PredayLuaProvider::getPredayModel() {
    ensureContext();
    return threadContext.get()->predayModel;
}
