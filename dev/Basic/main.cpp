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


using std::cout;
using std::endl;
using std::vector;
using boost::thread;


using namespace sim_mob;


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
#ifndef DISABLE_DYNAMIC_DISPATCH
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
  //Loader params for our Agents
  WorkGroup::EntityLoadParams entLoader(Agent::pending_agents.impl, Agent::all_agents, Agent::all_agents_lock);

  //Initialization: Scenario definition
  vector<Entity*>& agents = Agent::all_agents;

  //Load our user config file; save a handle to the shared definition of it.
  if (!ConfigParams::InitUserConf(configFileName, agents)) {   //Note: Agent "shells" are loaded here.
	  return false;
  }
  const ConfigParams& config = ConfigParams::GetInstance();


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


  //Initialize our work groups.
  WorkGroup agentWorkers(WG_AGENTS_SIZE, config.totalRuntimeTicks, config.granAgentsTicks, true);
  //Agent::TMP_AgentWorkGroup = &agentWorkers;
  Worker::ActionFunction entityWork = boost::bind(entity_worker, _1, _2);
  agentWorkers.initWorkers(&entityWork, &entLoader);

  //Migrate all Agents from the all_agents array to the pending_agents priority queue unless they are
  // actually starting at time tick zero.
  vector<Entity*> starting_agents;
  for (vector<Entity*>::iterator it=agents.begin(); it!=agents.end(); it++) {
	  Entity* const ag = *it;
#ifndef DISABLE_DYNAMIC_DISPATCH
	  if (ag->getStartTime()==0) {
#endif
		  //Only agents with a start time of zero should start immediately in the all_agents list.
		  agentWorkers.assignAWorker(ag);
		  starting_agents.push_back(ag);
#ifndef DISABLE_DYNAMIC_DISPATCH
	  } else {
		  //Start later.
		  Agent::pending_agents.impl.push(ag);
	  }
#endif
  }
  agents.clear();
  agents.insert(agents.end(), starting_agents.begin(), starting_agents.end());

  cout <<"Initial Agents dispatched or pushed to pending." <<endl;

  //Initialize our signal status work groups
  //  TODO: There needs to be a more general way to do this.
  WorkGroup signalStatusWorkers(WG_SIGNALS_SIZE, config.totalRuntimeTicks, config.granSignalsTicks);
  Worker::ActionFunction spWork = boost::bind(signal_status_worker, _1, _2);
  signalStatusWorkers.initWorkers(&spWork, nullptr);
  for (size_t i=0; i<Signal::all_signals_.size(); i++) {
	  signalStatusWorkers.assignAWorker(Signal::all_signals_[i]);
  }

  //Initialize the aura manager
  AuraManager& auraMgr = AuraManager::instance();
  auraMgr.init();

  //Start work groups and all threads.
  agentWorkers.startAll();
  signalStatusWorkers.startAll();

  /////////////////////////////////////////////////////////////////
  // NOTE: WorkGroups are able to handle skipping steps by themselves.
  //       So, we simply call "wait()" on every tick, and on non-divisible
  //       time ticks, the WorkGroups will return without performing
  //       a barrier sync.
  /////////////////////////////////////////////////////////////////
  size_t numStartAgents = Agent::all_agents.size();

#ifndef DISABLE_DYNAMIC_DISPATCH
  size_t numPendingAgents = Agent::pending_agents.impl.size();
#endif

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

  cout <<"Starting Agents: " <<numStartAgents;
#ifndef DISABLE_DYNAMIC_DISPATCH
  cout <<",     Pending: " <<numPendingAgents;
#endif
  cout <<endl;

  if (Agent::all_agents.empty()) {
	  cout <<"All Agents have left the simulation.\n";
  } else {
	  size_t numPerson = 0;
	  size_t numDriver = 0;
	  size_t numPedestrian = 0;
	  for (vector<Entity*>::iterator it=Agent::all_agents.begin(); it!=Agent::all_agents.end(); it++) {
		  Person* p = dynamic_cast<Person*>(*it);
		  if (p) {
			  numPerson++;
			  if (p->getRole() && dynamic_cast<Driver*>(p->getRole())) {
				  numDriver++;
			  }
			  if (p->getRole() && dynamic_cast<Pedestrian*>(p->getRole())) {
				  numPedestrian++;
			  }
		  }
	  }
	  cout <<"Remaining Agents: " <<numPerson <<" (Person)   " <<(Agent::all_agents.size()-numPerson) <<" (Other)" <<endl;
	  cout <<"   Person Agents: " <<numDriver <<" (Driver)   " <<numPedestrian <<" (Pedestrian)   " <<(numPerson-numDriver-numPedestrian) <<" (Other)" <<endl;
  }

#ifndef DISABLE_DYNAMIC_DISPATCH
  if (!Agent::pending_agents.impl.empty()) {
	  cout <<"WARNING! There are still " <<Agent::pending_agents.impl.size() <<" Agents waiting to be scheduled; next start time is: " <<Agent::pending_agents.impl.top()->getStartTime() <<" ms\n";
  }
#endif

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



