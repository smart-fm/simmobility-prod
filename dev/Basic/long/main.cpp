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

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"

#include "model/HM_Model.hpp"
#include "Common.hpp"
#include "config/LT_Config.hpp"

using namespace sim_mob::db;

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
const int WORKERS = 1;
const int DATA_SIZE = 30;


int printReport(int simulationNumber, vector<Model*>& models, StopWatch& simulationTime) {
    PrintOut("#################### LONG-TERM SIMULATION ####################" << endl);
    //Simulation Statistics
    PrintOut("#Simulation Number  :" << simulationNumber << endl);
    PrintOut("#Simulation Time (s):" << simulationTime.getTime() << endl);
    //Models Statistics
    PrintOut(endl);
    PrintOut("#Models:" << endl);
    for (vector<Model*>::iterator itr = models.begin(); itr != models.end(); itr++) {
        Model* model = *itr;
        PrintOut("### Model Name    : " << model->getName() << endl);
        PrintOut("### Start Time (s): " << model->getStartTime()<< endl);
        PrintOut("###  Stop Time (s): " << model->getStopTime() << endl);
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
    vector<Model*> models; 
    HM_Model* model = nullptr;
    {
        simulationWatch.start();   
        WorkGroupManager wgMgr;
        wgMgr.setSingleThreadMode(config.singleThreaded());

        //Work Group specifications
        WorkGroup* agentWorkers = wgMgr.newWorkGroup(WORKERS, DAYS, TICK_STEP);
        wgMgr.initAllGroups();
        agentWorkers->initWorkers(nullptr);
        DatabaseConfig dbConfig(LT_DB_CONFIG_FILE);

        //models 
        model = new HM_Model(dbConfig, *agentWorkers);
        models.push_back(model);
        model->start();
       
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
        model->stop();
        simulationWatch.stop();
    }
   
    printReport(simulationNumber, models, simulationWatch);
    //delete all models.
    while (!models.empty()) {
        Model* modelToDelete = models.back();
        models.pop_back();
        safe_delete_item(modelToDelete);
    }
    models.clear();
    
    
}

int main(int ARGC, char* ARGV[]) {
    std::vector<std::string> args = Utils::ParseArgs(ARGC, ARGV);
    Print::Init("<stdout>");
    //get start time of the simulation.
    std::list<std::string> resLogFiles;
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        PrintOut("Simulation #:  " << (i + 1) << endl);
        performMain((i+1), resLogFiles);
    }
 
    //Concatenate output files?
    if (!resLogFiles.empty()) {
        resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
        Utils::PrintAndDeleteLogFiles(resLogFiles);
    }
    ConfigManager::GetInstanceRW().reset();
    return 0;
}