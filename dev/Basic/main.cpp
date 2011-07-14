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

#include "simple_classes.h"
#include "constants.h"
#include "stubs.h"

#include "workers/Worker.hpp"
#include "workers/EntityWorker.hpp"
#include "workers/ShortestPathWorker.hpp"
#include "WorkGroup.hpp"

#include "conf/simpleconf.hpp"


using std::cout;
using std::endl;
using std::vector;
using boost::thread;


using namespace sim_mob;


//Helper
typedef WorkGroup<Entity> EntityWorkGroup;


//trivial defined here
bool trivial(unsigned int id) {
	return id%2==0;
}


/**
 * First "loading" step is special. Initialize all agents using work groups in parallel.
 */
void InitializeAll(vector<Agent*>& agents, vector<Region*>& regions, vector<TripChain*>& trips,
		      vector<ChoiceSet*>& choiceSets, vector<Vehicle*>& vehicles);



//TEST
void entity_worker(sim_mob::Worker<sim_mob::Entity>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		(*it)->update(frameNumber);
	}
}
void signal_status_worker(sim_mob::Worker<sim_mob::Entity>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Entity*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		//((Region*)(*it))->updateSignalStatus(); //TODO
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
bool performMain()
{
  //Initialization: Scenario definition
  vector<Agent*>& agents = Agent::all_agents;
  vector<Region*> regions;
  vector<TripChain*> trips;
  vector<ChoiceSet*> choiceSets;
  vector<Vehicle*> vehicles;

  //Load our user config file; save a handle to the shared definition of it.
  if (!ConfigParams::InitUserConf(agents, regions, trips, choiceSets, vehicles)) {   //Note: Agent "shells" are loaded here.
	  return false;
  }
  const ConfigParams& config = ConfigParams::GetInstance();

  //Initialization: Server configuration
  setConfiguration();

  //Initialization: Network decomposition among multiple machines.
  loadNetwork();



  ///////////////////////////////////////////////////////////////////////////////////
  // NOTE: Because of the way we cache the old values of agents, we need to run our
  //       initialization workers and then flip their values (otherwise there will be
  //       no data to read.) The other option is to load all "properties" with a default
  //       value, but at the moment we don't even have a "properties class"
  ///////////////////////////////////////////////////////////////////////////////////
  cout <<"Beginning Initialization" <<endl;
  InitializeAll(agents, regions, trips, choiceSets, vehicles);
  cout <<"  " <<"Initialization done" <<endl;

  //Sanity check (simple)
  if (!checkIDs(agents, trips, choiceSets, vehicles)) {
	  return false;
  }

  //Output
  cout <<"  " <<"(Sanity Check Passed)" <<endl;

  //Initialize our work groups, assign agents randomly to these groups.
  EntityWorkGroup agentWorkers(WG_AGENTS_SIZE, config.totalRuntimeTicks, config.granAgentsTicks);
  Worker<sim_mob::Entity>::actionFunction entityWork = boost::bind(entity_worker, _1, _2);
  agentWorkers.initWorkers(&entityWork);
  for (size_t i=0; i<agents.size(); i++) {
	  agentWorkers.migrate(agents[i], -1, i%WG_AGENTS_SIZE);
  }

  //Initialize our signal status work groups
  //  TODO: There needs to be a more general way to do this.
  EntityWorkGroup signalStatusWorkers(WG_SIGNALS_SIZE, config.totalRuntimeTicks, config.granSignalsTicks);
  Worker<sim_mob::Entity>::actionFunction spWork = boost::bind(signal_status_worker, _1, _2);
  signalStatusWorkers.initWorkers(&spWork);
  for (size_t i=0; i<regions.size(); i++) {
	  signalStatusWorkers.migrate(regions[i], -1, i%WG_SIGNALS_SIZE);
  }

  //Initialize our shortest path work groups
  //  TODO: There needs to be a more general way to do this.
  //EntityWorkGroup shortestPathWorkers(WG_SHORTEST_PATH_SIZE, config.totalRuntimeTicks, config.granPathsTicks);
  //shortestPathWorkers.initWorkers();
  /////////////////////////////////////////////////////////////////////////////
  // NOTE: Currently, an Agent can only be "managed" by one Worker. We need a way to
  //       say that the agent is the data object of a worker, but WON'T be managed by it.
  // For example, shortest-path-worker only needs to read X/Y, but will update "shortestPath"
  //        (which the EntityWorker won't touch).
  /////////////////////////////////////////////////////////////////////////////
  /*for (size_t i=0; i<agents.size(); i++) {
	  shortestPathWorkers.migrate(&agents[i], -1, i%WG_SHORTEST_PATH_SIZE);
  }*/

  //Start work groups
  agentWorkers.startAll();
  signalStatusWorkers.startAll();
  //shortestPathWorkers.startAll();


  /////////////////////////////////////////////////////////////////
  // NOTE: WorkGroups are able to handle skipping steps by themselves.
  //       So, we simply call "wait()" on every tick, and on non-divisible
  //       time ticks, the WorkGroups will return without performing
  //       a barrier sync.
  /////////////////////////////////////////////////////////////////
  for (unsigned int currTick=0; currTick<config.totalRuntimeTicks; currTick++) {
	  //Output
	  {
		boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
	    cout <<"Tick " <<currTick <<", " <<(currTick*config.baseGranMS) <<" ms" <<endl;
	    cout <<"  NOTE: Ticks provide only a rough guideline when using cout; do not expect all agents to finish exactly when this tick is displayed." <<endl;
	  }

	  //Update the signal logic and plans for every intersection grouped by region
	  signalStatusWorkers.wait();

	  //Update weather, traffic conditions, etc.
	  updateTrafficInfo(regions);

	  //Longer Time-based cycle
	  //shortestPathWorkers.wait();

	  //Longer Time-based cycle
	  //TODO: Put these on Worker threads too.
	  agentDecomposition(agents);

	  //One Queue is created for each core
	  updateVehicleQueue(vehicles);

	  //Agent-based cycle
	  agentWorkers.wait();

	  //Surveillance update
	  updateSurveillanceData(agents);

	  //Check if the warmup period has ended.
	  if (currTick >= config.totalWarmupTicks) {
		  updateGUI(agents);
		  saveStatistics(agents);
	  } else {
		  {
			boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
		    cout <<"  " <<"(Warmup, output ignored)" <<endl;
		  }
	  }

	  saveStatisticsToDB(agents);
  }

  cout <<"Simulation complete; closing worker threads." <<endl;
  return true;
}



int main(int argc, char* argv[])
{
  int returnVal = performMain() ? 0 : 1;

  cout <<"Done" <<endl;

  return returnVal;
}



/**
 * Parallel initialization step.
 */
void InitializeAll(vector<Agent*>& agents, vector<Region*>& regions, vector<TripChain*>& trips,
	      vector<ChoiceSet*>& choiceSets, vector<Vehicle*>& vehicles)
{
	  //Our work groups. Will be disposed after this time tick.
	  WorkGroup<TripChain> tripChainWorkers(WG_TRIPCHAINS_SIZE, 1);
	  WorkGroup<sim_mob::Agent> createAgentWorkers(WG_CREATE_AGENT_SIZE, 1);
	  WorkGroup<ChoiceSet> choiceSetWorkers(WG_CHOICESET_SIZE, 1);
	  WorkGroup<Vehicle> vehicleWorkers(WG_VEHICLES_SIZE, 1);

	  //Create object from DB; for long time spans objects must be created on demand.
	  Worker<TripChain>::actionFunction func1 = boost::bind(load_trip_chain, _1, _2);
	  tripChainWorkers.initWorkers(&func1);
	  for (size_t i=0; i<trips.size(); i++) {
		  tripChainWorkers.migrate(trips[i], -1, i%WG_TRIPCHAINS_SIZE);
	  }

	  //Agents, choice sets, and vehicles
	  Worker<sim_mob::Agent>::actionFunction func2 = boost::bind(load_agents, _1, _2);
	  createAgentWorkers.initWorkers(&func2);
	  for (size_t i=0; i<agents.size(); i++) {
		  createAgentWorkers.migrate(agents[i], -1, i%WG_CREATE_AGENT_SIZE);
	  }
	  Worker<ChoiceSet>::actionFunction func3 = boost::bind(load_choice_sets, _1, _2);
	  choiceSetWorkers.initWorkers(&func3);
	  for (size_t i=0; i<choiceSets.size(); i++) {
		  choiceSetWorkers.migrate(choiceSets[i], -1, i%WG_CHOICESET_SIZE);
	  }
	  Worker<Vehicle>::actionFunction func4 = boost::bind(load_vehicles, _1, _2);
	  vehicleWorkers.initWorkers(&func4);
	  for (size_t i=0; i<vehicles.size(); i++) {
		  vehicleWorkers.migrate(vehicles[i], -1, i%WG_VEHICLES_SIZE);
	  }

	  //Start
	  cout <<"  Starting threads..." <<endl;
	  tripChainWorkers.startAll();
	  createAgentWorkers.startAll();
	  choiceSetWorkers.startAll();
	  vehicleWorkers.startAll();

	  //Flip once
	  tripChainWorkers.wait();
	  createAgentWorkers.wait();
	  choiceSetWorkers.wait();
	  vehicleWorkers.wait();

	  cout <<"  Closing all work groups..." <<endl;
}




