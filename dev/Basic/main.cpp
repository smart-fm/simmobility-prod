/**
 * A first approximation of the basic pseudo-code in C++
 */
#include <iostream>
#include <vector>
#include <boost/thread.hpp>

#include "simple_classes.h"
#include "constants.h"
#include "stubs.h"
#include "workers.h"

#include "Worker.hpp"
#include "WorkGroup.hpp"

using std::cout;
using std::endl;
using std::vector;
using boost::thread;


//NOTE: boost::thread and std::thread have a few minor differences. Boost::threads appear to
//      "join" automatically on destruction, and std::threads don't. For now, I explicitly
//      forced threads to "join".
int main(int argc, char* argv[])
{
  //Our work groups
  WorkGroup agentWorkers(WG_AGENTS_SIZE);

  //Initialization: Scenario definition
  vector<Agent> agents;
  vector<Region> regions;
  vector<TripChain> trips;
  vector<ChoiceSet> choiceSets;
  vector<Vehicle> vehicles;
  loadUserConf(agents, regions);   //Note: Agent "shells" are loaded here.

  //Initialize our work groups
  for (size_t i=0; i<WG_AGENTS_SIZE; i++)
	  agentWorkers.initWorker();

  //Assign agents randomly to a work group
  for (size_t i=0; i<agents.size() i++) {
	  agentWorkers.migrate(&agents.get(i), -1, i%WG_AGENTS_SIZE);
  }



  //Initialization: Server configuration
  setConfiguration();

  //Initialization: Network decomposition among multiple machines.
  loadNetwork();

  //Time-based cycle.
  const unsigned int TOTAL_TIME = 21; //Temp.
  //const unsigned int TIME_STEP = std::min(std::min(objectMgmtTimeStep, agentDecompositionTimeStep), shortestPathLoopTimeStep);
  const unsigned int TIME_STEP = 1; //NOTE: Is this correct?
  const unsigned int simulationStartTime = 3; //Temp.
  for (unsigned int currTime=0; currTime<TOTAL_TIME; currTime+=TIME_STEP) {
	  //Output
	  cout <<"Time " <<currTime <<endl;

	  //NOTE:
	  //  This is supposed to be the "objectMgmtTimeStep for loop", but I am
	  //  not sure exactly how to wrap this in a loop. For now, I just checked if this
	  //  is the first time step
	  if (currTime==0) {
		  //Create object from DB; for long time spans objects must be created on demand.
		  loadTripChains(agents, trips);

		  //Agents, choice sets, and vehicles
		  loadAgentsChoiceSetsAndVehicles(agents, choiceSets, vehicles);

		  //Sanity check (simple)
		  if (!checkIDs(agents, trips, choiceSets, vehicles))
			  return 1;

		  //Output
		  cout <<"  " <<"Initialization done" <<endl;
	  }

	  //Update the signal logic and plans for every intersection grouped by region
	  updateSignalStatus(regions);

	  //Update weather, traffic conditions, etc.
	  updateTrafficInfo(regions);

	  //NOTE:
	  //  The "shortestPathLoopTimeStep for loop" and others are unclear to me.
	  //  For now, I am just performing their tasks if the currTime is evenly
	  //  divisible by the time-tick for that loop.

	  //Longer Time-based cycle
	  if (currTime%shortestPathLoopTimeStep == 0) {
		  calculateTimeDependentShortestPath(agents);

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
		  updateAndAdvancePhase(agents);
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

  cout <<"Done" <<endl;

  return 0;
}



