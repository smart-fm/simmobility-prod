/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * file main_med.cpp
 * A first skeleton for mid-term module
 * author vuvinhan
 */
#include <vector>
#include <string>
#include <ctime>

#include "GenConfig.h"

#include "workers/Worker.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "workers/WorkGroup.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "util/OutputUtil.hpp"
#include "util/DailyTime.hpp"

//Just temporarily, so we know it compiles:
#include "entities/Signal.hpp"
#include "conf/simpleconf.hpp"
#include "entities/AuraManager.hpp"
#include "entities/TrafficWatch.hpp"
#include "entities/Bus.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Route.hpp"
#include "geospatial/BusRoute.hpp"
#include "perception/FixedDelayed.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/Locked.hpp"
#include "buffering/Shared.hpp"

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

//Temporary flag: Shuffle all agents (signals and otherwise) onto the Agent worker threads?
// This is needed for performance testing; it will cause signals to fluxuate faster than they should.
//#define TEMP_FORCE_ONE_WORK_GROUP

//Start time of program
timeval start_time_med;

//Helper for computing differences. May be off by ~1ms
namespace {
int diff_ms(timeval t1, timeval t2) {
    return (((t1.tv_sec - t2.tv_sec) * 1000000) + (t1.tv_usec - t2.tv_usec))/1000;
}
} //End anon namespace

//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;

//Function prototypes.
//void InitializeAllAgentsAndAssignToWorkgroups(vector<Agent*>& agents);
bool CheckAgentIDs(const std::vector<sim_mob::Agent*>& agents);



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
	
	ProfileBuilder* prof = nullptr;
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	ProfileBuilder::InitLogFile("agent_update_trace.txt");
	ProfileBuilder prof_i;
	prof = &prof_i;
#endif

	//Loader params for our Agents
#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);
#endif

	//Load our user config file; save a handle to the shared definition of it.
	if (!ConfigParams::InitUserConf(configFileName, Agent::all_agents, Agent::pending_agents, prof)) { //Note: Agent "shells" are loaded here.
		return false;
	}
	const ConfigParams& config = ConfigParams::GetInstance();


	//Sanity check (nullptr)
	void* x = nullptr;
	if (x) {
		return false;
	}

	//Output
	cout << "  " << "...Sanity Check Passed" << endl;

	//Start boundaries
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.initBoundaryTrafficItems();
	}
#endif

	//Initialize our work groups.
	WorkGroup agentWorkers(config.agentWorkGroupSize, config.totalRuntimeTicks,
			config.granAgentsTicks, true);
	//Agent::TMP_AgentWorkGroup = &agentWorkers;
	//Worker::ActionFunction entityWork = boost::bind(entity_worker, _1, _2);
	agentWorkers.initWorkers(//&entityWork,
#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
		&entLoader
#else
		nullptr
#endif
	);

	bool NoDynamicDispatch = ConfigParams::GetInstance().DynamicDispatchDisabled();
	//randomly assign link to workers
	agentWorkers.assignLinkWorker();

	//Add all agents to workers. If they are in all_agents, then their start times have already been taken
	//  into account; just add them. Otherwise, by definition, they will be in pending_agents.
	for (vector<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
		agentWorkers.assignAWorkerConstraint(*it);
	}

	cout << "Initial Agents dispatched or pushed to pending." << endl;

	//Initialize our signal status work groups
	//  TODO: There needs to be a more general way to do this.
#ifndef TEMP_FORCE_ONE_WORK_GROUP
	WorkGroup signalStatusWorkers(config.signalWorkGroupSize, config.totalRuntimeTicks, config.granSignalsTicks);
	//Worker::ActionFunction spWork = boost::bind(signal_status_worker, _1, _2);
	signalStatusWorkers.initWorkers(/*&spWork,*/ nullptr);
#endif
	for (size_t i = 0; i < Signal::all_signals_.size(); i++) {
		//add by xuyan
//		if(Signal::all_signals_[i]->isFake)
//			continue;
#ifdef TEMP_FORCE_ONE_WORK_GROUP
		agentWorkers.assignAWorkerConstraint(Signal::all_signals_[i]);
#else
		signalStatusWorkers.assignAWorkerConstraint(Signal::all_signals_[i]);
#endif
	}

	//Initialize the aura manager
	AuraManager& auraMgr = AuraManager::instance();
	auraMgr.init();

	//Inititalize the traffic watch
//	TrafficWatch& trafficWatch = TrafficWatch::instance();

	//Start work groups and all threads.
	agentWorkers.startAll();
#ifndef TEMP_FORCE_ONE_WORK_GROUP
	signalStatusWorkers.startAll();
#endif

	//
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(&agentWorkers, &signalStatusWorkers);

		std::cout << "partition_solution_id in main function:" << partitionImpl.partition_config->partition_solution_id << std::endl;
		//std::cout << partitionImpl. << partitionImpl.partition_config->partition_solution_id << std::endl;
		//temp no need
//		if (config.is_simulation_repeatable) {
//			partitionImpl.updateRandomSeed();
//		}
	}
#endif

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

#ifdef SIMMOB_DISABLE_OUTPUT
	int lastTickPercent = 0; //So we have some idea how much time is left.
#endif

	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++) {
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Get a rough idea how far along we are
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output
#ifndef SIMMOB_DISABLE_OUTPUT
		{
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			cout << "Approximate Tick Boundary: " << currTick << ", ";
			cout << (currTick * config.baseGranMS) << " ms   [" <<currTickPercent <<"%]" << endl;
			if (!warmupDone) {
				cout << "  Warmup; output ignored." << endl;
			}
		}
#else
		//We don't need to lock this output if general output is disabled, since Agents won't
		//  perform any output (and hence there will be no contention)
		if (currTickPercent-lastTickPercent>9) {
			lastTickPercent = currTickPercent;
			cout <<currTickPercent <<"%" <<endl;
		}
#endif

		//Update the signal logic and plans for every intersection grouped by region
#ifndef TEMP_FORCE_ONE_WORK_GROUP
		signalStatusWorkers.wait();
#endif
		//Agent-based cycle
		agentWorkers.wait();
#ifndef SIMMOB_DISABLE_MPI
		if (config.is_run_on_many_computers) {
			PartitionManager& partitionImpl = PartitionManager::instance();

//			cout <<"0" <<endl;
			partitionImpl.crossPCBarrier();

//			cout <<"1" <<endl;
			partitionImpl.crossPCboundaryProcess(currTick);

//			cout <<"2" <<endl;
			partitionImpl.crossPCBarrier();

//			cout <<"3" <<endl;
			partitionImpl.outputAllEntities(currTick);
		}
#endif

		auraMgr.update(currTick);
//		trafficWatch.update(currTick);
		agentWorkers.waitExternAgain(); // The workers wait on the AuraManager.



		//Surveillance update
		//updateSurveillanceData(agents);

		//Check if the warmup period has ended.
		if (warmupDone) {
			//updateGUI(agents);
			//saveStatistics(agents);
		}

		//saveStatisticsToDB(agents);
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
				if (p->getRole() && dynamic_cast<Driver*> (p->getRole())) {
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
				<< Agent::pending_agents.top().start << " ms\n";
		if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}
	}

	cout << "Simulation complete; closing worker threads." << endl;
	return true;
}

int main(int argc, char* argv[])
{
#ifdef SIMMOB_NEW_SIGNAL
	std::cout << "Using New Signal Model" << std::endl;
#else
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
#ifndef SIMMOB_DISABLE_OUTPUT
	if (argc > 2) {
		if (!Logger::log_init(argv[2]))
		{
			cout << "Loading output file failed; using cout" << endl;
			cout << argv[2] << endl;
		}
	} else {
		Logger::log_init("");
		cout << "No output file specified; using cout." << endl;
	}
#endif

	//This should be moved later, but we'll likely need to manage random numbers
	//ourselves anyway, to make simulations as repeatable as possible.
	//if (config.is_simulation_repeatable)
	//{
		//TODO: Output the random seed here (and only here)
	//}

	//Perform main loop
	int returnVal = performMainMed(configFileName) ? 0 : 1;

	//Close log file, return.
#ifndef SIMMOB_DISABLE_OUTPUT
	Logger::log_done();
#endif
	cout << "Done" << endl;
	return returnVal;
}

