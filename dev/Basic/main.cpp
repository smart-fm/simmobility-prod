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

#include "constants.h"

#include "workers/Worker.hpp"
#include "workers/EntityWorker.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "WorkGroup.hpp"
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
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Route.hpp"
#include "geospatial/BusRoute.hpp"
#include "perception/FixedDelayed.hpp"


using std::cout;
using std::endl;
using std::vector;
using boost::thread;


using namespace sim_mob;


//Function prototypes.
void InitializeAllAgentsAndAssignToWorkgroups(vector<Agent*>& agents);
bool CheckAgentIDs(const std::vector<sim_mob::Agent*>& agents);
bool TestTimeClass();



///Worker function for entity-related loading tasks.
void entity_worker(sim_mob::Worker<sim_mob::Entity>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		if (!(*it)->update(frameNumber)) {
			//This Entity is done; schedule for deletion.
			wk.scheduleForRemoval(*it);
		}
	}
}

///Worker function for signal status loading task.
void signal_status_worker(sim_mob::Worker<sim_mob::Entity>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
            (*it)->update(frameNumber);
	}
}

///Worker function for loading agents.
void load_agents(sim_mob::Worker<sim_mob::Agent>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Agent*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		trivial((*it)->getId());
	}
}





/**
 * Main simulation loop.
 * \note
 * For doxygen, I'd like to have the variable JAVADOC AUTOBRIEF set to "true"
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
bool performMain(const std::string& configFileName)
{
  //Initialization: Scenario definition
  vector<Agent*>& agents = Agent::all_agents;

  //Load our user config file; save a handle to the shared definition of it.
  if (!ConfigParams::InitUserConf(configFileName, agents)) {   //Note: Agent "shells" are loaded here.
	  return false;
  }
  const ConfigParams& config = ConfigParams::GetInstance();

  //Initialization: Server configuration
  //setConfiguration();  //NOTE: This is done within InitUserConf().

  //Initialization: Network decomposition among multiple machines.
  //loadNetwork();   //NOTE: This will occur within the partition manager.



  ///////////////////////////////////////////////////////////////////////////////////
  // NOTE: Because of the way we cache the old values of agents, we need to run our
  //       initialization workers and then flip their values (otherwise there will be
  //       no data to read.) The other option is to load all "properties" with a default
  //       value, but at the moment we don't even have a "properties class"
  ///////////////////////////////////////////////////////////////////////////////////
  cout <<"Beginning Initialization" <<endl;
  InitializeAllAgentsAndAssignToWorkgroups(agents);
  cout <<"  " <<"Initialization done" <<endl;

  //Sanity check (simple)
  if (!CheckAgentIDs(agents /*trips,*/ /*choiceSets*/)) {
	  return false;
  }

  //Sanity check (nullptr)
  void* x = nullptr;
  if (x) {
	  return false;
  }

  //Sanity check (time class)
  if (!TestTimeClass()) {
	  std::cout <<"Aborting: Time class tests failed.\n";
	  return false;
  }


  //Output
  cout <<"  " <<"...Sanity Check Passed" <<endl;


  //Initialize our work groups, assign agents randomly to these groups.
  WorkGroup agentWorkers(WG_AGENTS_SIZE, config.totalRuntimeTicks, config.granAgentsTicks, true);
  Agent::TMP_AgentWorkGroup = &agentWorkers;
  Worker<sim_mob::Entity>::ActionFunction entityWork = boost::bind(entity_worker, _1, _2);
  agentWorkers.initWorkers(&entityWork);
  for (size_t i=0; i<agents.size(); i++) {
	  agentWorkers.migrate(agents[i], i%WG_AGENTS_SIZE);
  }

  //Initialize our signal status work groups
  //  TODO: There needs to be a more general way to do this.
  WorkGroup signalStatusWorkers(WG_SIGNALS_SIZE, config.totalRuntimeTicks, config.granSignalsTicks);
  Worker<sim_mob::Entity>::ActionFunction spWork = boost::bind(signal_status_worker, _1, _2);
  signalStatusWorkers.initWorkers(&spWork);
  for (size_t i=0; i<Signal::all_signals_.size(); i++) {
	  signalStatusWorkers.migrate(const_cast<Signal*>(Signal::all_signals_[i]), i%WG_SIGNALS_SIZE);
  }

  //Start work groups
  agentWorkers.startAll();
  signalStatusWorkers.startAll();
  //shortestPathWorkers.startAll();

  AuraManager& auraMgr = AuraManager::instance();
  auraMgr.init();

  /////////////////////////////////////////////////////////////////
  // NOTE: WorkGroups are able to handle skipping steps by themselves.
  //       So, we simply call "wait()" on every tick, and on non-divisible
  //       time ticks, the WorkGroups will return without performing
  //       a barrier sync.
  /////////////////////////////////////////////////////////////////
  size_t numStartAgents = Agent::all_agents.size();
  for (unsigned int currTick=0; currTick<config.totalRuntimeTicks; currTick++) {
	  //Flag
	  bool warmupDone = (currTick >= config.totalWarmupTicks);

	  //Output
	  {
		  boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
          cout <<"Approximate Tick Boundary: " <<currTick <<", " <<(currTick*config.baseGranMS) <<" ms" <<endl;
		  if (!warmupDone) {
			  cout <<"  Warmup; output ignored." <<endl;
		  }
	  }

	  //Update the signal logic and plans for every intersection grouped by region
	  signalStatusWorkers.wait();

	  //Update weather, traffic conditions, etc.
	  //updateTrafficInfo(regions);

	  //Longer Time-based cycle
	  //shortestPathWorkers.wait();

	  //Longer Time-based cycle
	  //agentDecomposition(agents);  //NOTE: This should be performed by some other Agent on some kind of worker thread.

	  //Agent-based cycle
	  agentWorkers.wait();

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

  cout <<"Starting Agents: " <<numStartAgents <<endl;
  if (Agent::all_agents.empty()) {
	  cout <<"All Agents have left the simulation.\n";
  } else {
	  size_t numPerson = 0;
	  for (vector<Agent*>::iterator it=Agent::all_agents.begin(); it!=Agent::all_agents.end(); it++) {
		  if (dynamic_cast<Person*>(*it)) {
			  numPerson++;
		  }
	  }
	  cout <<"Remaining Agents: " <<numPerson <<" (Person)   " <<(Agent::all_agents.size()-numPerson) <<" (Other)" <<endl;
  }

  cout <<"Simulation complete; closing worker threads." <<endl;
  return true;
}



int main(int argc, char* argv[])
{
	//Argument 1: Config file
	//Note: Don't chnage this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/config.xml";
	if (argc>1) {
		configFileName = argv[1];
	} else {
		cout <<"No config file specified; using default." <<endl;
	}
	cout <<"Using config file: " <<configFileName <<endl;

	//Argument 2: Log file
	if (argc>2) {
		if (!Logger::log_init(argv[2])) {
			cout <<"Loading output file failed; using cout" <<endl;
			cout <<argv[2] <<endl;
		}
	} else {
		Logger::log_init("");
		cout <<"No output file specified; using cout." <<endl;
	}

	//This should be moved later, but we'll likely need to manage random numbers
	//ourselves anyway, to make simulations as repeatable as possible.
	time_t t = time(NULL);
	srand (t);
	cout <<"Random Seed Init: " <<t <<endl;

	//Perform main loop
	int returnVal = performMain(configFileName) ? 0 : 1;

	//Close log file, return.
	Logger::log_done();
	cout <<"Done" <<endl;
	return returnVal;
}



/**
 * Parallel initialization step. Note that this function was created very early in development,
 *   and will eventually have to be migrated to the dispatcher.
 */
void InitializeAllAgentsAndAssignToWorkgroups(vector<Agent*>& agents)
{
	  //Our work groups. Will be disposed after this time tick.
	  SimpleWorkGroup<sim_mob::Agent> createAgentWorkers(WG_CREATE_AGENT_SIZE, 1);

	  //Create agents
	  Worker<sim_mob::Agent>::ActionFunction func2 = boost::bind(load_agents, _1, _2);
	  createAgentWorkers.initWorkers(&func2);
	  for (size_t i=0; i<agents.size(); i++) {
		  createAgentWorkers.migrate(agents[i], createAgentWorkers.getWorker(-1), createAgentWorkers.getWorker(i%WG_CREATE_AGENT_SIZE));
	  }

	  //Start
	  cout <<"  Starting threads..." <<endl;
	  createAgentWorkers.startAll();

	  //Flip once
	  createAgentWorkers.wait();

	  cout <<"  Closing all work groups..." <<endl;
}



/**
 * Simple sanity check on Agent IDs. Checks that IDs start at 0, end at size(agents)-1,
 *   and contain every value in between. Order is not important.
 */
bool CheckAgentIDs(const std::vector<sim_mob::Agent*>& agents) {
    std::vector<sim_mob::Agent const *> all_agents(agents.begin(), agents.end());
    std::copy(Signal::all_signals_.begin(), Signal::all_signals_.end(), std::back_inserter(all_agents));

	std::set<int> agent_ids;
	bool foundZero = false;
	bool foundMax = false;
	for (size_t i=0; i<all_agents.size(); i++) {
		int id = all_agents[i]->getId();
		agent_ids.insert(id);
		if (id==0) {
			foundZero = true;
		}
		if (id+1==static_cast<int>(all_agents.size())) {
			foundMax = true;
		}
	}
	if (all_agents.size()!=agent_ids.size() || !foundZero || !foundMax) {
		std::cout <<"Error, invalid Agent ID: agent_size(" <<all_agents.size() <<"=>" <<agent_ids.size() <<"), "
				<<"foundZero: " <<foundZero <<", foundMax: " <<foundMax <<std::endl;
		return false;
	}

	return true;
}


bool TestTimeClass()
{
	{ //Ensure nonsense isn't parsed
	try {
		DailyTime a("ABCDEFG");
		std::cout <<"Nonsensical input test failed.\n";
		return false;
	} catch (std::exception& ex) {}
	}

	{ //Ensure optional seconds can be parsed.
	DailyTime a("08:30:00");
	DailyTime b("08:30");
	if (!a.isEqual(b)) {
		std::cout <<"Optional seconds test failed.\n";
		return false;
	}
	}

	{ //Ensure hours/minutes are mandatory
	try {
		DailyTime a("08");
		std::cout <<"Non-optional seconds test failed.\n";
		return false;
	} catch (std::exception& ex) {}
	}

	{ //Check time comparison
	DailyTime a("08:30:00");
	DailyTime b("08:30:01");
	if (a.isEqual(b) || a.isAfter(b) || !a.isBefore(b)) {
		std::cout <<"Single second after test failed.\n";
		return false;
	}
	}

	{ //Check time comparison
	DailyTime a("09:30:00");
	DailyTime b("08:30:00");
	if (a.isEqual(b) || !a.isAfter(b) || a.isBefore(b)) {
		std::cout <<"Single hour before test failed.\n";
		return false;
	}
	}

	{ //Check parsing fractions
	DailyTime a("08:30:00.5");
	DailyTime b("08:30:00");
	if (a.isEqual(b) || !a.isAfter(b) || a.isBefore(b)) {
		std::cout <<"Half second before test failed.\n";
		return false;
	}
	}

	{ //Check mandatory hours/minutes
	try {
		DailyTime a("08.5");
		std::cout <<"Mandatory minutes test 1 failed.\n";
		return false;
	} catch (std::exception& ex) {}
	}

	{ //Check mandatory hours/minutes
	try {
		DailyTime a("08:30.5");
		std::cout <<"Mandatory minutes test 2 failed.\n";
		return false;
	} catch (std::exception& ex) {}
	}

	{ //Check fraction comparisons
	DailyTime a("08:30:00.3");
	DailyTime b("08:30:00.5");
	if (a.isEqual(b) || a.isAfter(b) || !a.isBefore(b)) {
		std::cout <<"Sub-second comparison test failed.\n";
		return false;
	}
	}

	std::cout <<"Warning: No tests were performed on DailyTime with a time_t constructor.\n";
	std::cout <<"DailyTime tests passed.\n";
	return true;
}



