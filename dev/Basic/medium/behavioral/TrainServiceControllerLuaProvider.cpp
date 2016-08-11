//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include <behavioral/TrainServiceControllerLuaProvider.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "config/MT_Config.hpp"

using namespace sim_mob;
using namespace sim_mob::lua;
using namespace sim_mob::medium;

namespace
{
    struct RailServiceControllerModelContext
    {
        ServiceController *traincontrollerModel;
        RailServiceControllerModelContext()
        {
        	traincontrollerModel=ServiceController::getInstance();
        }
    };

    boost::thread_specific_ptr<RailServiceControllerModelContext> threadContext;

    void ensureContext()
    {
        if (!threadContext.get())
        {
        	try
        	{
        		const ModelScriptsMap& extScripts = MT_Config::getInstance().getModelScriptsMap();
        		const std::string& scriptsPath = extScripts.getPath();
        		RailServiceControllerModelContext* modelCtx = new RailServiceControllerModelContext();
        		modelCtx->traincontrollerModel->loadFile(scriptsPath + extScripts.getScriptFileName("serv"));
        		modelCtx->traincontrollerModel->initialize();
        		threadContext.reset(modelCtx);
        	}
        	catch (const std::out_of_range& oorx)
        	{
        		throw std::runtime_error("missing service controller script");
        	}
        }
    }
}

ServiceController* TrainServiceControllerLuaProvider::getTrainControllerModel()
{
    ensureContext();
    return threadContext.get()->traincontrollerModel;
}
