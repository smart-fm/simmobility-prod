//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <vector>
#include <string>
#include <set>
#include <cstdlib>

//TODO: Replace with <chrono> or something similar.
#include <sys/time.h>

//main.cpp (top-level) files can generally get away with including GenConfig.h
#include "GenConfig.h"

#include "behavioral/PredayManager.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/ExpandMidTermConfigFile.hpp"
#include "config/MT_Config.hpp"
#include "config/ParseMidTermConfigFile.hpp"
#include "conf/ParseConfigFile.hpp"
#include "database/DB_Connection.hpp"
#include "database/pt_network_dao/PT_NetworkSqlDao.hpp"
#include "entities/Agent.hpp"
#include "entities/AuraManager.hpp"
#include "entities/BusController.hpp"
#include "entities/TrainController.hpp"
#include "entities/TrainController.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/TrainStationAgent.hpp"
#include "entities/PT_EdgeTravelTime.hpp"
#include "entities/incident/IncidentManager.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "entities/MT_PersonLoader.hpp"
#include "entities/Person_MT.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/Biker.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "entities/ScreenLineCounter.hpp"
#include "entities/TravelTimeManager.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/streetdir/A_StarPublicTransitShortestPathImpl.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/Lane.hpp"
#include "logging/Log.hpp"
#include "partitions/PartitionManager.hpp"
#include "path/PathSetManager.hpp"
#include "path/PathSetParam.hpp"
#include "path/PT_PathSetManager.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"


//If you want to force a header file to compile, you can put it here temporarily:
//#include "entities/BusController.hpp"

//Note: This must be the LAST include, so that other header files don't have
//      access to cout if output is disabled.
#include <iostream>

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
	std::map<std::string, TrainStop*>&  MRTStopMap = PT_Network::getInstance().MRTStopsMap;
	std::map<std::string, TrainStop*>::iterator trainStopIt;
	for(trainStopIt = MRTStopMap.begin();trainStopIt!=MRTStopMap.end();trainStopIt++){
		Agent* stationAgent = new TrainStationAgent();
		TrainController<Person_MT>::registerStationAgent(trainStopIt->first, stationAgent);
		const Node* node = trainStopIt->second->getRandomStationSegment()->getParentLink()->getFromNode();
		ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
		MT_Config& mtCfg = MT_Config::getInstance();
		std::map<const Node*, Conflux*>& nodeConfluxesMap = mtCfg.getConfluxNodes();
		std::map<const Node*, Conflux*>::iterator it = nodeConfluxesMap.find(node);
		if(it!=nodeConfluxesMap.end()){
			it->second->addStationAgent(stationAgent);
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
	rf->registerRole("waitBusActivity", new sim_mob::medium::WaitBusActivity(nullptr));
	rf->registerRole("pedestrian", new sim_mob::medium::Pedestrian(nullptr));
	rf->registerRole("passenger", new sim_mob::medium::Passenger(nullptr));
	rf->registerRole("biker", new sim_mob::medium::Biker(nullptr));

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
	//Save handles to definition of configurations.
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const MT_Config& mtConfig = MT_Config::getInstance();

	PeriodicPersonLoader* periodicPersonLoader = new MT_PersonLoader(Agent::all_agents, Agent::pending_agents);
	const ScreenLineCounter* screenLnCtr = ScreenLineCounter::getInstance(); //This line is necessary. It creates the singleton ScreenlineCounter object before any workers are created.

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

	if(BusController::HasBusController())
	{
		personWorkers->assignAWorker(BusController::GetInstance());
	}
	if(TrainController<Person_MT>::HasTrainController())
	{
		personWorkers->assignAWorker(TrainController<Person_MT>::getInstance());
	}
	//incident
	personWorkers->assignAWorker(IncidentManager::getInstance());

	//before starting the groups, initialize the time interval for one of the pathset manager's helpers
	PathSetManager::initTimeInterval();

	cout << "Initial Agents dispatched or pushed to pending.\nall_agents: " << Agent::all_agents.size() << " pending: " << Agent::pending_agents.size() << endl;

	//Start work groups and all threads.
	wgMgr.startAllWorkGroups();

	/////////////////////////////////////////////////////////////////
	// NOTE: WorkGroups are able to handle skipping steps by themselves.
	//       So, we simply call "wait()" on every tick, and on non-divisible
	//       time ticks, the WorkGroups will return without performing
	//       a barrier sync.
	/////////////////////////////////////////////////////////////////
	size_t numStartAgents = Agent::all_agents.size();
	size_t numPendingAgents = Agent::pending_agents.size();
	size_t maxAgents = Agent::all_agents.size();

	timeval loop_start_time;
	gettimeofday(&loop_start_time, nullptr);
	int loop_start_offset = ProfileBuilder::diff_ms(loop_start_time, start_time_med);

	int lastTickPercent = 0; //So we have some idea how much time is left.
	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++)
	{
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Get a rough idea how far along we are
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output
		if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
		{
			std::stringstream msg;
			msg << "Approximate Tick Boundary: " << currTick << ", ";
			msg << (currTick * config.baseGranSecond())
				<< "s   [" <<currTickPercent <<"%]" << endl;
			if (!warmupDone)
			{
				msg << "  Warmup; output ignored." << endl;
			}
			PrintOut(msg.str());
		}
		else
		{
			//We don't need to lock this output if general output is disabled, since Agents won't
			//  perform any output (and hence there will be no contention)
			if (currTickPercent-lastTickPercent>9)
			{
				lastTickPercent = currTickPercent;
				cout<< currTickPercent <<"%"
					<< ", Agents:" << Agent::all_agents.size() <<endl;
			}
		}

		//Agent-based cycle, steps 1,2,3,4
		wgMgr.waitAllGroups();
	}

	BusStopAgent::removeAllBusStopAgents();
	sim_mob::PathSetParam::resetInstance();

	//finalize
	TravelTimeManager::getInstance()->storeCurrentSimulationTT();

	cout <<"Database lookup took: " << (loop_start_offset/1000.0) <<" s" <<endl;
	cout << "Max Agents at any given time: " <<maxAgents <<endl;
	cout << "Starting Agents: " << numStartAgents
			<< ",     Pending: " << numPendingAgents << endl;

	if (Agent::all_agents.empty() && Agent::pending_agents.empty())
	{
		cout << "All Agents have left the simulation.\n";
	}
	else
	{
		cout<< "Pending Agents: " << (Agent::all_agents.size() + Agent::pending_agents.size()) << endl;
	}

	PT_Statistics::getInstance()->storeStatistics();
	PT_Statistics::resetInstance();

	if(mtConfig.enabledEdgeTravelTime)
	{
		PT_EdgeTravelTime::getInstance()->exportEdgeTravelTime();
	}

	if (config.numAgentsSkipped>0)
	{
		cout<<"Agents SKIPPED due to invalid route assignment: "
			<<config.numAgentsSkipped
			<<endl;
	}

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
	safe_delete_item(periodicPersonLoader);
	cout << "Simulation complete; closing worker threads." << endl;

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
	const db::BackendType populationSource = mtConfig.getPopulationSource();
	PredayManager predayManager;
	predayManager.loadZones(db::MONGO_DB);
	predayManager.load2012_2008ZoneMapping(db::MONGO_DB);
	predayManager.loadCosts(db::MONGO_DB);
	predayManager.loadPersonIds(populationSource);
	predayManager.loadUnavailableODs(db::MONGO_DB);
	if(mtConfig.runningPredaySimulation() && mtConfig.isFileOutputEnabled())
	{
		predayManager.loadZoneNodes(db::MONGO_DB);
		predayManager.loadPostcodeNodeMapping(db::POSTGRES);
	}

	if(mtConfig.runningPredayCalibration())
	{
		Print() << "Preday mode: calibration" << std::endl;
		predayManager.calibratePreday();
	}
	else
	{
		Print() << "Preday mode: " << (mtConfig.runningPredaySimulation()? "simulation":"logsum computation")  << std::endl;
		if(populationSource == db::POSTGRES) { predayManager.dispatchLT_Persons(); }
		else { predayManager.dispatchMongodbPersons(); }
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
	}
	else
	{
		//Log::Ignore();
		Warn::Ignore();
		Print::Ignore();
	}

    if (MT_Config::getInstance().RunningMidSupply())
	{
		Print() << "Mid-term run mode: supply" << endl;
		return performMainSupply(configFileName, resLogFiles);
	}
    else if (MT_Config::getInstance().RunningMidDemand())
	{
		Print() << "Mid-term run mode: preday" << endl;
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
	Print() << "Using config file: " << configFileName << endl;

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
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
		Utils::printAndDeleteLogFiles(resLogFiles);
	}

	timeval simEndTime;
	gettimeofday(&simEndTime, nullptr);

	Print() << "Done" << endl;
	cout << "Total simulation time: "<< (ProfileBuilder::diff_ms(simEndTime, simStartTime))/1000.0 << " seconds." << endl;

	return returnVal;
}

