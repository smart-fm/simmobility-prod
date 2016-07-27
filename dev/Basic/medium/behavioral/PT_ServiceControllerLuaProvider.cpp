/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PT_ServiceController.cpp
 * Author: Jabir <jabir@smart.mit.edu>
 * 
 */

#include "PT_ServiceControllerLuaProvider.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "config/MT_Config.hpp"

using namespace sim_mob;
using namespace sim_mob::lua;
using namespace sim_mob::medium;

namespace
{
    struct ServiceControllerModelContext
    {
        ServiceController *ptrcModel;
        ServiceControllerModelContext()
        {
        	ptrcModel=ServiceController::getInstance();
        }
    };

    boost::thread_specific_ptr<ServiceControllerModelContext> threadContext;

    void ensureContext()
    {
        if (!threadContext.get())
        {
        	try
        	{
        		const ModelScriptsMap& extScripts = MT_Config::getInstance().getServiceControllerScriptsMap();
        		const std::string& scriptsPath = extScripts.getPath();
        		ServiceControllerModelContext* modelCtx = new ServiceControllerModelContext();
        		modelCtx->ptrcModel->loadFile(scriptsPath + extScripts.getScriptFileName("serv"));
        		modelCtx->ptrcModel->initialize();
        		threadContext.reset(modelCtx);
        	}
        	catch (const std::out_of_range& oorx)
        	{
        		throw std::runtime_error("missing service controller script");
        	}
        }
    }
}

ServiceController* PT_ServiceControllerLuaProvider::getPTRC_Model()
{
    ensureContext();
    return threadContext.get()->ptrcModel;
}
