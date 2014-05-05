//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * file main.cpp
 * Empty file for the (future) long-term simulation
 * \author Pedro Gandola
 */

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <unistd.h>

#include "GenConfig.h"
//#include "tinyxml.h"
#include <boost/format.hpp>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"

#include "model/HM_Model.hpp"
#include "Common.hpp"
#include "config/LT_Config.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"

#include "model/DeveloperModel.hpp"


using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::list;
using std::pair;
using std::map;
using namespace sim_mob;
using namespace sim_mob::long_term;

//Current software version.
const string SIMMOB_VERSION = string(
        SIMMOB_VERSION_MAJOR) + "." + SIMMOB_VERSION_MINOR;

//Start time of program
timeval start_time;

//SIMOBILITY TEST PARAMS
const int MAX_ITERATIONS = 1;
const int TICK_STEP = 1;
const int DAYS = 365;
const int WORKERS = 8;
const int DATA_SIZE = 30;
const std::string MODEL_LINE_FORMAT = "### %-30s : %-20s";
//options
const std::string OPTION_TESTS = "--tests";

int printReport(int simulationNumber, vector<Model*>& models, StopWatch& simulationTime) {
    PrintOut("#################### LONG-TERM SIMULATION ####################" << endl);
    //Simulation Statistics
    PrintOut("#Simulation Number  : " << simulationNumber << endl);
    PrintOut("#Simulation Time (s): " << simulationTime.getTime() << endl);
    //Models Statistics
    PrintOut(endl);
    PrintOut("#Models:" << endl);
    for (vector<Model*>::iterator itr = models.begin(); itr != models.end(); itr++) {
        Model* model = *itr;
        const Model::Metadata& metadata = model->getMetadata();
        for (Model::Metadata::const_iterator itMeta = metadata.begin();
                itMeta != metadata.end(); itMeta++) {
            boost::format fmtr = boost::format(MODEL_LINE_FORMAT);
            fmtr % itMeta->getKey() % itMeta->getValue();
            PrintOut(fmtr.str() << endl);
        }
        PrintOut(endl);
    }
    PrintOut("##############################################################" << endl);
    return 0;
}

void performMain(int simulationNumber, std::list<std::string>& resLogFiles) {
    //Initiate configuration instance
    LT_ConfigSingleton::getInstance();
    PrintOut("Starting SimMobility, version " << SIMMOB_VERSION << endl);
    
    //configure time.
    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
    config.baseGranMS() = TICK_STEP;
    config.totalRuntimeTicks = DAYS;
    config.defaultWrkGrpAssignment() = WorkGroup::ASSIGN_ROUNDROBIN;
    config.singleThreaded() = false;
   
    //simulation time.
    StopWatch simulationWatch;
    
    //Starts clock.
    simulationWatch.start();
    
    //Loads data and initialize singletons.
    DataManager& dataManager = DataManagerSingleton::getInstance();
    AgentsLookup& agentsLookup = AgentsLookupSingleton::getInstance();
    //loads all necessary data
    dataManager.load();
    
    vector<Model*> models;
    {
        WorkGroupManager wgMgr;
        wgMgr.setSingleThreadMode(config.singleThreaded());
        
        // -- Events injector work group.
        WorkGroup* logsWorker = wgMgr.newWorkGroup(1, DAYS, TICK_STEP);
        WorkGroup* eventsWorker = wgMgr.newWorkGroup(1, DAYS, TICK_STEP);
        WorkGroup* hmWorkers = wgMgr.newWorkGroup(WORKERS, DAYS, TICK_STEP);
        WorkGroup* devWorkers = wgMgr.newWorkGroup(1, DAYS, TICK_STEP);
        
        //init work groups.
        wgMgr.initAllGroups();
        logsWorker->initWorkers(nullptr);
        hmWorkers->initWorkers(nullptr);
        eventsWorker->initWorkers(nullptr);
        devWorkers->initWorkers(nullptr);
        
        //assign agents
        logsWorker->assignAWorker(&(agentsLookup.getLogger()));
        eventsWorker->assignAWorker(&(agentsLookup.getEventsInjector()));
        //models 
        models.push_back(new HM_Model(*hmWorkers));
        models.push_back(new DeveloperModel(*devWorkers));
        //start all models.
        for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++) {
            (*it)->start();
        }
        
        //Start work groups and all threads.
        wgMgr.startAllWorkGroups();

        PrintOut("Started all workgroups." << endl);
        for (unsigned int currTick = 0; currTick < DAYS; currTick++) {
            PrintOut("Day: " << currTick << endl);
            wgMgr.waitAllGroups();
        }
        //Save our output files if we are merging them later.
        if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && 
            config.mergeLogFiles()) {
            resLogFiles = wgMgr.retrieveOutFileNames();
        }
        
        //stop all models.
        for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++) {
            (*it)->stop();
        }
    }
    
    simulationWatch.stop();
    printReport(simulationNumber, models, simulationWatch);
    //delete all models.
    while (!models.empty()) {
        Model* modelToDelete = models.back();
        models.pop_back();
        safe_delete_item(modelToDelete);
    }
    models.clear();

    //reset singletons and stop watch.
    dataManager.reset();
    agentsLookup.reset();    
}

int main(int ARGC, char* ARGV[]) {
    std::vector<std::string> args = Utils::parseArgs(ARGC, ARGV);
    Print::Init("<stdout>");
    bool runTests = false;
    //process arguments.
    std::vector<std::string>::iterator it;
    for (it = args.begin(); it != args.end(); it++){
        if (it->compare(OPTION_TESTS) == 0){
            runTests = true;
            continue;
        }
    }
    
    if (!runTests) {
        //get start time of the simulation.
        std::list<std::string> resLogFiles;
        for (int i = 0; i < MAX_ITERATIONS; i++) {
            PrintOut("Simulation #:  " << (i + 1) << endl);
            performMain((i + 1), resLogFiles);
        }

        //Concatenate output files?
        if (!resLogFiles.empty()) {
            resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
            Utils::printAndDeleteLogFiles(resLogFiles);
        }
        ConfigManager::GetInstanceRW().reset();
    } else {
/*        unit_tests::DaoTests tests;
        tests.testAll();*/
    }
    
    return 0;
}
