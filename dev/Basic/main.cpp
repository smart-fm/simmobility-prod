/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file main.cpp
 * A first approximation of the basic pseudo-code in C++. The main file loads several
 * properties from data/config.xml, and attempts a simulation run. Currently, the various
 * granularities and pedestrian starting locations are loaded.
 */
#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>
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
#include "entities/Bus.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Route.hpp"
#include "geospatial/BusRoute.hpp"
#include "perception/FixedDelayed.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/Locked.hpp"
#include "buffering/Shared.hpp"

//add by xuyan
#include "partitions/PartitionManager.hpp"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using boost::thread;

using namespace sim_mob;

//Start time of program
timeval start_time;

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
bool TestTimeClass();

///Worker function for entity-related loading tasks.
void entity_worker(sim_mob::Worker& wk, frame_t frameNumber)
{
	for (vector<Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		if (!(*it)->update(frameNumber)) {
			//This Entity is done; schedule for deletion.
#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
			wk.scheduleForRemoval(*it);
#endif
		}
	}
}

///Worker function for signal status loading task.
void signal_status_worker(sim_mob::Worker& wk, frame_t frameNumber)
{
	for (vector<Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		(*it)->update(frameNumber);
	}
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
bool performMain(const std::string& configFileName) {
	cout <<"Starting SimMobility, version " <<SIMMOB_VERSION <<endl;

	//Loader params for our Agents
#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents,
			Agent::all_agents);
#endif

	//Initialization: Scenario definition
	vector<Entity*>& agents = Agent::all_agents;

	//Load our user config file; save a handle to the shared definition of it.
	if (!ConfigParams::InitUserConf(configFileName, agents)) { //Note: Agent "shells" are loaded here.
		return false;
	}
	const ConfigParams& config = ConfigParams::GetInstance();


	//Sanity check (nullptr)
	void* x = nullptr;
	if (x) {
		return false;
	}

	//Sanity check (time class)
	//TODO: Move to their own unit tests!
	if (!TestTimeClass()) {
		std::cout << "Aborting: Time class tests failed.\n";
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
	WorkGroup agentWorkers(WG_AGENTS_SIZE, config.totalRuntimeTicks,
			config.granAgentsTicks, true);
	//Agent::TMP_AgentWorkGroup = &agentWorkers;
	Worker::ActionFunction entityWork = boost::bind(entity_worker, _1, _2);
	agentWorkers.initWorkers(&entityWork,
#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
		&entLoader
#else
		nullptr
#endif
	);

	bool NoDynamicDispatch = ConfigParams::GetInstance().DynamicDispatchDisabled();

	//Migrate all Agents from the all_agents array to the pending_agents priority queue unless they are
	// actually starting at time tick zero.
	vector<Entity*> starting_agents;
	for (vector<Entity*>::iterator it = agents.begin(); it != agents.end(); it++) {
		Entity* const ag = *it;
		if (NoDynamicDispatch || ag->getStartTime()==0) {
			//Only agents with a start time of zero should start immediately in the all_agents list.
			agentWorkers.assignAWorker(ag);
			starting_agents.push_back(ag);
		} else {
			//Start later.
			Agent::pending_agents.push(ag);
		}
	}
	agents.clear();
	agents.insert(agents.end(), starting_agents.begin(), starting_agents.end());

	cout << "Initial Agents dispatched or pushed to pending." << endl;

	//Initialize our signal status work groups
	//  TODO: There needs to be a more general way to do this.
	WorkGroup signalStatusWorkers(WG_SIGNALS_SIZE, config.totalRuntimeTicks, config.granSignalsTicks);
	Worker::ActionFunction spWork = boost::bind(signal_status_worker, _1, _2);
	signalStatusWorkers.initWorkers(&spWork, nullptr);
	for (size_t i = 0; i < Signal::all_signals_.size(); i++) {
		//add by xuyan
//		if(Signal::all_signals_[i]->isFake)
//			continue;

		signalStatusWorkers.assignAWorker(Signal::all_signals_[i]);
	}

	//Initialize the aura manager
	AuraManager& auraMgr = AuraManager::instance();
	auraMgr.init();

	//Start work groups and all threads.
	agentWorkers.startAll();
	signalStatusWorkers.startAll();

	//
#ifndef SIMMOB_DISABLE_MPI
	if (config.is_run_on_many_computers) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(&agentWorkers, &signalStatusWorkers);

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

	timeval loop_start_time;
	gettimeofday(&loop_start_time, nullptr);
	int loop_start_offset = diff_ms(loop_start_time, start_time);

	for (unsigned int currTick = 0; currTick < config.totalRuntimeTicks; currTick++) {
		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Output
		{
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			cout << "Approximate Tick Boundary: " << currTick << ", "
					<< (currTick * config.baseGranMS) << " ms" << endl;
			if (!warmupDone) {
				cout << "  Warmup; output ignored." << endl;
			}
		}

		//Update the signal logic and plans for every intersection grouped by region
		signalStatusWorkers.wait();

		//Agent-based cycle
		agentWorkers.wait();

#ifndef SIMMOB_DISABLE_MPI
		if (config.is_run_on_many_computers) {
			PartitionManager& partitionImpl = PartitionManager::instance();
			partitionImpl.crossPCBarrier();

			partitionImpl.crossPCboundaryProcess(currTick);

			partitionImpl.crossPCBarrier();

			partitionImpl.outputAllEntities(currTick);
		}
#endif

		auraMgr.update(currTick);
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

	if (!Agent::pending_agents.empty()) {
		cout << "WARNING! There are still " << Agent::pending_agents.size()
				<< " Agents waiting to be scheduled; next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
		if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}
	}

	cout << "Simulation complete; closing worker threads." << endl;
	return true;
}

int main(int argc, char* argv[])
{
	//Save start time
	gettimeofday(&start_time, nullptr);

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
	//Note: Don't chnage this here; change it by supplying an argument on the
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
	if (argc > 2)
	{
		if (!Logger::log_init(argv[2]))
		{
			cout << "Loading output file failed; using cout" << endl;
			cout << argv[2] << endl;
		}
	}
	else
	{
		Logger::log_init("");
		cout << "No output file specified; using cout." << endl;
	}

	//This should be moved later, but we'll likely need to manage random numbers
	//ourselves anyway, to make simulations as repeatable as possible.
	//if (config.is_simulation_repeatable)
	//{
		//TODO: Output the random seed here (and only here)
	//}

	//Perform main loop
	int returnVal = performMain(configFileName) ? 0 : 1;

	//Close log file, return.
	Logger::log_done();
	cout << "Done" << endl;
	return returnVal;
}


bool TestTimeClass()
{
	{ //Ensure nonsense isn't parsed
		try
		{
			DailyTime a("ABCDEFG");
			std::cout << "Nonsensical input test failed.\n";
			return false;
		}
		catch (std::exception& ex)
		{
		}
	}

	{ //Ensure optional seconds can be parsed.
		DailyTime a("08:30:00");
		DailyTime b("08:30");
		if (!a.isEqual(b))
		{
			std::cout << "Optional seconds test failed.\n";
			return false;
		}
	}

	{ //Ensure hours/minutes are mandatory
		try
		{
			DailyTime a("08");
			std::cout << "Non-optional seconds test failed.\n";
			return false;
		}
		catch (std::exception& ex)
		{
		}
	}

	{ //Check time comparison
		DailyTime a("08:30:00");
		DailyTime b("08:30:01");
		if (a.isEqual(b) || a.isAfter(b) || !a.isBefore(b))
		{
			std::cout << "Single second after test failed.\n";
			return false;
		}
	}

	{ //Check time comparison
		DailyTime a("09:30:00");
		DailyTime b("08:30:00");
		if (a.isEqual(b) || !a.isAfter(b) || a.isBefore(b))
		{
			std::cout << "Single hour before test failed.\n";
			return false;
		}
	}

	{ //Check parsing fractions
		DailyTime a("08:30:00.5");
		DailyTime b("08:30:00");
		if (a.isEqual(b) || !a.isAfter(b) || a.isBefore(b))
		{
			std::cout << "Half second before test failed.\n";
			return false;
		}
	}

	{ //Check mandatory hours/minutes
		try
		{
			DailyTime a("08.5");
			std::cout << "Mandatory minutes test 1 failed.\n";
			return false;
		}
		catch (std::exception& ex)
		{
		}
	}

	{ //Check mandatory hours/minutes
		try
		{
			DailyTime a("08:30.5");
			std::cout << "Mandatory minutes test 2 failed.\n";
			return false;
		}
		catch (std::exception& ex)
		{
		}
	}

	{ //Check fraction comparisons
		DailyTime a("08:30:00.3");
		DailyTime b("08:30:00.5");
		if (a.isEqual(b) || a.isAfter(b) || !a.isBefore(b))
		{
			std::cout << "Sub-second comparison test failed.\n";
			return false;
		}
	}

	std::cout << "Warning: No tests were performed on DailyTime with a time_t constructor.\n";
	std::cout << "DailyTime tests passed.\n";
	return true;
}

