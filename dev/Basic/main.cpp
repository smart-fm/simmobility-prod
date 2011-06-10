/**
 * A first approximation of the basic pseudo-code in C++
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

//trivial defined here
bool trivial(unsigned int id) {
	return id%2==0;
}


//First "loading" step is special.
void StepZero(vector<Agent>& agents, vector<Region>& regions, vector<TripChain>& trips,
		      vector<ChoiceSet>& choiceSets, vector<Vehicle>& vehicles);




int main(int argc, char* argv[])
{
  //Some assumed properties
  const unsigned int TOTAL_TIME = 21; //Temp.
  const unsigned int TIME_STEP = 1; //NOTE: Is this correct?
  const unsigned int simulationStartTime = 3; //Temp.

  { //Temporary scope; attempting to figure out why the program hangs after completing.

  //Our work groups
  WorkGroup agentWorkers(WG_AGENTS_SIZE);
  WorkGroup signalStatusWorkers(WG_SIGNALS_SIZE);
  WorkGroup shortestPathWorkers(WG_SHORTEST_PATH_SIZE);

  //Initialization: Scenario definition
  vector<Agent> agents;
  vector<Region> regions;
  vector<TripChain> trips;
  vector<ChoiceSet> choiceSets;
  vector<Vehicle> vehicles;

  //Load our user config file; save a handle to the shared definition of it.
  if (!ConfigParams::InitUserConf(agents, regions, trips, choiceSets, vehicles)) {   //Note: Agent "shells" are loaded here.
	  return 1;
  }
  const ConfigParams& config = ConfigParams::GetInstance();

  //Initialize our work groups, assign agents randomly to these groups.
  agentWorkers.initWorkers<EntityWorker>();
  for (size_t i=0; i<agents.size(); i++) {
	  agentWorkers.migrate(&agents[i], -1, i%WG_AGENTS_SIZE);
  }
  for (size_t i=0; i<agentWorkers.size(); i++) {
	  //TODO: This doesn't take multiple granularities into account. Need to fix.
	  ((EntityWorker&)agentWorkers.getWorker(i)).setSimulationEnd(TOTAL_TIME*TIME_STEP);
  }

  //Initialize our signal status work groups
  //  TODO: There needs to be a more general way to do this.
  signalStatusWorkers.initWorkers<EntityWorker>();
  for (size_t i=0; i<regions.size(); i++) {
	  signalStatusWorkers.migrate(&regions[i], -1, i%WG_SIGNALS_SIZE);
  }
  for (size_t i=0; i<signalStatusWorkers.size(); i++) {
	  ((EntityWorker&)signalStatusWorkers.getWorker(i)).setSimulationEnd(TOTAL_TIME*TIME_STEP);
  }

  //Initialize our shortest path work groups
  //  TODO: There needs to be a more general way to do this.
  shortestPathWorkers.initWorkers<ShortestPathWorker>();
  for (size_t i=0; i<agents.size(); i++) {
	  shortestPathWorkers.migrate(&agents[i], -1, i%WG_SHORTEST_PATH_SIZE);
  }
  for (size_t i=0; i<shortestPathWorkers.size(); i++) {
	  ((EntityWorker&)shortestPathWorkers.getWorker(i)).setSimulationEnd((TOTAL_TIME*TIME_STEP)/shortestPathLoopTimeStep);
  }

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
  StepZero(agents, regions, trips, choiceSets, vehicles);
  cout <<"  " <<"Initialization done" <<endl;

  //Sanity check (simple)
  if (!checkIDs(agents, trips, choiceSets, vehicles)) {
	  return 1;
  }

  //Output
  cout <<"  " <<"(Sanity Check Passed)" <<endl;

  //Start work groups
  agentWorkers.startAll();
  signalStatusWorkers.startAll();
  shortestPathWorkers.startAll();


  //Time-based cycle.
  for (unsigned int currTime=1; currTime<TOTAL_TIME; currTime+=TIME_STEP) {
	  //Output
	  cout <<"Time " <<currTime <<endl;

	  //Update the signal logic and plans for every intersection grouped by region
	  signalStatusWorkers.wait();

	  //Update weather, traffic conditions, etc.
	  updateTrafficInfo(regions);

	  //NOTE:
	  //  The "shortestPathLoopTimeStep for loop" and others are unclear to me.
	  //  For now, I am just performing their tasks if the currTime is evenly
	  //  divisible by the time-tick for that loop.

	  //Longer Time-based cycle
	  if (currTime%shortestPathLoopTimeStep == 0) {
		  shortestPathWorkers.wait();
		  cout <<"  " <<"Longer-time cycle" <<endl;
	  }

	  //Longer Time-based cycle
	  if (currTime%agentDecompositionTimeStep == 0) {
		  //Thread controller / processor affinity / Load Balancer
		  agentDecomposition(agents);

		  //One Queue is created for each core
		  updateVehicleQueue(vehicles);

		  cout <<"  " <<"Longer-time cycle" <<endl;
	  }

	  //Agent-based cycle
	  if (true) { //Seems to operate every time step?
		  agentWorkers.wait();
	  }

	  //Surveillance update
	  updateSurveillanceData(agents);

	  //Check if the warmup period has ended.
	  if (currTime >= simulationStartTime) {
		  updateGUI(agents);
		  saveStatistics(agents);
	  } else {
		  cout <<"  " <<"(Warmup, output ignored)" <<endl;
	  }

	  //Longer Time-based cycle
	  if (currTime%objectMgmtTimeStep == 0) {
		  saveStatisticsToDB(agents);

		  cout <<"  " <<"Statistics saved" <<endl;
	  }
  }

  cout <<"Timesteps complete; closing worker threads." <<endl;

  } //End of temporary scope


  cout <<"Done" <<endl;

  return 0;
}



//Time tick zero is essentially a parallelized "initialization" step. Leaving in Main for now...
void StepZero(vector<Agent>& agents, vector<Region>& regions, vector<TripChain>& trips,
	      vector<ChoiceSet>& choiceSets, vector<Vehicle>& vehicles)
{
	  //Our work groups. Will be disposed after this time tick.
	  WorkGroup tripChainWorkers(WG_TRIPCHAINS_SIZE);
	  WorkGroup createAgentWorkers(WG_CREATE_AGENT_SIZE);
	  WorkGroup choiceSetWorkers(WG_CHOICESET_SIZE);
	  WorkGroup vehicleWorkers(WG_VEHICLES_SIZE);

	  //Create object from DB; for long time spans objects must be created on demand.
	  boost::function<void (Worker*)> func1 = boost::bind(load_trip_chain, _1);
	  tripChainWorkers.initWorkers<Worker>(&func1);
	  for (size_t i=0; i<trips.size(); i++) {
		  tripChainWorkers.migrate(&trips[i], -1, i%WG_TRIPCHAINS_SIZE);
	  }

	  //Agents, choice sets, and vehicles
	  boost::function<void (Worker*)> func2 = boost::bind(load_agents, _1);
	  createAgentWorkers.initWorkers<Worker>(&func2);
	  for (size_t i=0; i<agents.size(); i++) {
		  createAgentWorkers.migrate(&agents[i], -1, i%WG_CREATE_AGENT_SIZE);
	  }
	  boost::function<void (Worker*)> func3 = boost::bind(load_choice_sets, _1);
	  choiceSetWorkers.initWorkers<Worker>(&func3);
	  for (size_t i=0; i<choiceSets.size(); i++) {
		  choiceSetWorkers.migrate(&choiceSets[i], -1, i%WG_CHOICESET_SIZE);
	  }
	  boost::function<void (Worker*)> func4 = boost::bind(load_vehicles, _1);
	  vehicleWorkers.initWorkers<Worker>(&func4);
	  for (size_t i=0; i<vehicles.size(); i++) {
		  vehicleWorkers.migrate(&vehicles[i], -1, i%WG_VEHICLES_SIZE);
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







