/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 9, 2013, 4:39 PM
 */

#include "PredayLogsumLuaProvider.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::lua;

namespace
{

struct ModelContext
{
	PredayLogsumLuaModel predayModel;
};

boost::thread_specific_ptr<ModelContext> threadContext;

void ensureContext()
{
	if (!threadContext.get())
	{
		try
		{
			const ModelScriptsMap& extScripts = ConfigManager::GetInstance().FullConfig().luaScriptsMap;
			const std::string& scriptsPath = extScripts.getPath();
			ModelContext* modelCtx = new ModelContext();
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("logit"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dpb"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dpt"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("dps"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmw"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmdw"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmds"));
			modelCtx->predayModel.loadFile(scriptsPath + extScripts.getScriptFileName("tmdo"));
			modelCtx->predayModel.initialize();
			threadContext.reset(modelCtx);
		} 
		catch (const std::out_of_range& oorx)
		{
			throw std::runtime_error("missing or invalid generic property 'external_scripts'");
		}
	}
}
}

const PredayLogsumLuaModel& PredayLogsumLuaProvider::getPredayModel()
{
	ensureContext();
	return threadContext.get()->predayModel;
}
