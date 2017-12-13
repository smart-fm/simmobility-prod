//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayLuaProvider.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "config/MT_Config.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using namespace sim_mob::lua;

namespace
{

struct ModelContext
{
	PredayLuaModel predayModel;
};

boost::thread_specific_ptr<ModelContext> threadContext;

void ensureContext()
{
	if (!threadContext.get())
	{
		try
		{
			const ModelScriptsMap& extScripts = ConfigManager::GetInstance().FullConfig().predayLuaScriptsMap;
			const std::string& scriptsPath = extScripts.getPath();
            const std::map<std::string, std::string>& predayScriptsName = extScripts.getScriptsFileNameMap();
			ModelContext* modelCtx = new ModelContext();
            for (const auto& item : predayScriptsName)
            {
                modelCtx->predayModel.loadFile(scriptsPath + item.second);
            }
			modelCtx->predayModel.initialize();
			threadContext.reset(modelCtx);
		} catch (const std::out_of_range& oorx)
		{
			throw std::runtime_error("missing or invalid generic property 'external_scripts'");
		}
	}
}
}

const PredayLuaModel& PredayLuaProvider::getPredayModel()
{
	ensureContext();
	return threadContext.get()->predayModel;
}
