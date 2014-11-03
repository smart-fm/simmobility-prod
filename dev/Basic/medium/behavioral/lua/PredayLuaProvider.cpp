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

#include "config/MT_Config.hpp"

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
        	try {
				const ModelScriptsMap& extScripts = MT_Config::getInstance().getModelScriptsMap();
				const std::string& scriptsPath = extScripts.getPath();
				ModelContext* modelCtx = new ModelContext();
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("logit"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dpb"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dpt"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dps"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("ntw"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("nte"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("nts"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("nto"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("uw"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tme"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmw"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmdw"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmds"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmdo"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("ttdw"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("ttde"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("ttdo"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tws"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("stmd"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("sttd"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("isg"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("imd"));
				modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("itd"));
				modelCtx->predayModel.initialize();
				threadContext.reset(modelCtx);
        	}
        	catch (const std::out_of_range& oorx) {
        		throw std::runtime_error("missing or invalid generic property 'external_scripts'");
        	}
        }
    }
}

const PredayLuaModel& PredayLuaProvider::getPredayModel() {
    ensureContext();
    return threadContext.get()->predayModel;
}
