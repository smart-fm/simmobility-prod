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
#include "conf/ParseConfigFile.hpp"
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
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + "." + SIMMOB_VERSION_MINOR;

//Start time of program
timeval start_time;

//SIMOBILITY TEST PARAMS
const int DATA_SIZE = 30;
const std::string MODEL_LINE_FORMAT = "### %-30s : %-20s";
//options
const std::string OPTION_TESTS = "--tests";

int printReport(int simulationNumber, vector<Model*>& models, StopWatch& simulationTime)
{
    PrintOut("#################### LONG-TERM SIMULATION ####################" << endl);
    //Simulation Statistics
    PrintOut("#Simulation Number  : " << simulationNumber << endl);
    PrintOut("#Simulation Time (s): " << simulationTime.getTime() << endl);
    //Models Statistics
    PrintOut(endl);
    PrintOut("#Models:" << endl);

    for (vector<Model*>::iterator itr = models.begin(); itr != models.end(); itr++)
    {
        Model* model = *itr;
        const Model::Metadata& metadata = model->getMetadata();

        for (Model::Metadata::const_iterator itMeta = metadata.begin(); itMeta != metadata.end(); itMeta++)
        {
            boost::format fmtr = boost::format(MODEL_LINE_FORMAT);
            fmtr % itMeta->getKey() % itMeta->getValue();
            PrintOut(fmtr.str() << endl);
        }
        PrintOut(endl);
    }
    PrintOut("##############################################################" << endl);
    return 0;
}

void performMain(int simulationNumber, std::list<std::string>& resLogFiles)
{
	time_t timeInSeconds = std::time(0);
	srand(timeInSeconds);

    //Initiate configuration instance
    LT_ConfigSingleton::getInstance();
    PrintOut("Starting SimMobility, version " << SIMMOB_VERSION << endl);
    
    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

    //Simmobility Test Params
    const unsigned int tickStep = config.ltParams.tickStep;
    const unsigned int days = config.ltParams.days;
    const unsigned int workers = config.ltParams.workers;
    const bool enableHousingMarket = config.ltParams.housingModel.enabled;

    const bool enableDeveloperModel = config.ltParams.developerModel.enabled;
    const unsigned int timeIntervalDevModel = config.ltParams.developerModel.timeInterval;

    //configure time.
    config.baseGranMS() = tickStep;
    config.totalRuntimeTicks = days;
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
        WorkGroup* logsWorker = wgMgr.newWorkGroup(1, days, tickStep);
        WorkGroup* eventsWorker = wgMgr.newWorkGroup(1, days, tickStep);
        WorkGroup* hmWorkers;
        WorkGroup* devWorkers;
        DeveloperModel *developerModel = nullptr;

        if( enableHousingMarket )
        	hmWorkers = wgMgr.newWorkGroup( workers, days, tickStep);

        if( enableDeveloperModel )
        	devWorkers = wgMgr.newWorkGroup(workers, days, tickStep);
        
        //init work groups.
        wgMgr.initAllGroups();
        logsWorker->initWorkers(nullptr);
        eventsWorker->initWorkers(nullptr);

        if( enableHousingMarket )
        	hmWorkers->initWorkers(nullptr);

        if( enableDeveloperModel )
        	devWorkers->initWorkers(nullptr);
        
        //assign agents
        logsWorker->assignAWorker(&(agentsLookup.getLogger()));
        eventsWorker->assignAWorker(&(agentsLookup.getEventsInjector()));

        if( enableHousingMarket )
        	 models.push_back(new HM_Model(*hmWorkers));

        if( enableDeveloperModel )
        	 //initiate developer model; to be referred later at each time tick (day)
        	 developerModel = new DeveloperModel(*devWorkers, timeIntervalDevModel);
        	 developerModel->setDays(days);
        	 models.push_back(developerModel);

        //start all models.
        for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++)
        {
            (*it)->start();
        }
        
        //Start work groups and all threads.
        wgMgr.startAllWorkGroups();

        PrintOut("Started all workgroups." << endl);
        PrintOut("Day of Simulation: " << std::endl);

        //we add a new line break to format the output in a
        //reasonable way. 20 was found to be adequate.
        const int LINE_BREAK = 20;
        bool isParcelRemain = true;
        for (unsigned int currTick = 0; currTick < days; currTick++)
        {
            PrintOut("Day " << currTick << std::endl );
            PrintOut("The housing market has " << std::dec << (dynamic_cast<HM_Model*>(models[0]))->getMarket()->getEntrySize() << " units and \t" << (dynamic_cast<HM_Model*>(models[0]))->getNumberOfBidders() << " bidders on the market" << std::endl );

            if( currTick == 1 )
            {
				PrintOut("Lifestyle1:" << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle1HHs() <<
						 "Lifestyle2:" << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle2HHs() <<
						 "Lifestyle3:" << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle3HHs() << std::endl );
            }

            wgMgr.waitAllGroups();

            DeveloperModel::ParcelList parcels;
            if(isParcelRemain)
            {
            	parcels = developerModel->getDevelopmentCandidateParcels(false);
            }
            if(parcels.size()!=0)
            {
            	developerModel->createDeveloperAgents(parcels);
            }
            else
            {
            	isParcelRemain = false;
            }
        }
        PrintOut( endl );

        //Save our output files if we are merging them later.
        if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && config.mergeLogFiles())
        {
            resLogFiles = wgMgr.retrieveOutFileNames();
        }
        
        //stop all models.
        for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++)
        {
            (*it)->stop();
        }
    }
    
    simulationWatch.stop();
    printReport(simulationNumber, models, simulationWatch);
    //delete all models.
    while (!models.empty())
    {
        Model* modelToDelete = models.back();
        models.pop_back();
        safe_delete_item(modelToDelete);
    }
    models.clear();

    //reset singletons and stop watch.
    dataManager.reset();
    agentsLookup.reset();    
}

int main_impl(int ARGC, char* ARGV[])
{

    std::vector<std::string> args = Utils::parseArgs(ARGC, ARGV);
    if(args.size() < 2)
    {
       throw std::runtime_error("\n\nIt is necessary to provide the configuration XML file.\n\n    Example: ./SimMobility_Long ../data/simrun_LongTerm.xml\n\n");
    }
    const std::string configFileName = args[1];
    //Parse the config file (this *does not* create anything, it just reads it.).
    ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig());

    //Save a handle to the shared definition of the configuration.
    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();

    Print::Init("<stdout>");
    bool runTests = false;
    //process arguments.
    std::vector<std::string>::iterator it;
    for (it = args.begin(); it != args.end(); it++)
    {
        if (it->compare(OPTION_TESTS) == 0)
        {
            runTests = true;
            continue;
        }
    }
    
	time_t  start_clock   = time(0);
	clock_t start_process = clock();

	//get start time of the simulation.
	std::list<std::string> resLogFiles;
	const unsigned int maxIterations = config.ltParams.maxIterations;
	for (int i = 0; i < maxIterations; i++)
	{
		PrintOut("Simulation #:  " << std::dec << (i + 1) << endl);
		performMain((i + 1), resLogFiles);
	}

	ConfigManager::GetInstanceRW().reset();

	time_t end_clock = time(0);
	double time_clock = difftime( end_clock, start_clock );

	clock_t end_process = clock();
	double time_process = (double) ( end_process - start_process ) / CLOCKS_PER_SEC;

	cout << "Wall clock time passed: ";
	if( time_clock > 60 )
	{
		cout << (int)(time_clock / 60) << " minutes ";
	}

	cout << time_clock - ( (int)( time_clock / 60 ) * 60 )<< " seconds." << endl;

	cout << "CPU time used: ";
	if( time_process > 60 )
	{
		cout << (int)time_process / 60 << " minutes ";
	}

	cout << (int)( time_process - ( (int)(time_process / 60)  * 60 ) ) << " seconds" << endl;
    
    return 0;
}
