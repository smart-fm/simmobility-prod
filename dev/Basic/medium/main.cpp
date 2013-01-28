/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * file main_med.cpp
 * A first skeleton for mid-term module
 * \author Vu Vinh An
 */
#include <vector>
#include <string>
#include <ctime>

#include "GenConfig.h"

#include "buffering/BufferedDataManager.hpp"
#include "entities/AuraManager.hpp"
#include "entities/Signal.hpp"
#include "entities/Agent.hpp"
#include "entities/models/CarFollowModel.hpp"
#include "entities/models/LaneChangeModel.hpp"
#include "entities/models/IntersectionDrivingModel.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
//#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "util/OutputUtil.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"

//If you want to force a header file to compile, you can put it here temporarily:
//#include "entities/BusController.hpp"

//add by xuyan
#include "partitions/PartitionManager.hpp"

//Note: This must be the LAST include, so that other header files don't have
//      access to cout if SIMMOB_DISABLE_OUTPUT is true.
#include <iostream>

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;
using namespace sim_mob::medium;

//Start time of program
timeval start_time_med;

//Helper for computing differences. May be off by ~1ms
namespace {
int diff_ms(timeval t1, timeval t2) {
    return (((t1.tv_sec - t2.tv_sec) * 1000000) + (t1.tv_usec - t2.tv_usec))/1000;
}

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
void fakeModels(Config::BuiltInModels& builtIn) {
	builtIn.carFollowModels["mitsim"] = new Fake_CF_Model();
	builtIn.laneChangeModels["mitsim"] = new Fake_LC_Model();
	builtIn.intDrivingModels["linear"] = new Fake_IntDriving_Model();
}

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
bool performMainMed(const std::string& configFileName) {
	cout <<"Starting SimMobility, version " <<SIMMOB_VERSION <<endl;
	
#ifdef SIMMOB_USE_CONFLUXES
	std::cout << "Confluxes ON!" << std::endl;
#endif

	ProfileBuilder* prof = nullptr;
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	ProfileBuilder::InitLogFile("agent_update_trace.txt");
	ProfileBuilder prof_i;
	prof = &prof_i;
#endif

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Register our Role types.
	//TODO: Accessing ConfigParams before loading it is technically safe, but we
	//      should really be clear about when this is not ok.
	RoleFactory& rf = ConfigParams::GetInstance().getRoleFactoryRW();
	rf.registerRole("driver", new sim_mob::medium::Driver(nullptr, ConfigParams::GetInstance().mutexStategy));
	rf.registerRole("pedestrian", new sim_mob::medium::Pedestrian(nullptr));
	//rf.registerRole("BusPassenger", new sim_mob::Passenger(nullptr));
	rf.registerRole("activityRole", new sim_mob::ActivityPerformer(nullptr));

	//No built-in models available to the medium term (yet).
	Config::BuiltInModels builtIn;
	fakeModels(builtIn);

	//Load our user config file
	if (!ConfigParams::InitUserConf(configFileName, Agent::all_agents, Agent::pending_agents, prof, builtIn)) {
		return false;
	}

	//Save a handle to the shared definition of the configuration.
	const ConfigParams& config = ConfigParams::GetInstance();


	//Start boundaries
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.initBoundaryTrafficItems();
	}
#endif

	bool NoDynamicDispatch = config.DynamicDispatchDisabled();

	PartitionManager* partMgr = nullptr;
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		partMgr = &PartitionManager::instance();
	}

	{ //Begin scope: WorkGroups
	//Work Group specifications
	WorkGroup* agentWorkers = WorkGroup::NewWorkGroup(config.agentWorkGroupSize, config.totalRuntimeTicks, config.granAgentsTicks, &AuraManager::instance(), partMgr);
	WorkGroup* signalStatusWorkers = WorkGroup::NewWorkGroup(config.signalWorkGroupSize, config.totalRuntimeTicks, config.granSignalsTicks);

	//Initialize all work groups (this creates barriers, and locks down creation of new groups).
	WorkGroup::InitAllGroups();

	//Initialize each work group individually
	agentWorkers->initWorkers(NoDynamicDispatch ? nullptr :  &entLoader);
	signalStatusWorkers->initWorkers(nullptr);


	agentWorkers->assignConfluxToWorkers();

	//Anything in all_agents is starting on time 0, and should be added now.
	/* Loop detectors are just ignored for now. Later when Confluxes are made compatible with the short term,
	 * they will be assigned a worker.
	 */
	for (vector<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
		// agentWorkers->assignAWorker(*it);
		agentWorkers->putAgentOnConflux(dynamic_cast<sim_mob::Agent*>(*it));
	}

	//Assign all signals too
	for (vector<Signal*>::iterator it = Signal::all_signals_.begin(); it != Signal::all_signals_.end(); it++) {
		signalStatusWorkers->assignAWorker(*it);
	}

	cout << "Initial Agents dispatched or pushed to pending." << endl;

	//Initialize the aura manager
	AuraManager::instance().init();

	//Start work groups and all threads.
	WorkGroup::StartAllWorkGroups();

	//
	if (!config.MPI_Disabled() && config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(agentWorkers, signalStatusWorkers);

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
	int loop_start_offset = diff_ms(loop_start_time, start_time_med);

	int lastTickPercent = 0; //So we have some idea how much time is left.
	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++) {
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Get a rough idea how far along we are
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output
		if (ConfigParams::GetInstance().OutputEnabled()) {
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			cout << "Approximate Tick Boundary: " << currTick << ", ";
			cout << (currTick * config.baseGranMS) << " ms   [" <<currTickPercent <<"%]" << endl;
			if (!warmupDone) {
				cout << "  Warmup; output ignored." << endl;
			}
		} else {
			//We don't need to lock this output if general output is disabled, since Agents won't
			//  perform any output (and hence there will be no contention)
			if (currTickPercent-lastTickPercent>9) {
				lastTickPercent = currTickPercent;
				cout <<currTickPercent <<"%" <<endl;
			}
		}

		//Agent-based cycle, steps 1,2,3,4 of 4
		WorkGroup::WaitAllGroups();

		//Check if the warmup period has ended.
		if (warmupDone) {
			//updateGUI(agents);
			//saveStatistics(agents);
		}
	}

	//Finalize partition manager
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.stopMPIEnvironment();
	}
#endif

	std::cout <<"Database lookup took: " <<loop_start_offset <<" ms" <<std::endl;

	cout << "Max Agents at any given time: " <<maxAgents <<std::endl;
	cout << "Starting Agents: " << numStartAgents;
	cout << ",     Pending: ";
	if (NoDynamicDispatch) {
		cout <<"<Disabled>";
	} else {
		cout <<numPendingAgents;
	}
	cout << endl;

	if (Agent::all_agents.empty()) {
		cout << "All Agents have left the simulation.\n";
	} else {
		size_t numPerson = 0;
		size_t numDriver = 0;
		size_t numPedestrian = 0;
		for (vector<Entity*>::iterator it = Agent::all_agents.begin(); it
				!= Agent::all_agents.end(); it++) {
			Person* p = dynamic_cast<Person*> (*it);
			if (p) {
				numPerson++;
				if (p->getRole() && dynamic_cast<Driver*>(p->getRole())) {
					numDriver++;
				}
				if (p->getRole() && dynamic_cast<Pedestrian*> (p->getRole())) {
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

	if (ConfigParams::GetInstance().numAgentsSkipped>0) {
		cout <<"Agents SKIPPED due to invalid route assignment: " <<ConfigParams::GetInstance().numAgentsSkipped <<endl;
	}

	if (!Agent::pending_agents.empty()) {
		cout << "WARNING! There are still " << Agent::pending_agents.size()
				<< " Agents waiting to be scheduled; next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
		if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}
	}

	//NOTE: This dangerous behavior; the Worker will still be tracking the Agent!  ~Seth
	//BusController::busctrller.currWorker = nullptr;// Update our Entity's pointer before ending main()

	//This is the safer way to do it:
	//BusController::busctrller.currWorker->migrateOut(BusController::busctrller);
	//...but that method is private (and fails to do other important things, like removing managed properties).

	//Instead, we will simply scope-out the WorkGroups, and they will migrate out all remaining Agents.
	}  //End scope: WorkGroups. (Todo: should move this into its own function later)
	WorkGroup::FinalizeAllWorkGroups();

	//Test: At this point, it should be possible to delete all Signals and Agents.
	clear_delete_vector(Signal::all_signals_);
	clear_delete_vector(Agent::all_agents);

	cout << "Simulation complete; closing worker threads." << endl;
	return true;
}

int main(int argc, char* argv[])
{
	std::cout << "Using New Signal Model" << std::endl;

#if 0
	std::cout << "Not Using New Signal Model" << std::endl;
#endif

	//Save start time
	gettimeofday(&start_time_med, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 */
	ConfigParams& config = ConfigParams::GetInstance();
	config.is_run_on_many_computers = false;
#ifndef SIMMOB_DISABLE_MPI
	if (argc > 3 && strcmp(argv[3], "mpi") == 0) {
		config.is_run_on_many_computers = true;
	}
#endif

	/**
	 * set random be repeatable
	 */
	config.is_simulation_repeatable = true;

	/**
	 * Start MPI if is_run_on_many_computers is true
	 */
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		std::string mpi_result = partitionImpl.startMPIEnvironment(argc, argv);
		if (mpi_result.compare("") != 0)
		{
			cout << "Error:" << mpi_result << endl;
			exit(1);
		}
	}
#endif

	//Argument 1: Config file
	//Note: Don't change this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/config.xml";
	if (argc > 1)
	{
		configFileName = argv[1];
	}
	else
	{
		cout << "No config file specified; using default." << endl;
	}
	cout << "Using config file: " << configFileName << endl;

	//Argument 2: Log file
	string logFileName = argc>2 ? argv[2] : "";
	if (ConfigParams::GetInstance().OutputEnabled()) {
		if (!Logger::log_init(logFileName)) {
			cout <<"Failed to initialized log file: \"" <<logFileName <<"\"" <<", defaulting to cout." <<endl;
		}
	}

	//This should be moved later, but we'll likely need to manage random numbers
	//ourselves anyway, to make simulations as repeatable as possible.
	//if (config.is_simulation_repeatable)
	//{
		//TODO: Output the random seed here (and only here)
	//}

	//Perform main loop
	int returnVal = performMainMed(configFileName) ? 0 : 1;

	//Close log file, return.
	if (ConfigParams::GetInstance().OutputEnabled()) {
		Logger::log_done();
	}
	cout << "Done" << endl;
	return returnVal;
}

