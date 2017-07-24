//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <vector>
#include <string>
#include <set>

//TODO: Replace with <chrono> or something similar.
#include <sys/time.h>

//main.cpp (top-level) files can generally get away with including GenConfig.h
#include "GenConfig.h"

#include "behavioral/PredayManager.hpp"
#include "behavioral/WithindayHelper.hpp"
#include "config/ExpandMidTermConfigFile.hpp"
#include "config/ParseMidTermConfigFile.hpp"
#include "conf/ParseConfigFile.hpp"
#include "database/pt_network_dao/PT_NetworkSqlDao.hpp"
#include "entities/AuraManager.hpp"
#include "entities/BusController.hpp"
#include "entities/TrainController.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/TrainStationAgent.hpp"
#include "entities/ClosedLoopRunManager.hpp"
#include "entities/MT_PersonLoader.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/DriverVariants.hpp"
#include "entities/roles/driver/TaxiDriver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/roles/waitTaxiActivity/WaitTaxiActivity.hpp"
#include "entities/ScreenLineCounter.hpp"
#include "entities/TaxiStandAgent.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/streetdir/A_StarPublicTransitShortestPathImpl.hpp"
#include "logging/ControllerLog.hpp"
#include "partitions/PartitionManager.hpp"
#include "path/PathSetManager.hpp"
#include "path/PT_PathSetManager.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
#include "util/Utils.hpp"
#include "workers/WorkGroupManager.hpp"
#include "behavioral/ServiceController.hpp"
#include "behavioral/TrainServiceControllerLuaProvider.hpp"
#include "entities/TrainRemoval.hpp"


//If you want to force a header file to compile, you can put it here temporarily:
//#include "entities/BusController.hpp"

//Note: This must be the LAST include, so that other header files don't have
//      access to cout if output is disabled.
#include <entities/FleetController_MT.hpp>

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;
using namespace sim_mob::medium;

//Start time of program
timeval start_time_med;

//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;

void assignConfluxLoaderToWorker(WorkGroup* workGrp, unsigned int workerIdx)
{
	const sim_mob::MutexStrategy& mtxStrat = ConfigManager::GetInstance().FullConfig().mutexStategy();
	Conflux* conflux = new Conflux(nullptr, mtxStrat, -1, true);
	if(workGrp->assignWorker(conflux, workerIdx))
	{
		conflux->setParentWorkerAssigned();
		workGrp->registerLoaderEntity(conflux);
	}
	else
	{
		throw std::runtime_error("worker assignment failed for conflux loader");
	}
}

bool assignConfluxToWorkerRecursive(WorkGroup* workGrp, Conflux* conflux, unsigned int workerIdx, int numConfluxesToAddInWorker)
{
	typedef std::set<Conflux*> ConfluxSet;
	ConfluxSet& confluxes = MT_Config::getInstance().getConfluxes();
	bool workerFilled = false;

	if(numConfluxesToAddInWorker > 0)
	{
		if (workGrp->assignWorker(conflux, workerIdx))
		{
			confluxes.erase(conflux);
			numConfluxesToAddInWorker--;
			conflux->setParentWorkerAssigned();
		}

		ConfluxSet& connectedConfluxes = conflux->getConnectedConfluxes();

		// assign the confluxes of the downstream MultiNodes to the same worker if possible
		for(ConfluxSet::iterator i = connectedConfluxes.begin();
				i != connectedConfluxes.end() && numConfluxesToAddInWorker > 0 && !confluxes.empty();
				i++)
		{
			Conflux* connConflux = *i;
			if(!(*i)->hasParentWorker()) {
				// insert this conflux if it has not already been assigned to another worker
				if (workGrp->assignWorker(connConflux, workerIdx))
				{
					// One conflux was added by the insert. So...
					confluxes.erase(connConflux);
					numConfluxesToAddInWorker--;
					connConflux->setParentWorkerAssigned(); // set the worker pointer in the Conflux
				}
			}
		}

		// after inserting all confluxes of the downstream segments
		if(numConfluxesToAddInWorker > 0 && !confluxes.empty())
		{
			// call this function recursively with whichever conflux is at the beginning of the confluxes set
			workerFilled = assignConfluxToWorkerRecursive(workGrp, (*confluxes.begin()), workerIdx, numConfluxesToAddInWorker);
		}
		else
		{
			workerFilled = true;
		}
	}
	return workerFilled;
}

/**
 * adds each conflux to the managedEntities list of workers.
 * This function attempts to assign all adjacent confluxes to the same worker.
 *
 * Future work:
 * If this assignment performs badly, we might want to think of a heuristics based optimization algorithm
 * which improves this assignment. We can indeed model this problem as a graph partitioning problem. Each
 * worker is a partition; the confluxes can be modeled as the nodes of the graph; and the edges will represent
 * the flow of vehicles between confluxes. Our objective is to minimize the (expected) flow of agents from one
 * partition to the other. We can try to fit the Kernighan-Lin algorithm or Fiduccia-Mattheyses algorithm
 * for partitioning, if it works. This is a little more complex due to the variable flow rates of vehicles
 * (edge weights); might require more thinking.
 *
 * @param workGrp the work group containing workers which must take confluxes
 */
void assignConfluxToWorkers(WorkGroup* workGrp)
{
	//Using confluxes by reference as we remove items as and when we assign them to a worker
	std::set<Conflux*>& confluxes = MT_Config::getInstance().getConfluxes();
	size_t numWorkers = workGrp->size();
	int numConfluxesPerWorker = (int)(confluxes.size() / numWorkers);

	for(unsigned wrkrIdx=0; wrkrIdx<numWorkers; wrkrIdx++)
	{
		if(numConfluxesPerWorker > 0)
		{
			assignConfluxToWorkerRecursive(workGrp, (*confluxes.begin()), wrkrIdx, numConfluxesPerWorker);
		}
		assignConfluxLoaderToWorker(workGrp, wrkrIdx);
	}
	if(!confluxes.empty())
	{
		//There can be up to (workers.size() - 1) confluxes for which the parent
		//worker is not yet assigned. We distribute these confluxes to the workers in round robin fashion
		unsigned wrkrIdx=0;
		for(std::set<Conflux*>::iterator cfxIt=confluxes.begin(); cfxIt!=confluxes.end(); cfxIt++)
		{
			Conflux* cfx = *cfxIt;
			if (workGrp->assignWorker(cfx, wrkrIdx))
			{
				cfx->setParentWorkerAssigned();
			}
			wrkrIdx = (wrkrIdx+1)%numWorkers;
		}
		confluxes.clear();
	}
}

/**
 * assign train station agent to conflux
 */
void assignStationAgentToConfluxes()
{
	std::map<std::string, TrainStop*>&  MRTStopMap = PT_NetworkCreater::getInstance().MRTStopsMap;
	std::map<std::string, TrainStop*>::iterator trainStopIt;
	for(trainStopIt = MRTStopMap.begin();trainStopIt!=MRTStopMap.end();trainStopIt++)
	{
		TrainStationAgent* stationAgent = new TrainStationAgent();
		TrainController<Person_MT>::registerStationAgent(trainStopIt->first, stationAgent);
		TrainController<sim_mob::medium::Person_MT> *trainController=TrainController<sim_mob::medium::Person_MT>::getInstance();
		Station *station=trainController->getStationFromId(trainStopIt->first);
		stationAgent->setStationName(trainStopIt->first);
		if(station)
		{
			stationAgent->setStation(station);
			stationAgent->setLines();
		}
		const Node* node = trainStopIt->second->getRandomStationSegment()->getParentLink()->getFromNode();
		ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
		MT_Config& mtCfg = MT_Config::getInstance();
		std::map<const Node*, Conflux*>& nodeConfluxesMap = mtCfg.getConfluxNodes();
		std::map<const Node*, Conflux*>::iterator it = nodeConfluxesMap.find(node);
		if(it!=nodeConfluxesMap.end())
		{
			it->second->addStationAgent(stationAgent);
			stationAgent->setConflux(it->second);
		}
	}
}

/**
 * Main simulation loop for the supply simulator
 * @param configFileName name of the input config xml file
 * @param resLogFiles name of the output log file
 * @return true if the function finishes execution normally
 */
bool performMainSupply(const std::string& configFileName, std::list<std::string>& resLogFiles)
{
	ProfileBuilder* prof = nullptr;
	if (ConfigManager::GetInstance().CMakeConfig().ProfileOn())
	{
		ProfileBuilder::InitLogFile("profile_trace.txt");
		prof = new ProfileBuilder();
	}

	sim_mob::DailyTime::initAllTimes();

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Register our Role types.
	//NOTE: Accessing ConfigParams before loading it is technically safe, but we
	//      should really be clear about when this is not okay.
	const MutexStrategy& mtx = ConfigManager::GetInstance().FullConfig().mutexStategy();
	
	//Create an instance of role factory
	RoleFactory<Person_MT>* rf = new RoleFactory<Person_MT>();
	RoleFactory<Person_MT>::setInstance(rf);
	
	rf->registerRole("driver", new sim_mob::medium::Driver(nullptr));
	rf->registerRole("activityRole", new sim_mob::ActivityPerformer<Person_MT>(nullptr));
	rf->registerRole("busdriver", new sim_mob::medium::BusDriver(nullptr, mtx));
	rf->registerRole("taxidriver", new sim_mob::medium::TaxiDriver(nullptr, mtx));
	rf->registerRole("waitBusActivity", new sim_mob::medium::WaitBusActivity(nullptr));
	rf->registerRole("waitTrainActivity", new sim_mob::medium::WaitTrainActivity(nullptr));
	rf->registerRole("pedestrian", new sim_mob::medium::Pedestrian(nullptr));
	rf->registerRole("waitTaxiActivity", new sim_mob::medium::WaitTaxiActivity(nullptr));
	rf->registerRole("passenger", new sim_mob::medium::Passenger(nullptr));
	rf->registerRole("biker", new sim_mob::medium::Biker(nullptr));
	rf->registerRole("trainDriver", new sim_mob::medium::TrainDriver(nullptr));
	rf->registerRole("truckerLGV", new sim_mob::medium::TruckerLGV(nullptr));
	rf->registerRole("truckerHGV", new sim_mob::medium::TruckerHGV(nullptr));



	//Load our user config file, which is a time costly function
	ExpandMidTermConfigFile expand(MT_Config::getInstance(), ConfigManager::GetInstanceRW().FullConfig(), Agent::all_agents);

	//insert bus stop agent to segmentStats;
	std::set<SegmentStats*>& segmentStatsWithStops = MT_Config::getInstance().getSegmentStatsWithBusStops();
	std::set<SegmentStats*>::iterator itSegStats;
	std::vector<const sim_mob::BusStop*>::iterator itBusStop;
	StreetDirectory& strDirectory= StreetDirectory::Instance();
	for (itSegStats = segmentStatsWithStops.begin(); itSegStats != segmentStatsWithStops.end(); itSegStats++)
	{
		SegmentStats* stats = *itSegStats;
		std::vector<const sim_mob::BusStop*>& busStops = stats->getBusStops();
		for (itBusStop = busStops.begin(); itBusStop != busStops.end(); itBusStop++)
		{
			const sim_mob::BusStop* stop = *itBusStop;
			sim_mob::medium::BusStopAgent* busStopAgent = new sim_mob::medium::BusStopAgent(mtx, -1, stop, stats);
			stats->addBusStopAgent(busStopAgent);
			BusStopAgent::registerBusStopAgent(busStopAgent);
		}
	}
	auto& segmentStatsWithStands = MT_Config::getInstance().getSegmentStatsWithTaxiStands();
	for (auto i = segmentStatsWithStands.begin(); i != segmentStatsWithStands.end(); i++) {
		auto& taxiStands = (*i)->getTaxiStand();
		for (auto iStand = taxiStands.begin(); iStand != taxiStands.end(); iStand++) {
			sim_mob::medium::TaxiStandAgent* taxiStandAgent = new sim_mob::medium::TaxiStandAgent(mtx, -1, *iStand);
			(*i)->addTaxiStandAgent(taxiStandAgent);
			TaxiStandAgent::registerTaxiStandAgent(taxiStandAgent);
		}
	}
	//Save handles to definition of configurations.
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const MT_Config& mtConfig = MT_Config::getInstance();

	//aa{ I just run this to retrieve the TAZes. Unfortunately, at the moment only preday reads them
	PredayManager predayManager;
	predayManager.loadZoneNodes();
	//aa}

	PeriodicPersonLoader* periodicPersonLoader = new MT_PersonLoader(Agent::all_agents, Agent::pending_agents);
	const ScreenLineCounter* screenLnCtr = ScreenLineCounter::getInstance(); //This line is necessary. It creates the singleton ScreenlineCounter object before any workers are created.
	WithindayModelsHelper::loadZones(); //load zone information from db

	{ //Begin scope: WorkGroups
	WorkGroupManager wgMgr;
	wgMgr.setSingleThreadMode(false);

	//Work Group specifications
	//Mid-term is not using Aura Manager at the moment. Therefore setting it to nullptr
	WorkGroup* personWorkers = wgMgr.newWorkGroup(mtConfig.personWorkGroupSize(), config.totalRuntimeTicks, mtConfig.granPersonTicks,
			nullptr /*AuraManager is not used in mid-term*/, nullptr/*partition manager is not used in mid-term*/, periodicPersonLoader);

	//Initialize all work groups (this creates barriers, and locks down creation of new groups).
	wgMgr.initAllGroups();

	messaging::MessageBus::RegisterHandler(PT_Statistics::getInstance());

	//Load persons for 0th tick
	periodicPersonLoader->loadPersonDemand();

	//Initialize each work group individually
	personWorkers->initWorkers(&entLoader);

	//distribute confluxes among workers
	assignConfluxToWorkers(personWorkers);

	//distribute station agents among confluxes
	assignStationAgentToConfluxes();

	//Anything in all_agents is starting on time 0, and should be added now.
	for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++)
	{
		personWorkers->loadPerson((*it));
	}
	Agent::all_agents.clear();

	if (BusController::HasBusController())
	{
		personWorkers->assignAWorker(BusController::GetInstance());
	}
	if(TrainController<Person_MT>::HasTrainController())
	{
		personWorkers->assignAWorker(TrainController<Person_MT>::getInstance());
	}
	if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		personWorkers->assignAWorker(FleetController_MT::getInstance());
		auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();

		for (auto it = controllers.begin(); it != controllers.end(); it++)
		{
			personWorkers->assignAWorker(it->second);
		}
	}
	//incident
	personWorkers->assignAWorker(IncidentManager::getInstance());

	if(config.simulation.closedLoop.enabled)
	{
		const ClosedLoopParams &params = config.simulation.closedLoop;
		ClosedLoopRunManager::initialise(params.guidanceFile, params.tollFile, params.incentivesFile);
	}

	//before starting the groups, initialize the time interval for one of the pathset manager's helpers
	PathSetManager::initTimeInterval();

	Print() << "\nSimulating...\n";

	//Start work groups and all threads.
	wgMgr.startAllWorkGroups();

	/////////////////////////////////////////////////////////////////
	// NOTE: WorkGroups are able to handle skipping steps by themselves.
	//       So, we simply call "wait()" on every tick, and on non-divisible
	//       time ticks, the WorkGroups will return without performing
	//       a barrier sync.
	/////////////////////////////////////////////////////////////////
	timeval loop_start_time;
	gettimeofday(&loop_start_time, nullptr);
	int loop_start_offset = ProfileBuilder::diff_ms(loop_start_time, start_time_med);

	StateSwitcher<int> numTicksShown(0); //Only goes up to 10
	StateSwitcher<int> lastTickPercent(0); //So we have some idea how much time is left.
	bool firstTick = true;

	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++)
	{
		const DailyTime dailyTime=ConfigManager::GetInstance().FullConfig().simStartTime()+DailyTime(currTick*5000);

		//Output. We show every 10% change. (just to give some indication of progress)
		int currTickPercent = (currTick * 100) / config.totalRuntimeTicks;
		currTickPercent /= 10; //Only update 10%, 20%, etc.

		//Determine whether to print this time tick or not.
		bool printTick = lastTickPercent.update(currTickPercent);

		//Note that OutputEnabled also affects locking.
		if (printTick)
		{
			std::stringstream msg;
			msg << currTickPercent << "0%" << std::endl;
			PrintOut(msg.str());
			ControllerLog() << "Simulation done: " << msg.str();
		}

		//Agent-based cycle, steps 1,2,3,4
		{
			std::set<Entity*> removedEntities;

			//Call each function in turn.
			//NOTE: Each sub-function tests the current state.
			if (firstTick && ConfigManager::GetInstance().FullConfig().RunningMidTerm())
			{
				TrainServiceControllerLuaProvider::getTrainControllerModel()->useServiceController(dailyTime.getStrRepr());
				//first tick has two frameTickBarr
				wgMgr.waitForFrameTickBar();
				firstTick = false;
			}

			wgMgr.waitAllGroups_FrameTick();
			wgMgr.waitAllGroups_FlipBuffers(&removedEntities);
			//removing the trains from the simulation which are to be removed after the finish of frame tick barrier and flip buffer barrier for thread safety
			TrainRemoval *trainRemovalInstance=TrainRemoval::getInstance();
			trainRemovalInstance->removeTrainsBeforeNextFrameTick();
			TrainServiceControllerLuaProvider::getTrainControllerModel()->useServiceController((dailyTime+DailyTime(5000)).getStrRepr());
			wgMgr.waitAllGroups_DistributeMessages(removedEntities);
			wgMgr.waitAllGroups_MacroTimeTick();

			//Delete all collected entities:
			while (!removedEntities.empty())
			{
				Entity* ag = *removedEntities.begin();
				removedEntities.erase(removedEntities.begin());
				delete ag;
			}
		}

		unsigned long currTimeMS = currTick * config.baseGranMS();

		//Check if we are running in closed loop with DynaMIT
		if(config.simulation.closedLoop.enabled && (currTimeMS + config.baseGranMS()) % (config.simulation.closedLoop.sensorStepSize * 1000) == 0)
		{
			SurveillanceStation::writeSurveillanceOutput(config, currTimeMS + config.baseGranMS());
			ClosedLoopRunManager::waitForDynaMIT(config);
		}
	}

	timeval loop_end_time;
	gettimeofday(&loop_end_time, nullptr);
	int loop_time = (int) ProfileBuilder::diff_ms(loop_end_time, loop_start_time);
	Print() << "100%\n\nTime required to execute the simulation: "
	        << DailyTime((uint32_t) loop_time).getStrRepr() << std::endl;

	BusStopAgent::removeAllBusStopAgents();
	sim_mob::PathSetParam::resetInstance();

	//finalize
	TravelTimeManager::getInstance()->storeCurrentSimulationTT();

	Print() << "Time required for initialisation [Loading configuration, network, demand ...]: "
	        << DailyTime((uint32_t) loop_start_offset).getStrRepr() << std::endl;

	Print() << "\nNumber of trips [demand] loaded: " << config.numTripsLoaded;

	if(config.numTripsNotLoaded > 0)
	{
		Print() << "\nNumber of trips [demand] that failed to load [Refer to warn.log for more details]: "
				<< config.numTripsNotLoaded;
	}

	Print() << "\nNumber of trips [demand] completed: " << config.numTripsCompleted;
	Print() << "\n\nNumber of persons loaded: " << config.numPersonsLoaded << endl;

	if(config.numPathNotFound > 0)
	{
		Print() << "Persons not simulated as the path was not found [Refer to warn.log for more details]: "
				<< config.numPathNotFound << endl;
	}

	if (config.numAgentsKilled > 0)
	{
		Print() << "Agents removed from simulation due to errors [Refer to warn.log for more details]: "
				<< config.numAgentsKilled << endl;
	}

	if (!Agent::pending_agents.empty())
	{
		Print() << "\nWARNING! There are still " << Agent::pending_agents.size()
		        << " agents waiting to be scheduled. Next start time is: "
		        << DailyTime(Agent::pending_agents.top()->getStartTime()).getStrRepr() << "\n";
	}

	(Conflux::activeAgentsLock).lock();
	if (Agent::activeAgents.size() > 0)
	{
		size_t numPersons = 0;
		size_t numActivities = 0, numBiker = 0, numCarPassenger = 0, numDriver = 0, numPassenger = 0;
		size_t numPedestrian = 0, numPrivBusPassenger = 0, numTaxiPassenger = 0, numTrainPassenger = 0;
		size_t numTravelPedestrian = 0, numWaitBus = 0, numWaitTaxi = 0, numWaitTrain = 0;

		for (std::vector<Entity*>::iterator it = Agent::activeAgents.begin(); it != Agent::activeAgents.end(); ++it)
		{
			Person_MT *person = dynamic_cast<Person_MT *> (*it);

			if (person)
			{
				Role<Person_MT> *role = person->getRole();

				if (role)
				{
					numPersons++;
					switch(role->roleType)
					{
					case Role<Person_MT>::RL_ACTIVITY:
						numActivities++;
						break;
					case Role<Person_MT>::RL_BIKER:
						numBiker++;
						break;
					case Role<Person_MT>::RL_BUSDRIVER:
						numPersons--;
						break;
					case Role<Person_MT>::RL_CARPASSENGER:
						numCarPassenger++;
						break;
					case Role<Person_MT>::RL_DRIVER:
						numDriver++;
						break;
					case Role<Person_MT>::RL_PASSENGER:
						numPassenger++;
						break;
					case Role<Person_MT>::RL_PEDESTRIAN:
						numPedestrian++;
						break;
					case Role<Person_MT>::RL_PRIVATEBUSPASSENGER:
						numPrivBusPassenger++;
						break;
					case Role<Person_MT>::RL_TAXIPASSENGER:
						numTaxiPassenger++;
						break;
					case Role<Person_MT>::RL_TRAINDRIVER:
						numPersons--;
						break;
					case Role<Person_MT>::RL_TRAINPASSENGER:
						numTrainPassenger++;
						break;
					case Role<Person_MT>::RL_TRAVELPEDESTRIAN:
						numTravelPedestrian++;
						break;
					case Role<Person_MT>::RL_TRUCKER_HGV:
						numPersons--;
						break;
					case Role<Person_MT>::RL_TRUCKER_LGV:
						numPersons--;
						break;
					case Role<Person_MT>::RL_WAITBUSACTIVITY:
						numWaitBus++;
						break;
					case Role<Person_MT>::RL_WAITTAXIACTIVITY:
						numWaitTaxi++;
						break;
					case Role<Person_MT>::RL_WAITTRAINACTIVITY:
						numWaitTrain++;
						break;
					case Role<Person_MT>::RL_TAXIDRIVER:
						numPersons--;
						break;

					}
				}
			}
		}

		Print() << "\nPersons still in the simulation: " << numPersons << "\n"
		        << numActivities << " Performing activity,\n" << numBiker << " Bikers,\n"
		        << numCarPassenger << " Car passengers,\n"
		        << numDriver << " Drivers,\n" << numPassenger << " Passengers,\n" << numPedestrian << " Pedestrians,\n"
		        << numPrivBusPassenger << " Private bus passengers,\n" << numTaxiPassenger << " Taxi passengers,\n"
				<< numTrainPassenger << " Train passengers,\n"
		        << numTravelPedestrian << " Travel pedestrians,\n" << numWaitBus << " Waiting for bus,\n"
				<< numWaitTaxi << " Waiting for Taxi,\n" << numWaitTrain << " Waiting for train\n";
	}
	(Conflux::activeAgentsLock).unlock();

	PT_Statistics::getInstance()->storeStatistics();
	PT_Statistics::resetInstance();

	//Save our output files if we are merging them later.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()
			&& ConfigManager::GetInstance().FullConfig().isMergeLogFiles())
	{
		resLogFiles = wgMgr.retrieveOutFileNames();
	}

	}  //End scope: WorkGroups.

    //Save screen line counts
    if(mtConfig.screenLineParams.outputEnabled)
    {
        screenLnCtr->exportScreenLineCount();
    }

	//At this point, it should be possible to delete all Signals and Agents.
    clear_delete_vector(Agent::all_agents);
	while(!Agent::pending_agents.empty())
	{
		Entity* topAg = Agent::pending_agents.top();
		Agent::pending_agents.pop();
		safe_delete_item(topAg);
	}

	int boardCount=sim_mob::medium::TrainDriver::boardPassengerCount;
	const std::string& fileName("BoardingCount.csv");
	sim_mob::BasicLogger& ptMRTMoveLogger  = sim_mob::Logger::log(fileName);
	ptMRTMoveLogger<<boardCount<<endl;

	safe_delete_item(periodicPersonLoader);
	cout << "\nSimulation complete. Closing worker threads...\n" << endl;

	//Delete our profile pointer (if it exists)
	safe_delete_item(prof);
	return true;
}

/**
 * The preday demand simulator
 */
bool performMainDemand()
{
	const MT_Config& mtConfig = MT_Config::getInstance();
	PredayManager predayManager;
	predayManager.loadZones();
	predayManager.loadCosts();
	predayManager.loadPersonIds();
	predayManager.loadUnavailableODs();
	if(mtConfig.runningPredaySimulation() && mtConfig.isFileOutputEnabled())
	{
		predayManager.loadZoneNodes();
		predayManager.loadPostcodeNodeMapping();
		predayManager.removeInvalidAddresses();
	}

	if(mtConfig.runningPredayCalibration())
	{
		Print() << "Preday mode: calibration" << std::endl;
		predayManager.calibratePreday();
	}
	else
	{
		Print() << "Preday mode: " << (mtConfig.runningPredaySimulation()? "simulation":"logsum computation")  << std::endl;
		predayManager.dispatchLT_Persons();
//		const db::BackendType populationSource = mtConfig.getPopulationSource();
//		if(populationSource == db::POSTGRES)
//		{
//			predayManager.dispatchLT_Persons();
//		}
//		else
//		{
//			predayManager.dispatchMongodbPersons();
//		}
	}
	return true;
}

/**
 * Main simulation loop.
 * \note
 * For doxygen, we are setting the variable JAVADOC AUTOBRIEF to "true"
 * This isn't necessary for class-level documentation, but if we want
 * documentation for a short method (like "get" or "set") then it makes sense to
 * have a few lines containing brief/full comments. (See the manual's description
 * of JAVADOC AUTOBRIEF). Of course, we can discuss this first.
 *
 * \par
 * See Buffered.hpp for an example of this in action.
 *
 * \par
 * ~Seth
 *
 * This function is separate from main() to allow for easy scoping of WorkGroup objects.
 */
bool performMainMed(const std::string& configFileName, const std::string& mtConfigFileName, std::list<std::string>& resLogFiles)
{
	std::srand(clock()); // set random seed for RNGs
	cout <<"Starting SimMobility, version " << SIMMOB_VERSION << endl;
	cout << "\nLoading the configuration files: " << configFileName << ", " << mtConfigFileName << endl;

	//Parse the config file (this *does not* create anything, it just reads it.).
	ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig());

	//load configuration file for mid-term
	ParseMidTermConfigFile parseMT_Cfg(mtConfigFileName, MT_Config::getInstance(), ConfigManager::GetInstanceRW().FullConfig());

	//Enable or disable logging (all together, for now).
	//NOTE: This may seem like an odd place to put this, but it makes sense in context.
	//      OutputEnabled is always set to the correct value, regardless of whether ConfigParams()
	//      has been loaded or not. The new Config class makes this much clearer.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
	{
		//Log::Init("out.txt");
		Warn::Init("warn.log");
		Print::Init("<stdout>");
		ControllerLog::Init("controller.log");
	}
	else
	{
		//Log::Ignore();
		Warn::Ignore();
		Print::Ignore();
		ControllerLog::Ignore();
	}

    if (MT_Config::getInstance().RunningMidSupply())
	{
		Print() << "Mid-term run mode: supply\n" << endl;
		return performMainSupply(configFileName, resLogFiles);
	}
    else if (MT_Config::getInstance().RunningMidDemand())
	{
		Print() << "Mid-term run mode: preday\n" << endl;
		return performMainDemand();
	}
	else
	{
		throw std::runtime_error("Invalid Mid-term run mode. Admissible values are \"demand\" and \"supply\"");
	}
}

int main_impl(int ARGC, char* ARGV[])
{
	std::vector<std::string> args = Utils::parseArgs(ARGC, ARGV);

	//Save start time
	gettimeofday(&start_time_med, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 */
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	config.using_MPI = false;
#ifndef SIMMOB_DISABLE_MPI
	if (args.size() > 2 && args[2]=="mpi")
	{
		config.using_MPI = true;
	}
#endif

	/**
	 * set random be repeatable
	 */
	config.is_simulation_repeatable = true;

	/**
	 * set run mode as mid-term
	 */
	config.simMobRunMode = ConfigParams::MID_TERM;

	/**
	 * Start MPI if using_MPI is true
	 */
#ifndef SIMMOB_DISABLE_MPI
	if (config.using_MPI)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		std::string mpi_result = partitionImpl.startMPIEnvironment(ARGC, ARGV);
		if (mpi_result.compare("") != 0)
		{
			Warn() << "MPI Error:" << mpi_result << endl;
			exit(1);
		}
	}
#endif

	//Argument 1: Config file
	//Note: Don't change this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/simulation.xml";
	std::string mtConfigFileName = "data/simrun_MidTerm.xml";

	if (args.size() > 2)
	{
		configFileName = args[1];
		mtConfigFileName = args[2];
	}
	else
	{
		Print() << "One or both config file not specified; using defaults: " << configFileName << " and " << mtConfigFileName << endl;
	}

	//This should be moved later, but we'll likely need to manage random numbers
	//ourselves anyway, to make simulations as repeatable as possible.
	//if (config.is_simulation_repeatable)
	//{
		//TODO: Output the random seed here (and only here)
	//}

	//Perform main loop
	timeval simStartTime;
	gettimeofday(&simStartTime, nullptr);

	std::list<std::string> resLogFiles;
	int returnVal = performMainMed(configFileName, mtConfigFileName, resLogFiles) ? 0 : 1;

	//Concatenate output files?
	if (!resLogFiles.empty())
	{
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outSimInfoFileName);
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outTrainNetworkFilename);
		Utils::printAndDeleteLogFiles(resLogFiles);
	}
	int retVal = std::system("rm out_0_*.txt out.network.txt");

	return returnVal;
}
