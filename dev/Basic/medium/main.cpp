//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * file main_med.cpp
 * A first skeleton for mid-term module
 * \author Vu Vinh An
 */
#include <vector>
#include <string>

//TODO: Replace with <chrono> or something similar.
#include <sys/time.h>

//main.cpp (top-level) files can generally get away with including GenConfig.h
#include "GenConfig.h"

#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ParseConfigFile.hpp"
#include "conf/ExpandAndValidateConfigFile.hpp"
#include "entities/AuraManager.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/models/CarFollowModel.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "entities/models/IntersectionDrivingModel.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"
#include "util/Utils.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"
#include "logging/Log.hpp"
#include "entities/BusController.hpp"

//If you want to force a header file to compile, you can put it here temporarily:
//#include "entities/BusController.hpp"

//add by xuyan
#include "partitions/PartitionManager.hpp"

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

namespace {

/**
 * For now, the medium-term expects the following models to be available. We have to fake these,
 * otherwise the config file will fail to parse. Later, we will have separate config file
 * "model" sections for the short/medium term (most likely via a plugin architecture).
 */
class Fake_CF_Model : public CarFollowModel {
public:
	virtual double makeAcceleratingDecision(sim_mob::DriverUpdateParams& p, double targetSpeed, double maxLaneSpeed) {
		throw std::runtime_error("Fake CF_model used for medium term.");
	}
};
class Fake_LC_Model : public LaneChangeModel {
public:
	virtual double executeLaneChanging(sim_mob::DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir) {
		throw std::runtime_error("Fake LC_model used for medium term.");
	}
};
class Fake_IntDriving_Model : public IntersectionDrivingModel {
public:
	virtual void startDriving(const DPoint& fromLanePt, const DPoint& toLanePt, double startOffset) { throwIt(); }
	virtual DPoint continueDriving(double amount) { throwIt(); return DPoint(); }
	virtual bool isDone() { throwIt(); return false; }
	virtual double getCurrentAngle() { throwIt(); return 0; }
private:
	void throwIt() { throw std::runtime_error("Fake LC_model used for medium term."); }
};
/*void fakeModels(Config::BuiltInModels& builtIn) {
	builtIn.carFollowModels["mitsim"] = new Fake_CF_Model();
	builtIn.laneChangeModels["mitsim"] = new Fake_LC_Model();
	builtIn.intDrivingModels["linear"] = new Fake_IntDriving_Model();
}*/

} //End anon namespace

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
bool performMainMed(const std::string& configFileName, std::list<std::string>& resLogFiles) {
	cout <<"Starting SimMobility, version " <<SIMMOB_VERSION <<endl;
	
	//Parse the config file (this *does not* create anything, it just reads it.).
	ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig());

	//Enable or disable logging (all together, for now).
	//NOTE: This may seem like an odd place to put this, but it makes sense in context.
	//      OutputEnabled is always set to the correct value, regardless of whether ConfigParams()
	//      has been loaded or not. The new Config class makes this much clearer.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		//Log::Init("out.txt");
		Warn::Init("warn.log");
		Print::Init("<stdout>");
	} else {
		//Log::Ignore();
		Warn::Ignore();
		Print::Ignore();
	}

	if (ConfigManager::GetInstance().CMakeConfig().UsingConfluxes()) {
		std::cout << "Confluxes ON!" << std::endl;
	}
	else {
		throw std::runtime_error("Confluxes OFF! Please turn SIMMOB_USE_CONFLUXES on in CMakeCache.txt and run the program again.");
	}

	ProfileBuilder* prof = nullptr;
	if (ConfigManager::GetInstance().CMakeConfig().ProfileOn()) {
		ProfileBuilder::InitLogFile("profile_trace.txt");
		prof = new ProfileBuilder();
	}

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Register our Role types.
	//TODO: Accessing ConfigParams before loading it is technically safe, but we
	//      should really be clear about when this is not ok.
	RoleFactory& rf = ConfigManager::GetInstanceRW().FullConfig().getRoleFactoryRW();
	rf.registerRole("driver", new sim_mob::medium::Driver(nullptr, ConfigManager::GetInstance().FullConfig().mutexStategy()));
	rf.registerRole("activityRole", new sim_mob::ActivityPerformer(nullptr));
	rf.registerRole("busdriver", new sim_mob::medium::BusDriver(nullptr, ConfigManager::GetInstance().FullConfig().mutexStategy()));
	//rf.registerRole("pedestrian", new sim_mob::medium::Pedestrian(nullptr)); //Pedestrian is not implemented yet for medium term


	//No built-in models available to the medium term (yet).
	//Config::BuiltInModels builtIn;
	//fakeModels(builtIn);

	//Load our user config file
	ExpandAndValidateConfigFile expand(ConfigManager::GetInstanceRW().FullConfig(), Agent::all_agents, Agent::pending_agents);
	//Load our user config file
//	ConfigParams::InitUserConf(configFileName, Agent::all_agents, Agent::pending_agents, prof, builtIn);
	std::cout<<"performMainMed: trip chain pool size "<<
			ConfigManager::GetInstance().FullConfig().getTripChains().size()<<std::endl;

	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		// init path set manager
		time_t t = time(0);   // get time now
		struct tm * now = localtime( & t );
		std::cout<<"begin time:"<<std::endl;
		std::cout<<now->tm_hour<<" "<<now->tm_min<<" "<<now->tm_sec<< std::endl;
		PathSetManager* psMgr = PathSetManager::getInstance();
		std::string name=configFileName;
		psMgr->setScenarioName(name);
//		psMgr->setTravleTimeTmpTableName(ConfigParams::GetInstance().travelTimeTmpTableName);
//		psMgr->createTravelTimeTmpTable(psMgr->getTravleTimeTmpTableName());
//		psMgr->getDataFromDB();
		if(psMgr->isUseCatchMode())
		{
			psMgr->generateAllPathSetWithTripChain2();
		}
//		psMgr->saveDataToDB();
		t = time(0);   // get time now
		now = localtime( & t );
		std::cout<<now->tm_hour<<" "<<now->tm_min<<" "<<now->tm_sec<< std::endl;
		std::cout<<psMgr->size()<<std::endl;
	}
	//Save a handle to the shared definition of the configuration.
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();

	//Start boundaries
#ifndef SIMMOB_DISABLE_MPI
	if (config.using_MPI) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.initBoundaryTrafficItems();
	}
#endif

	//bool NoDynamicDispatch = config.DynamicDispatchDisabled();

	PartitionManager* partMgr = nullptr;
	if (!config.MPI_Disabled() && config.using_MPI) {
		partMgr = &PartitionManager::instance();
	}

	{ //Begin scope: WorkGroups
	WorkGroupManager wgMgr;
	wgMgr.setSingleThreadMode(config.singleThreaded());

	//Work Group specifications
	//WorkGroup* agentWorkers = wgMgr.newWorkGroup(config.agentWorkGroupSize, config.totalRuntimeTicks, config.granAgentsTicks, &AuraManager::instance(), partMgr);
	//Mid-term is not using Aura Manager at the moment. Therefore setting it to nullptr
	WorkGroup* personWorkers = wgMgr.newWorkGroup(config.personWorkGroupSize(), config.totalRuntimeTicks, config.granPersonTicks, nullptr /*AuraManager is not used in mid-term*/, partMgr);
	WorkGroup* signalStatusWorkers = wgMgr.newWorkGroup(config.signalWorkGroupSize(), config.totalRuntimeTicks, config.granSignalsTicks);

	//Initialize all work groups (this creates barriers, and locks down creation of new groups).
	wgMgr.initAllGroups();

	//Initialize each work group individually
	personWorkers->initWorkers(&entLoader);
	signalStatusWorkers->initWorkers(nullptr);


	personWorkers->assignConfluxToWorkers();
//	personWorkers->findBoundaryConfluxes();


	//Anything in all_agents is starting on time 0, and should be added now.
	/* Loop detectors are just ignored for now. Later when Confluxes are made compatible with the short term,
	 * they will be assigned a worker.
	 */
	for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
		// personWorkers->assignAWorker(*it);
		personWorkers->putAgentOnConflux(dynamic_cast<sim_mob::Agent*>(*it));
	}

	if(BusController::HasBusControllers()){
		personWorkers->assignAWorker(BusController::TEMP_Get_Bc_1());
	}

	//Assign all signals too
	for (vector<Signal*>::iterator it = Signal::all_signals_.begin(); it != Signal::all_signals_.end(); it++) {
		signalStatusWorkers->assignAWorker(*it);
	}

	cout << "Initial Agents dispatched or pushed to pending." << endl;

	//Initialize the aura manager
	AuraManager::instance().init(config.aura_manager_impl());

	//Start work groups and all threads.
	wgMgr.startAllWorkGroups();

	//
	if (!config.MPI_Disabled() && config.using_MPI) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(personWorkers, signalStatusWorkers);

		std::cout << "partition_solution_id in main function:" << partitionImpl.partition_config->partition_solution_id << std::endl;
	}

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
	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++) {
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Get a rough idea how far along we are
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output
		if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
			std::stringstream msg;
			msg << "Approximate Tick Boundary: " << currTick << ", ";
			msg << (currTick * config.baseGranMS()) << " ms   [" <<currTickPercent <<"%]" << endl;
			if (!warmupDone) {
				msg << "  Warmup; output ignored." << endl;
			}
			PrintOut(msg.str());
		} else {
			//We don't need to lock this output if general output is disabled, since Agents won't
			//  perform any output (and hence there will be no contention)
			if (currTickPercent-lastTickPercent>9) {
				lastTickPercent = currTickPercent;
				cout <<currTickPercent <<"%" <<endl;
			}
		}

		//Agent-based cycle, steps 1,2,3,4 of 4
		wgMgr.waitAllGroups();

		//Check if the warmup period has ended.
		if (warmupDone) {
			//updateGUI(agents);
			//saveStatistics(agents);
		}
	}

	//Finalize partition manager
#ifndef SIMMOB_DISABLE_MPI
	if (config.using_MPI) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.stopMPIEnvironment();
	}
#endif

	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		PathSetManager::getInstance()->copyTravelTimeDataFromTmp2RealtimeTable();
		PathSetManager::getInstance()->dropTravelTimeTmpTable();
	}
	std::cout <<"Database lookup took: " <<loop_start_offset <<" ms" <<std::endl;

	cout << "Max Agents at any given time: " <<maxAgents <<std::endl;
	cout << "Starting Agents: " << numStartAgents;
	cout << ",     Pending: ";
	/*if (NoDynamicDispatch) {
		cout <<"<Disabled>";
	} else {*/
		cout <<numPendingAgents;
	//}
	cout << endl;

	if (Agent::all_agents.empty()) {
		cout << "All Agents have left the simulation.\n";
	} else {
		size_t numPerson = 0;
		size_t numDriver = 0;
		size_t numPedestrian = 0;
		for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
			Person* p = dynamic_cast<Person*> (*it);
			if (p) {
				numPerson++;
				if(p->getRole())
				if ( dynamic_cast<sim_mob::medium::Driver*>(p->getRole())) {
					numDriver++;
				}
				if (p->getRole() && dynamic_cast<sim_mob::medium::Pedestrian*> (p->getRole())) {
					numPedestrian++;
				}
			}
		}
		cout << "Remaining Agents: " << numPerson << " (Person)   "
				<< (Agent::all_agents.size() - numPerson) << " (Other)" << endl;
		cout << "   Person Agents: " << numDriver << " (Driver)   "
				<< numPedestrian << " (Pedestrian)   " << (numPerson
				- numDriver - numPedestrian) << " (Other)" << endl;
	}

	if (ConfigManager::GetInstance().FullConfig().numAgentsSkipped>0) {
		cout <<"Agents SKIPPED due to invalid route assignment: " <<ConfigManager::GetInstance().FullConfig().numAgentsSkipped <<endl;
	}

	if (!Agent::pending_agents.empty()) {
		cout << "WARNING! There are still " << Agent::pending_agents.size()
				<< " Agents waiting to be scheduled; next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
		/*if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}*/
	}

	//Save our output files if we are merging them later.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && ConfigManager::GetInstance().FullConfig().mergeLogFiles()) {
		resLogFiles = wgMgr.retrieveOutFileNames();
	}

	//NOTE: This dangerous behavior; the Worker will still be tracking the Agent!  ~Seth
	//BusController::busctrller.currWorker = nullptr;// Update our Entity's pointer before ending main()

	//This is the safer way to do it:
	//BusController::busctrller.currWorker->migrateOut(BusController::busctrller);
	//...but that method is private (and fails to do other important things, like removing managed properties).

	//Instead, we will simply scope-out the WorkGroups, and they will migrate out all remaining Agents.
	}  //End scope: WorkGroups. (Todo: should move this into its own function later)
	//wgMgr.finalizeAllWorkGroups();

	//Test: At this point, it should be possible to delete all Signals and Agents.
	clear_delete_vector(Signal::all_signals_);
	clear_delete_vector(Agent::all_agents);

	cout << "Simulation complete; closing worker threads." << endl;

	//Delete our profile pointer (if it exists)
	safe_delete_item(prof);
	return true;
}

int main(int ARGC, char* ARGV[])
{
	std::vector<std::string> args = Utils::ParseArgs(ARGC, ARGV);

	//Save start time
	gettimeofday(&start_time_med, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 */
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	config.using_MPI = false;
#ifndef SIMMOB_DISABLE_MPI
	if (args.size() > 2 && args[2]=="mpi") {
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
	std::string configFileName = "data/config.xml";

	if (args.size() > 1) {
		configFileName = args[1];
	} else {
		cout << "No config file specified; using default." << endl;
	}
	Print() << "Using config file: " << configFileName << endl;

	//Argument 2: Log file
	/*string logFileName = args.size()>2 ? args[2] : "out.txt";
	if (ConfigParams::GetInstance().OutputEnabled()) {
		if (!Logger::log_init(logFileName)) {
			Print() <<"Failed to initialized log file: \"" <<logFileName <<"\"" <<", defaulting to cout." <<endl;
		}
	}*/

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
	int returnVal = performMainMed(configFileName, resLogFiles) ? 0 : 1;

	//Concatenate output files?
	if (!resLogFiles.empty()) {
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
		Utils::PrintAndDeleteLogFiles(resLogFiles);
	}

	timeval simEndTime;
	gettimeofday(&simEndTime, nullptr);

	Print() << "Done" << endl;
	cout << "Total simulation time: "<< ProfileBuilder::diff_ms(simEndTime, simStartTime) << " ms." << endl;

	return returnVal;
}

