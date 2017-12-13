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
			const std::map<std::string, std::string>& predayScriptsName = extScripts.getScriptsFileNameMap();
			ModelContext* modelCtx = new ModelContext();
			for (const auto& item : predayScriptsName)
			{
				modelCtx->predayModel.loadFile(scriptsPath + item.second);
			}
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
