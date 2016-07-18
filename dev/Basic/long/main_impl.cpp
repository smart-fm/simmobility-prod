//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * main.cpp
 * Author: Pedro Gandola
 * 		   Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
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
#include "database/dao/SimulationStartPointDao.hpp"
#include "database/dao/SimulationStoppedPointDao.hpp"
#include "database/entity/SimulationStartPoint.hpp"
#include "database/dao/CreateOutputSchemaDao.hpp"
#include "database/dao/BuildingDao.hpp"
#include "database/dao/ParcelDao.hpp"
#include "database/dao/UnitDao.hpp"
#include "database/dao/ProjectDao.hpp"
#include "database/dao/BidDao.hpp"
#include "database/dao/UnitSaleDao.hpp"
#include "database/dao/DevelopmentPlanDao.hpp"
#include "database/dao/VehicleOwnershipChangesDao.hpp"
#include "database/dao/HouseholdDao.hpp"
#include "database/dao/HouseholdUnitDao.hpp"
#include "util/HelperFunctions.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::list;
using std::pair;
using std::map;
using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::db;

//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + "." + SIMMOB_VERSION_MINOR;

//Start time of program
timeval start_time;

//SIMOBILITY TEST PARAMS
const int DATA_SIZE = 30;
const std::string MODEL_LINE_FORMAT = "### %-30s : %-20s";

int printReport(int simulationNumber, vector<Model*>& models, StopWatch& simulationTime)
{
    PrintOutV("#################### LONG-TERM SIMULATION ####################" << endl);
    //Simulation Statistics
    PrintOutV("#Simulation Number  : " << simulationNumber << endl);
    PrintOutV("#Simulation Time (s): " << simulationTime.getTime() << endl);
    //Models Statistics
    PrintOut(endl);
    PrintOutV("#Models:" << endl);

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
    PrintOutV("##############################################################" << endl);
    return 0;
}

void createOutputSchema(db::DB_Connection& conn,const std::string& currentOutputSchema)
{
	if(conn.isConnected())
	{
		CreateOutputSchemaDao createOPDao(conn);
		std::string createSchemaquery = "CREATE SCHEMA " + currentOutputSchema +";";
		createOPDao.executeQuery(createSchemaquery);
		std::string setSearchPathQuery = "SET search_path to " + currentOutputSchema;
		createOPDao.executeQuery(setSearchPathQuery);


		std::vector<CreateOutputSchema*> createOPSchemaList;
		loadData<CreateOutputSchemaDao>(conn,createOPSchemaList);
		std::sort(createOPSchemaList.begin(), createOPSchemaList.end(), CreateOutputSchema::OrderById());
		std::vector<CreateOutputSchema*>::iterator opSchemaTablesItr;
		for(opSchemaTablesItr = createOPSchemaList.begin(); opSchemaTablesItr != createOPSchemaList.end(); ++opSchemaTablesItr)
		{
			std::string createTableQuery = (*opSchemaTablesItr)->getQuery();
			std::string query = "CREATE TABLE " + currentOutputSchema +"."+ createTableQuery +";";
			createOPDao.executeQuery(query);
		}
	}

}

void loadDataToOutputSchema(db::DB_Connection& conn,std::string &currentOutputSchema,BigSerial simVersionId,int simStoppedTick,DeveloperModel &developerModel,HM_Model &housingMarketModel)
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	bool resume = config.ltParams.resume;
	unsigned int year = config.ltParams.year;
	std::string scenario = config.ltParams.simulationScenario;
	std::string mainSchemaVersion = config.ltParams.mainSchemaVersion;
	std::string cfgSchemaVersion = config.ltParams.configSchemaVersion;
	std::string calibrationSchemaVersion = config.ltParams.calibrationSchemaVersion;
	std::string geometrySchemaVersion = config.ltParams.geometrySchemaVersion;
	std::string simScenario = boost::lexical_cast<std::string>(scenario)+"_"+boost::lexical_cast<std::string>(year);
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	boost::shared_ptr<SimulationStartPoint>simStartPointObj(new SimulationStartPoint(simVersionId,simScenario,*timeinfo,mainSchemaVersion,cfgSchemaVersion,calibrationSchemaVersion,geometrySchemaVersion,simStoppedTick));

	if(conn.isConnected())
	{
		SimulationStartPointDao simStartPointDao(conn);
		simStartPointDao.insertSimulationStartPoint(*simStartPointObj.get(),currentOutputSchema);

		std::vector<boost::shared_ptr<Building> > buildings = developerModel.getBuildingsVec();
		std::vector<boost::shared_ptr<Building> >::iterator buildingsItr;
		BuildingDao buildingDao(conn);
		for(buildingsItr = buildings.begin(); buildingsItr != buildings.end(); ++buildingsItr)
		{
			buildingDao.insertBuilding(*(*buildingsItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<Parcel> > parcels = developerModel.getProfitableParcelsVec();
		std::vector<boost::shared_ptr<Parcel> >::iterator parcelsItr;
		ParcelDao parcelDao(conn);
		for(parcelsItr = parcels.begin(); parcelsItr != parcels.end(); ++parcelsItr)
		{
			parcelDao.insertParcel(*(*parcelsItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<Unit> > units = developerModel.getUnitsVec();
		std::vector<boost::shared_ptr<Unit> >::iterator unitsItr;
		UnitDao unitDao(conn);
		for(unitsItr = units.begin(); unitsItr != units.end(); ++unitsItr)
		{
			unitDao.insertUnit(*(*unitsItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<Project> > projects = developerModel.getProjectsVec();
		std::vector<boost::shared_ptr<Project> >::iterator projectsItr;
		ProjectDao projectDao(conn);
		for(projectsItr = projects.begin(); projectsItr != projects.end(); ++projectsItr)
		{
			projectDao.insertProject(*(*projectsItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<Bid> > bids = housingMarketModel.getNewBids();
		std::vector<boost::shared_ptr<Bid> >::iterator bidsItr;
		BidDao bidDao(conn);
		for(bidsItr = bids.begin(); bidsItr != bids.end(); ++bidsItr)
		{
			bidDao.insertBid(*(*bidsItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<UnitSale> > unitSales = housingMarketModel.getUnitSales();
		std::vector<boost::shared_ptr<UnitSale> >::iterator unitSalesItr;
		UnitSaleDao unitSaleDao(conn);
		for(unitSalesItr = unitSales.begin(); unitSalesItr != unitSales.end(); ++unitSalesItr)
		{
			unitSaleDao.insertUnitSale(*(*unitSalesItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<DevelopmentPlan> > devPlans = developerModel.getDevelopmentPlansVec();
		std::vector<boost::shared_ptr<DevelopmentPlan> >::iterator devPlansItr;
		DevelopmentPlanDao devPlanDao(conn);
		for(devPlansItr = devPlans.begin(); devPlansItr != devPlans.end(); ++devPlansItr)
		{
			devPlanDao.insertDevelopmentPlan(*(*devPlansItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<VehicleOwnershipChanges> > vehicleOwnershipChanges = housingMarketModel.getVehicleOwnershipChanges();
		std::vector<boost::shared_ptr<VehicleOwnershipChanges> >::iterator vehOwnChangeItr;
		VehicleOwnershipChangesDao vehOwnChangeDao(conn);
		for(vehOwnChangeItr = vehicleOwnershipChanges.begin(); vehOwnChangeItr != vehicleOwnershipChanges.end(); ++vehOwnChangeItr)
		{
			vehOwnChangeDao.insertVehicleOwnershipChanges(*(*vehOwnChangeItr),currentOutputSchema);
		}

		std::vector<boost::shared_ptr<Household> > householdsWithBids = housingMarketModel.getHouseholdsWithBids();
		std::vector<boost::shared_ptr<Household> >::iterator hhItr;
		HouseholdDao hhDao(conn);
		for(hhItr = householdsWithBids.begin(); hhItr != householdsWithBids.end(); ++hhItr)
		{
			hhDao.insertHousehold(*(*hhItr),currentOutputSchema);
		}

		HM_Model::HouseholdList *households = housingMarketModel.getHouseholdList();
		HM_Model::HouseholdList::iterator houseHoldItr;
		for(houseHoldItr = households->begin(); houseHoldItr != households->end(); ++houseHoldItr)
		{

			if(housingMarketModel.getHouseholdWithBidsById((*houseHoldItr)->getId()) == nullptr)
			{
				if(((*houseHoldItr)->getIsBidder()) || ((*houseHoldItr)->getIsSeller()))
				{
					hhDao.insertHousehold(*(*houseHoldItr),currentOutputSchema);
				}

			}
		}

		std::vector<boost::shared_ptr<HouseholdUnit> > hhUnits = housingMarketModel.getNewHouseholdUnits();
		HouseholdUnitDao hhUnitDao(conn);
		for (boost::shared_ptr<HouseholdUnit> hhUnit :hhUnits)
		{
			hhUnitDao.insertHouseholdUnit(*hhUnit,currentOutputSchema);
		}
		SimulationStoppedPointDao simStoppedPointDao(conn);
		simStoppedPointDao.insertSimulationStoppedPoints(*(developerModel.getSimStoppedPointObj(simVersionId)).get(),currentOutputSchema);
//		developerModel.getBuildingsVec().clear();
	}
}

void performMain(int simulationNumber, std::list<std::string>& resLogFiles)
{
	time_t timeInSeconds = std::time(0);
	srand(timeInSeconds);

    //Initiate configuration instance
    LT_ConfigSingleton::getInstance();
    PrintOutV( "Starting SimMobility, version " << SIMMOB_VERSION << endl);

    ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

    //Simmobility Test Params
    const unsigned int tickStep = config.ltParams.tickStep;
    const unsigned int days = config.ltParams.days;
    const unsigned int workers = config.ltParams.workers;

    const unsigned int timeIntervalDevModel = config.ltParams.developerModel.timeInterval;
    unsigned int opSchemaloadingInterval = config.ltParams.opSchemaloadingInterval;

    int lastStoppedDay = 0;
    int simStoppedTick = 0;

    //configure time.
    config.baseGranMS() = tickStep;
    config.totalRuntimeTicks = days;
    config.defaultWrkGrpAssignment() = WorkGroup::ASSIGN_ROUNDROBIN;

    //simulation time.
    StopWatch simulationWatch;
    
    //Starts clock.
    simulationWatch.start();
    
    //Loads data and initialize singletons.
    DataManager& dataManager = DataManagerSingleton::getInstance();
    AgentsLookup& agentsLookup = AgentsLookupSingleton::getInstance();
    //loads all necessary data
    dataManager.load();


    std::vector<SimulationStartPoint*> simulationStartPointList;
    DB_Config dbConfig(LT_DB_CONFIG_FILE);
    dbConfig.load();

    // Connect to database.
    DB_Connection conn(sim_mob::db::POSTGRES, dbConfig);
    conn.connect();
    SimulationStartPointDao simStartPointDao(conn);
    bool resume = config.ltParams.resume;
    std::string currentOutputSchema;

    unsigned int year = config.ltParams.year;
    std::string scenario = config.ltParams.simulationScenario;
    std::string simScenario = boost::lexical_cast<std::string>(scenario)+"_"+boost::lexical_cast<std::string>(year);
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    BigSerial simVersionId = 1;

    if(resume)
    {
    	currentOutputSchema = config.ltParams.currentOutputSchema;
    	if(conn.isConnected())
    	{
    		simulationStartPointList = simStartPointDao.getAllSimulationStartPoints(currentOutputSchema);
    		if(!simulationStartPointList.empty())
    		{
    			simVersionId = simulationStartPointList[simulationStartPointList.size()-1]->getId() + 1;
    			lastStoppedDay = simulationStartPointList[simulationStartPointList.size()-1]->getSimStoppedTick() + 1;
    		}
    	}
    }
    else
    {
    	char buffer[80];
    	strftime(buffer,80,"%Y%m%d%I%M%S",timeinfo);
    	std::string dateTimeStr(buffer);
    	currentOutputSchema = simScenario +"_"+ dateTimeStr;

    }


    vector<Model*> models;
    {
        WorkGroupManager wgMgr;
        wgMgr.setSingleThreadMode(false);
        
        // -- Events injector work group.
        WorkGroup* logsWorker = wgMgr.newWorkGroup(1, days, tickStep, nullptr, nullptr, nullptr, (uint32_t)lastStoppedDay );
        WorkGroup* eventsWorker = wgMgr.newWorkGroup(1, days, tickStep,nullptr,nullptr,nullptr, (uint32_t)lastStoppedDay );
        WorkGroup* hmWorkers;
        WorkGroup* devWorkers;
        DeveloperModel *developerModel = nullptr;
        HM_Model *housingMarketModel = nullptr;


        hmWorkers = wgMgr.newWorkGroup( workers, days, tickStep ,nullptr,nullptr,nullptr, (uint32_t)lastStoppedDay );

        devWorkers = wgMgr.newWorkGroup(workers, days, tickStep,nullptr,nullptr,nullptr, (uint32_t)lastStoppedDay );
        
        //init work groups.
        wgMgr.initAllGroups();
        logsWorker->initWorkers(nullptr);
        eventsWorker->initWorkers(nullptr);

       	hmWorkers->initWorkers(nullptr);
       	devWorkers->initWorkers(nullptr);
        
        //assign agents
        logsWorker->assignAWorker(&(agentsLookup.getLogger()));
        eventsWorker->assignAWorker(&(agentsLookup.getEventsInjector()));

        unsigned int currentTick = 0;

        //set the currentTick to the last stopped date if it is a restart run
        if (resume )
        {
        	currentTick = lastStoppedDay;
        }

        {
        	 housingMarketModel = new HM_Model(*hmWorkers);//initializing the housing market model
             housingMarketModel->setStartDay(currentTick);
             housingMarketModel->setLastStoppedDay(lastStoppedDay);
        	 models.push_back(housingMarketModel);
        	 agentsLookup.getEventsInjector().setModel(housingMarketModel);
        }

        {
        	 //initiate developer model; to be referred later at each time tick (day)
        	 developerModel = new DeveloperModel(*devWorkers, timeIntervalDevModel);
        	 developerModel->setHousingMarketModel(housingMarketModel);
        	 developerModel->setStartDay(currentTick);
        	 developerModel->setOpSchemaloadingInterval(opSchemaloadingInterval);
        	 models.push_back(developerModel);
        }

        housingMarketModel->setDeveloperModel(developerModel);

        //start all models.
        for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++)
        {
            (*it)->start();
        }


        PrintOutV("XML Config Settings: " << endl);
        PrintOutV("XML Config lt params enabled " << config.ltParams.enabled << endl);
        PrintOutV("XML Config calibrationSchemaVersion " << config.ltParams.calibrationSchemaVersion << endl);
        PrintOutV("XML Config configSchemaVersion  " << config.ltParams.configSchemaVersion << endl);
       	PrintOutV("XML Config currentOutputSchema " << config.ltParams.currentOutputSchema << endl);
       	PrintOutV("XML Config days " << config.ltParams.days << endl);
        PrintOutV("XML Config DeveloperModel " << config.ltParams.developerModel.enabled << endl);
        PrintOutV("XML Config DeveloperModel initialBuildingId " << config.ltParams.developerModel.initialBuildingId << endl);
        PrintOutV("XML Config DeveloperModel initialPostcode " << config.ltParams.developerModel.initialPostcode << endl);
        PrintOutV("XML Config DeveloperModel initialProjectId " << config.ltParams.developerModel.initialProjectId << endl);
        PrintOutV("XML Config DeveloperModel initialUnitId " << config.ltParams.developerModel.initialUnitId << endl);
        PrintOutV("XML Config DeveloperModel minLotSize " << config.ltParams.developerModel.minLotSize << endl);
        PrintOutV("XML Config DeveloperModel timeInterval " << config.ltParams.developerModel.timeInterval << endl);
        PrintOutV("XML Config geometrySchemaVersion " << config.ltParams.geometrySchemaVersion << endl);
        PrintOutV("XML Config HousingModel bidderBTOUnitsChoiceSet " << config.ltParams.housingModel.bidderBTOUnitsChoiceSet << endl);
        PrintOutV("XML Config HousingModel bidderUnitsChoiceSet " << config.ltParams.housingModel.bidderUnitsChoiceSet << endl);
        PrintOutV("XML Config HousingModel dailyHouseholdAwakenings " << config.ltParams.housingModel.dailyHouseholdAwakenings << endl);
        PrintOutV("XML Config HousingModel enabled " << config.ltParams.housingModel.enabled << endl);
        PrintOutV("XML Config HousingModel householdBiddingWindow " << config.ltParams.housingModel.householdBiddingWindow << endl);
        PrintOutV("XML Config HousingModel housingMarketSearchPercentage " << config.ltParams.housingModel.housingMarketSearchPercentage << endl);
        PrintOutV("XML Config HousingModel housingMoveInDaysInterval " << config.ltParams.housingModel.housingMoveInDaysInterval << endl);
        PrintOutV("XML Config HousingModel initialHouseholdsOnMarket " << config.ltParams.housingModel.initialHouseholdsOnMarket << endl);
        PrintOutV("XML Config HousingModel offsetBetweenUnitBuyingAndSelling " << config.ltParams.housingModel.offsetBetweenUnitBuyingAndSelling << endl);
        PrintOutV("XML Config HousingModel timeInterval " << config.ltParams.housingModel.timeInterval << endl);
        PrintOutV("XML Config HousingModel timeOffMarket " << config.ltParams.housingModel.timeOffMarket << endl);
        PrintOutV("XML Config HousingModel timeOnMarket " << config.ltParams.housingModel.timeOnMarket << endl);
        PrintOutV("XML Config HousingModel vacantUnitActivationProbability " << config.ltParams.housingModel.vacantUnitActivationProbability << endl);
        PrintOutV("XML Config HousingModel householdAwakeningPercentageByBTO " << config.ltParams.housingModel.householdAwakeningPercentageByBTO << endl);
        PrintOutV("XML Config mainSchemaVersion " << config.ltParams.mainSchemaVersion << endl);
        PrintOutV("XML Config maxIterations " << config.ltParams.maxIterations << endl);
        PrintOutV("XML Config opSchemaloadingInterval " << config.ltParams.opSchemaloadingInterval << endl);
        PrintOutV("XML Config outputHouseholdLogsums " << config.ltParams.outputHouseholdLogsums.enabled << endl);
        PrintOutV("XML Config outputHouseholdLogsums fixedHomeVariableWork " << config.ltParams.outputHouseholdLogsums.fixedHomeVariableWork << endl);
        PrintOutV("XML Config outputHouseholdLogsums fixedWorkVariableHome " << config.ltParams.outputHouseholdLogsums.fixedWorkVariableHome << endl);
        PrintOutV("XML Config Simulation resumption " << config.ltParams.resume << endl);
        PrintOutV("XML Config schoolAssignmentModel enabled " << config.ltParams.schoolAssignmentModel.enabled << endl);
        PrintOutV("XML Config schoolAssignmentModel schoolChangeWaitingTimeInDays " << config.ltParams.schoolAssignmentModel.schoolChangeWaitingTimeInDays << endl);
        PrintOutV("XML Config simulationScenario " << config.ltParams.simulationScenario << endl);
        PrintOutV("XML Config tickStep " << config.ltParams.tickStep << endl);
        PrintOutV("XML Config vehicleOwnershipModel " << config.ltParams.vehicleOwnershipModel.enabled << endl);
        PrintOutV("XML Config vehicleOwnershipModel vehicleBuyingWaitingTimeInDays " << config.ltParams.vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays << endl);
        PrintOutV("XML Config workers " << config.ltParams.workers << endl);
        PrintOutV("XML Config year " << config.ltParams.year << endl);


        //Start work groups and all threads.
        wgMgr.startAllWorkGroups();

        PrintOutV("Started all workgroups." << endl);
        PrintOutV("Day of Simulation: " << std::endl);

        for (unsigned int currTick = currentTick; currTick < days; currTick++)
        {
        	simStoppedTick = currTick;
        	if(((currTick+1) == opSchemaloadingInterval) && (!resume))
        	{
        		createOutputSchema(conn,currentOutputSchema);
        	}
        	if((currTick > 0) && ((currTick+1)%opSchemaloadingInterval == 0))
        	{
        		loadDataToOutputSchema(conn,currentOutputSchema,simVersionId,simStoppedTick,*developerModel,*housingMarketModel);
        	}
            if( currTick == 0 )
            {
				PrintOutV(" Lifestyle1: " << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle1HHs() <<
						  " Lifestyle2: " << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle2HHs() <<
						  " Lifestyle3: " << (dynamic_cast<HM_Model*>(models[0]))->getLifestyle3HHs() << std::endl );
            }

			#ifdef VERBOSE_POSTCODE

            HM_Model::HouseholdList *householdList = (dynamic_cast<HM_Model*>(models[0]))->getHouseholdList();

			for( int n = 0; n < householdList->size(); n++)
			{
				const Unit *localUnit = (dynamic_cast<HM_Model*>(models[0]))->getUnitById( (*householdList)[n]->getUnitId());
				Postcode *postcode = (dynamic_cast<HM_Model*>(models[0]))->getPostcodeById(localUnit->getSlaAddressId());

				//PrintOut( currTick << "," << (*householdList)[n]->getId() << ","  <<  postcode->getSlaPostcode() << "," << postcode->getLongitude() << "," <<  postcode->getLatitude() << std::endl );

				const std::string LOG_HHPC = "%1%, %2%, %3%, %4%, %5%";
		        boost::format fmtr = boost::format(LOG_HHPC) % currTick
		        											 % (*householdList)[n]->getId()
															 % postcode->getSlaPostcode()
															 % postcode->getLongitude()
															 % postcode->getLatitude();

		        AgentsLookupSingleton::getInstance().getLogger().log(LoggerAgent::HH_PC, fmtr.str());
			 }
			#endif

			//start all models.
		    for (vector<Model*>::iterator it = models.begin(); it != models.end(); it++)
		    {
		 	   (*it)->update(currTick);
		    }

            wgMgr.waitAllGroups();

            DeveloperModel::ParcelList parcels;
            DeveloperModel::DeveloperList developerAgents;
            developerAgents = developerModel->getDeveloperAgents();
            developerModel->wakeUpDeveloperAgents(developerAgents);

            sleep(1);

            PrintOutV("Day " << currTick << " HUnits: " << std::dec << (dynamic_cast<HM_Model*>(models[0]))->getMarket()->getEntrySize()
				   << " Bidders: " 	<< (dynamic_cast<HM_Model*>(models[0]))->getNumberOfBidders() << " "
				   << " Sellers: " 	<< (dynamic_cast<HM_Model*>(models[0]))->getNumberOfSellers() << " "		
				   << " Bids: " 	<< (dynamic_cast<HM_Model*>(models[0]))->getBids()
				   << " Accepted: " << (dynamic_cast<HM_Model*>(models[0]))->getSuccessfulBids()
				   << " Exits: " 	<< (dynamic_cast<HM_Model*>(models[0]))->getExits()
				   << " Awaken: "	<< (dynamic_cast<HM_Model*>(models[0]))->getAwakeningCounter()
				   << " " << std::endl );

            (dynamic_cast<HM_Model*>(models[0]))->setNumberOfBidders(0);
            (dynamic_cast<HM_Model*>(models[0]))->setNumberOfSellers(0);

            (dynamic_cast<HM_Model*>(models[0]))->resetBAEStatistics();
        }

        //Save our output files if we are merging them later.
        if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && config.isMergeLogFiles())
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
    bool longTerm = true;
    ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig(), longTerm );

    //Save a handle to the shared definition of the configuration.
    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();

    Print::Init("<stdout>");
	time_t  start_clock   = time(0);
	clock_t start_process = clock();

	//get start time of the simulation.
	std::list<std::string> resLogFiles;
	const unsigned int maxIterations = config.ltParams.maxIterations;
	for (int i = 0; i < maxIterations; i++)
	{
		PrintOutV("Simulation #:  " << std::dec << (i + 1) << endl);
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
