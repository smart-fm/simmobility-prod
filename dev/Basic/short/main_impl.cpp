//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * \file main_impl.cpp
 * A first approximation of the basic pseudo-code in C++. The main file loads several
 * properties from data/config.xml, and attempts a simulation run. Currently, the various
 * granularities and pedestrian starting locations are loaded.
 *
 * \note
 * This file contains everything except a main method, which is contained within main.cpp.
 * The function main_impl() has the exact same signature as main(), and serves the exact same purpose.
 * See the note in main.cpp if you have any questions.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 */

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

//main.cpp (top-level) files can generally get away with including GenConfig.h
#include "GenConfig.h"

#include "buffering/BufferedDataManager.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/Locked.hpp"
#include "buffering/Shared.hpp"
#include "conf/ConfigManager.hpp"
#include "config/ExpandShortTermConfigFile.hpp"
#include "config/ParseShortTermConfigFile.hpp"
#include "conf/ParseConfigFile.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/ClosedLoopRunManager.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "entities/IntersectionManager.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/driverCommunication/DriverComm.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/fmodController/FMOD_Controller.hpp"
#include "network/CommunicationManager.hpp"
#include "network/ControlManager.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "partitions/ShortTermBoundaryProcessor.hpp"
#include "util/StateSwitcher.hpp"
#include "util/Utils.hpp"
#include "workers/WorkGroupManager.hpp"


//Note: This must be the LAST include, so that other header files don't have
//      access to cout if SIMMOB_DISABLE_OUTPUT is true.

using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;

//Start time of program
timeval start_time;


//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;

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
bool performMain(const std::string& configFileName, const std::string& shortConfigFile, std::list<std::string>& resLogFiles, const std::string& XML_OutPutFileName)
{
	Print() <<"Starting SimMobility, version " << SIMMOB_VERSION << endl;
	DailyTime::initAllTimes();

    ST_Config& stCfg = ST_Config::getInstance();

	//Parse the config file (this *does not* create anything, it just reads it.).
    ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig());

    ParseShortTermConfigFile stParse(shortConfigFile, ConfigManager::GetInstanceRW().FullConfig(), stCfg);
	
	//Enable or disable logging (all together, for now).
	//NOTE: This may seem like an odd place to put this, but it makes sense in context.
	//      OutputEnabled is always set to the correct value, regardless of whether ConfigParams()
	//      has been loaded or not. The new Config class makes this much clearer.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) 
	{
		Warn::Init("warn.log");
		Print::Init("<stdout>");		
	}
	else 
	{
		Warn::Ignore();
		Print::Ignore();		
	}

	ProfileBuilder* prof = nullptr;
	
	if (ConfigManager::GetInstance().CMakeConfig().ProfileOn()) 
	{
		ProfileBuilder::InitLogFile("profile_trace.txt");
		prof = new ProfileBuilder();
	}
	
    //Save a handle to the shared definition of the configuration.
    const ConfigParams &config = ConfigManager::GetInstance().FullConfig();
    const MutexStrategy &mtx = config.mutexStategy();

	//Create an instance of role factory
	RoleFactory<Person_ST> *rf = new RoleFactory<Person_ST>;
	RoleFactory<Person_ST>::setInstance(rf);

	//Register our Role types.
    if (stCfg.commSimEnabled())
	{
		rf->registerRole("driver", new DriverComm(nullptr, mtx));
	}
	else 
	{
		rf->registerRole("driver", new Driver(nullptr, mtx));
	}

	rf->registerRole("pedestrian", new Pedestrian(nullptr));
	rf->registerRole("passenger",new Passenger(nullptr));
	rf->registerRole("busdriver", new BusDriver(nullptr, mtx));
	rf->registerRole("activityRole", new ActivityPerformer<Person_ST>(nullptr));
	rf->registerRole("waitBusActivity", new WaitBusActivity(nullptr));
	rf->registerRole("taxidriver", new Driver(nullptr, mtx));

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Load the configuration file
	Print() << "\nLoading the configuration files: " << configFileName << ", " << shortConfigFile << "..." << std::endl;
	ExpandShortTermConfigFile expand(stCfg, ConfigManager::GetInstanceRW().FullConfig(), Agent::all_agents, Agent::pending_agents);

    if (config.PathSetMode())
	{
		//Initialise path-set manager
		PrivateTrafficRouteChoice* pvtRtChoice = PrivateTrafficRouteChoice::getInstance();
		std::string name = configFileName;
		pvtRtChoice->setScenarioName(name);
	}

	//Initialise the control manager and wait for an IDLE state (interactive mode only).
	ControlManager* ctrlMgr = nullptr;
	
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) 
	{
		Print() << "Scenario loaded...\nSimulation state is IDLE"<<std::endl;
		
		ctrlMgr = ConfigManager::GetInstance().FullConfig().getControlMgr();
		ctrlMgr->setSimState(IDLE);
		
		while(ctrlMgr->getSimState() == IDLE) 
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
	}

	//Start boundaries
	if (!config.MPI_Disabled() && config.using_MPI) 
	{
		PartitionManager::instance().initBoundaryTrafficItems();
	}

	//bool NoDynamicDispatch = config.DynamicDispatchDisabled();

	PartitionManager* partMgr = nullptr;
	
	if (!config.MPI_Disabled() && config.using_MPI) 
	{
		partMgr = &PartitionManager::instance();
	}

	{ 
	//Begin scope: WorkGroups
	//TODO: WorkGroup scope currently does nothing. We need to re-enable WorkGroup deletion at some later point. ~Seth
	WorkGroupManager wgMgr;
	wgMgr.setSingleThreadMode(false);

	//Work Group specifications
	WorkGroup* personWorkers = wgMgr.newWorkGroup(stCfg.personWorkGroupSize(), config.totalRuntimeTicks, stCfg.granPersonTicks, &AuraManager::instance(), partMgr);
	WorkGroup* signalStatusWorkers = wgMgr.newWorkGroup(stCfg.signalWorkGroupSize(), config.totalRuntimeTicks, stCfg.granSignalsTicks);
	WorkGroup* intMgrWorkers = wgMgr.newWorkGroup(stCfg.intMgrWorkGroupSize(), config.totalRuntimeTicks, stCfg.granIntMgrTicks);

	//TODO: Ideally, the Broker would go on the agent Work Group. However, the Broker often has to wait for all Agents to finish.
	//      If an Agent is "behind" the Broker, we have two options:
	//        1) Have some way of specifying that the Broker agent goes "last" (Agent priority?)
	//        2) Have some way of telling the parent Worker to "delay" this Agent (e.g., add it to a temporary list) from *within* update.
	WorkGroup* communicationWorkers = wgMgr.newWorkGroup(stCfg.commWorkGroupSize(), config.totalRuntimeTicks, stCfg.granCommunicationTicks);

	//Initialise the aura manager
	AuraManager::instance().init(stCfg.aura_manager_impl());

	//Initialise all work groups (this creates barriers, and locks down creation of new groups).
	wgMgr.initAllGroups();
	
	messaging::MessageBus::RegisterHandler(PT_Statistics::getInstance());

	//Initialise each work group individually
	personWorkers->initWorkers(&entLoader);
	signalStatusWorkers->initWorkers(nullptr);
	intMgrWorkers->initWorkers(nullptr);
	communicationWorkers->initWorkers(nullptr);

	//If communication simulator is enabled, start the Broker.
    if(stCfg.commSimEnabled())
	{
		//NOTE: I am fairly sure that MtxStrat_Locked is the wrong mutex strategy. However, Broker doesn't
		//      register any buffered properties (except x/y, which Agent registers), and it never updates these.
		//      I am changing this back to buffered; if this runs smoothly for a while, then you can remove this comment. ~Seth
		//NOTE: I am also changing the Agent ID; manually specifying it as 0 is dangerous.
		Broker *broker =  new Broker(MtxStrat_Buffered);
		Broker::SetSingleBroker(broker);
		communicationWorkers->assignAWorker(broker);
	}

	//Create and register bus stop agents
	const std::map<unsigned int, BusStop *> &busStops = RoadNetwork::getInstance()->getMapOfIdvsBusStops();
	
	for(std::map<unsigned int, BusStop *>::const_iterator itStops = busStops.begin(); itStops != busStops.end(); ++itStops)
	{
		BusStopAgent *busStopAgent = new BusStopAgent(mtx, -1, itStops->second);
		BusStopAgent::registerBusStopAgent(busStopAgent, *personWorkers);
	}

	//Assign all signals to the signals worker
	const std::map<unsigned int, Signal *> &signals = Signal::getMapOfIdVsSignals();
	
	for (std::map<unsigned int, Signal *>::const_iterator it = signals.begin(); it != signals.end(); ++it)
	{
		Signal_SCATS *signalScats = dynamic_cast<Signal_SCATS *>(it->second);
		
		//Create and initialise loop detectors
		LoopDetectorEntity *loopDetectorEntity = new LoopDetectorEntity(mtx);
		loopDetectorEntity->init(*signalScats);
		Agent::all_agents.insert((Entity *&&) loopDetectorEntity);
		signalScats->setLoopDetector(loopDetectorEntity);
		signalStatusWorkers->assignAWorker(it->second);
	}
	
	//Anything in all_agents is starting on time 0, and should be added now.
	for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); ++it) 
	{
		personWorkers->assignAWorker(*it);
	}
	
	//Assign all intersection managers to the signal worker
	const map<unsigned int, IntersectionManager *> &intManagers = IntersectionManager::getIntManagers();
	for (map<unsigned int, IntersectionManager *>::const_iterator it = intManagers.begin(); it != intManagers.end(); ++it)
	{
		intMgrWorkers->assignAWorker(it->second);
	}

	if (stCfg.amod.enabled && amod::AMODController::instanceExists())
	{
		personWorkers->assignAWorker(amod::AMODController::getInstance());
	}

	if (FMOD::FMOD_Controller::instanceExists())
	{
		personWorkers->assignAWorker(FMOD::FMOD_Controller::instance());
	}

	if(config.simulation.closedLoop.enabled)
	{
		const ClosedLoopParams &params = config.simulation.closedLoop;
		ClosedLoopRunManager::initialise(params.guidanceFile, params.tollFile, params.incentivesFile);
	}
	
	Print() << "Simulating...\n";

	//Start work groups and all threads.
	wgMgr.startAllWorkGroups();

	if (!config.MPI_Disabled() && config.using_MPI) 
	if (!config.MPI_Disabled() && config.using_MPI) 
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(personWorkers, signalStatusWorkers);

		Print() << "partition_solution_id in main function:" << partitionImpl.partition_config->partition_solution_id << std::endl;
	}

	/////////////////////////////////////////////////////////////////
	// NOTE: WorkGroups are able to handle skipping steps by themselves.
	//       So, we simply call "wait()" on every tick, and on non-divisible
	//       time ticks, the WorkGroups will return without performing
	//       a barrier sync.
	/////////////////////////////////////////////////////////////////
	timeval loop_start_time;
	gettimeofday(&loop_start_time, nullptr);
	int loop_start_offset = (int) ProfileBuilder::diff_ms(loop_start_time, start_time);

	StateSwitcher<int> numTicksShown(0); //Only goes up to 10
	StateSwitcher<int> lastTickPercent(0); //So we have some idea how much time is left.
	int endTick = config.totalRuntimeTicks;
	
	for (unsigned int currTick = 0; currTick < endTick; currTick++) 
	{
        if (config.InteractiveMode())
		{
			if(ctrlMgr->getSimState() == STOP) 
			{
				while (ctrlMgr->getEndTick() < 0) 
				{
					ctrlMgr->setEndTick(currTick+2);
				}
				
				endTick = ctrlMgr->getEndTick();
			}
		}

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
		}

		//Agent-based cycle, steps 1,2,3,4 of 4
		wgMgr.waitAllGroups();
		
		unsigned long currTimeMS = currTick * config.baseGranMS();

		//Check if we are running in closed loop with DynaMIT
		if(config.simulation.closedLoop.enabled && (currTimeMS + config.baseGranMS()) % (config.simulation.closedLoop.sensorStepSize * 1000) == 0)
		{
			SurveillanceStation::writeSurveillanceOutput(config, currTimeMS + config.baseGranMS());
			ClosedLoopRunManager::waitForDynaMIT(config);
		}

		if(stCfg.outputStats.segDensityMap.outputEnabled && ((currTimeMS + config.baseGranMS()) % stCfg.outputStats.segDensityMap.updateInterval == 0))
		{
			DriverMovement::outputDensityMap((unsigned int) (currTimeMS / stCfg.outputStats.segDensityMap.updateInterval));
		}
	}

	timeval loop_end_time;
	gettimeofday(&loop_end_time, nullptr);
	int loop_time = (int) ProfileBuilder::diff_ms(loop_end_time, loop_start_time);
	Print() << "100%\n\nTime required to execute the simulation: "
	        << DailyTime((uint32_t) loop_time).getStrRepr() << std::endl;

	//Finalize partition manager
	if (!config.MPI_Disabled() && config.using_MPI) 
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.stopMPIEnvironment();
	}
	
	//Store the segment travel times
	if (config.PathSetMode()) 
	{
		TravelTimeManager::getInstance()->storeCurrentSimulationTT();;
	}

	if(config.odTTConfig.enabled)
	{
		TravelTimeManager::getInstance()->dumpODTravelTimeToFile(config.odTTConfig.fileName);
	}

	if(config.rsTTConfig.enabled)
	{
		TravelTimeManager::getInstance()->dumpSegmentTravelTimeToFile(config.rsTTConfig.fileName);
	}

	Print() << "Time required for initialisation [Loading configuration, network, demand ...]: "
	        << DailyTime((uint32_t) loop_start_offset).getStrRepr() << std::endl;

	Print() << "\nNumber of trips/activities simulated: " << config.numTripsSimulated
	        << "\nNumber of trips/activities completed: " << config.numTripsCompleted << "\n";

	size_t numActivities = 0, numBusDriver = 0, numCarPassenger = 0, numDriver = 0, numPassenger = 0, numPedestrian = 0;
	size_t numPersons = 0, numPrivateBusPassenger = 0, numTrainPassenger = 0, numWaitBus = 0;

	for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); ++it)
	{
		Person_ST *person = dynamic_cast<Person_ST *> (*it);

		if (person)
		{
			Role<Person_ST> *role = person->getRole();

			if (role)
			{
				numPersons++;
				switch (role->roleType)
				{
				case Role<Person_ST>::RL_ACTIVITY:
					numActivities++;
					break;
				case Role<Person_ST>::RL_BUSDRIVER:
					numBusDriver++;
					break;
				case Role<Person_ST>::RL_CARPASSENGER:
					numCarPassenger++;
					break;
				case Role<Person_ST>::RL_DRIVER:
					numDriver++;
					break;
				case Role<Person_ST>::RL_PASSENGER:
					numPassenger++;
					break;
				case Role<Person_ST>::RL_PEDESTRIAN:
					numPedestrian++;
					break;
				case Role<Person_ST>::RL_PRIVATEBUSPASSENGER:
					numPrivateBusPassenger++;
					break;
				case Role<Person_ST>::RL_TRAINPASSENGER:
					numTrainPassenger++;
					break;
				case Role<Person_ST>::RL_WAITBUSACTIVITY:
					numWaitBus++;
					break;
				}
			}
		}
	}

	Print() << "\nPersons still in the simulation: " << numPersons << "\n"
			<< numActivities << " Performing activity,\t" << numBusDriver << " BusDrivers,\t"
			<< numCarPassenger << " CarPassengers,\t" << numDriver << " Drivers,\t"
			<< numPassenger << " Passengers,\t" << numPedestrian << " Pedestrians,\t"
			<< numPrivateBusPassenger << " PrivateBusPassenger,\t"
			<< numTrainPassenger << " TrainPassengers,\t"	<< numWaitBus << " Waiting for bus\n";

    if (config.numAgentsKilled > 0)
	{
		Print() << "\nAgents removed from simulation due to errors [Refer to warn.log for more details]: "
		        << config.numAgentsKilled << endl;
	}

	if(config.numPathNotFound > 0)
	{
		Print() << "\nPersons not simulated as the path was not found [Refer to warn.log for more details]: "
		        << config.numPathNotFound << endl;
	}

	if (!Agent::pending_agents.empty()) 
	{
		Print() << "\nWARNING! There are still " << Agent::pending_agents.size()
				<< " agents waiting to be scheduled. Next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
	}
	
	PT_Statistics::getInstance()->storeStatistics();
	PT_Statistics::resetInstance();

	//Save our output files if we are merging them later.
    if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && config.mergeLogFiles)
	{
		resLogFiles = wgMgr.retrieveOutFileNames();
	}

	//Here, we will simply scope-out the WorkGroups, and they will migrate out all remaining Agents.
	}  //End scope: WorkGroups. (Todo: should move this into its own function later)
	//WorkGroup::FinalizeAllWorkGroups();

	//At this point, it should be possible to delete all Signals and Agents.
	//TODO: For some reason, clear_delete_vector() does (may?) not work in INTERACTIVE mode.
	//      We can address this later, but it should *definitely* be possible to cleanly
	//      exit (even early) from the simulation.
	//TODO: I think that the WorkGroups and Workers need to have the "endTick" value propagated to
	//      them from the main loop, in the event that the simulator is shutting down early. This is
	//      probably causing the Workers to hang if clear_delete_vector is called. ~Seth
	//EDIT: Actually, Worker seems to handle the synchronisation fine too.... but I still think the main
	//      loop should propagate this value down. ~Seth
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) 
	{
		Agent::all_agents.clear();
	} 
	else 
	{
		clear_delete_map(IntersectionManager::getIntManagers());
		clear_delete_map(Signal::getMapOfIdVsSignals());
		BusStopAgent::removeAllBusStopAgents();
		clear_delete_vector(Agent::all_agents);
	}

	Print() << "\nSimulation complete. Closing worker threads...\n" << endl;
	
	//Destroy the road network
	NetworkLoader::deleteInstance();

	//Delete the AMOD controller instance
	amod::AMODController::deleteInstance();

	if(FMOD::FMOD_Controller::instanceExists())
	{
		FMOD::FMOD_Controller::instance()->finalizeMessageToFMOD();
	}

	//Delete the aura manger implementation instance
	AuraManager::instance().destroy();

	//Delete our profiler, if it exists.
	safe_delete_item(prof);

	return true;
}

/**
 * Run the main loop of Sim Mobility, using command-line input.
 * Returns the value of the last completed run of performMain().
 */
int run_simmob_interactive_loop()
{
	ControlManager *ctrlMgr = ConfigManager::GetInstance().FullConfig().getControlMgr();
	std::list<std::string> resLogFiles;
	int retVal = 1;
	
	for (;;) 
	{
		if(ctrlMgr->getSimState() == LOADSCENARIO)
		{
			ctrlMgr->setSimState(RUNNING);
			std::map<std::string,std::string> paras;
			ctrlMgr->getLoadScenarioParas(paras);
			std::string configFileName = paras["configFileName"];
            std::string stConfigFile = paras["shortTermConfigFile"];
            retVal = performMain(configFileName, stConfigFile, resLogFiles, "XML_OutPut.xml") ? 0 : 1;
			ctrlMgr->setSimState(STOP);
			ConfigManager::GetInstanceRW().reset();
			Print() << "scenario finished" << std::endl;
		}
		if(ctrlMgr->getSimState() == QUIT)
		{
			Print() <<"Thank you for using SIMMOB. Have a good day!" <<std::endl;
			break;
		}
	}

	return retVal;
}

int main_impl(int ARGC, char* ARGV[])
{
	std::vector<std::string> args = Utils::parseArgs(ARGC, ARGV);

	//Argument 1: Configuration file
	//Note: Don't change this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/config.xml";
    std::string shortConfigFile = "data/shortTerm.xml";
	
    if (args.size() > 2)
	{
		configFileName = args[1];
		shortConfigFile = args[2];
	}
	else if (args.size() > 1)
	{
		configFileName = args[1];
		Print() << "No short term configuration file specified; Using default short term configuration file..." << endl;
	}
	else
	{
		Print() << "No configuration files specified. \nUsing default short term configuration file: " << shortConfigFile << endl;
		Print() << "Using the default configuration file: " << configFileName << endl;
	}
	
	std::string outputFileName = "out.txt";
	
	if(args.size() > 3)
	{
		outputFileName = args[2];
	}

	//Currently needs the #ifdef because of the way threads initialize.
#ifdef SIMMOB_INTERACTIVE_MODE
	CommunicationManager *dataServer = new CommunicationManager(13333, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread dataWorkerThread(boost::bind(&CommunicationManager::start, dataServer));
	CommunicationManager *cmdServer = new CommunicationManager(13334, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread cmdWorkerThread(boost::bind(&CommunicationManager::start, cmdServer));
	CommunicationManager *roadNetworkServer = new CommunicationManager(13335, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread roadNetworkWorkerThread(boost::bind(&CommunicationManager::start, roadNetworkServer));
	boost::thread workerThread2(boost::bind(&ControlManager::start, ConfigManager::GetInstance().FullConfig().getControlMgr()));
#endif

	//Save start time
	gettimeofday(&start_time, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 */
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	config.using_MPI = false;
	
#ifndef SIMMOB_DISABLE_MPI
	if (args.size()>2 && args[2]=="mpi") {
		config.using_MPI = true;
	}
#endif

	/**
	 * set random be repeatable
	 */
	config.is_simulation_repeatable = true;

	/**
	 * Start MPI if using_MPI is true
	 */
#ifndef SIMMOB_DISABLE_MPI
	if (config.using_MPI)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		std::string mpi_result = partitionImpl.startMPIEnvironment(ARGC, ARGV); //NOTE: MPI_Init needs the raw argc/argv.
		if (mpi_result.compare("") != 0)
		{
			Warn() << "MPI Error:" << mpi_result << endl;
			exit(1);
		}

		ShortTermBoundaryProcessor* boundary_processor = new ShortTermBoundaryProcessor();
		partitionImpl.setBoundaryProcessor(boundary_processor);
	}
#endif

	//Perform main loop (this differs for interactive mode)
	int returnVal;
	std::list<std::string> resLogFiles;
	
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) 
	{
		returnVal = run_simmob_interactive_loop();
	}
    else
	{
		returnVal = performMain(configFileName, shortConfigFile, resLogFiles, "XML_OutPut.xml") ? 0 : 1;
	}

	//Concatenate output files?
	if (!resLogFiles.empty()) 
	{
		resLogFiles.insert(resLogFiles.begin(), config.outNetworkFileName);
		Utils::printAndDeleteLogFiles(resLogFiles,outputFileName);
	}

	int retVal = std::system("rm out_*.txt out.network.txt");

	//Delete the config manager instance
	ConfigManager::DeleteConfigMgrInstance();

	return returnVal;
}
