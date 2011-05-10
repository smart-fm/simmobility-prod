#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

using std::cout;
using std::endl;
using std::vector;
using boost::thread;
using boost::worker;

/**
 * A first approximation of the basic pseudo-code in C++
 */


//Constant parameters
const unsigned int shortestPathLoopTimeStep   =    10;
const unsigned int agentDecompositionTimeStep =   100;
const unsigned int objectMgmtTimeStep         =  1000;


//Class stubs
struct Signal {
	unsigned int id;
	Signal(unsigned int id=0) : id(id) {}
};
struct Region {
	unsigned int id;
	vector<Signal> signals;

	Region(unsigned int id=0) : id(id) {
		for (size_t i=id*3; i<id*3+3; i++) {
			signals.push_back(Signal(i));
		}
	}
};
struct Agent {
	unsigned int id;
	Agent(unsigned int id=0) : id(id) {}
};
struct TripChain {
	unsigned int id;
	TripChain(unsigned int id=0) : id(id) {}
};
struct ChoiceSet {
	unsigned int id;
	ChoiceSet(unsigned int id=0) : id(id) {}
};
struct Vehicle {
	unsigned int id;
	Vehicle(unsigned int id=0) : id(id) {}
};



//Function stubs
void loadUserConf(vector<Agent>& agents, vector<Region>& regions) {
	for (size_t i=0; i<20; i++)
		agents.push_back(Agent(i));
	for (size_t i=0; i<5; i++) {
		regions.push_back(Region(i));
	cout <<"Configuration file loaded." <<endl;
}
void setConfiguration() {
	cout <<"Server is configured." <<endl;
}
void loadNetwork() {
	cout <<"Network has been loaded." <<endl;
}
void loadSingleTripChain(const Agent& ag, TripChain& tc) {
	tc.id = ag.id;
}
void createSingleAgent(const Agent& ag) {
	ag.id = ag.id; //Trivial. Presumably, we'd set an agent's other properties here.
}
void createSingleChoiceSet(const ChoiceSet& cs, unsigned int newID) {
	cs.id = newID;
}
void createSingleVehicle(const Vehicle& v, unsigned int newID) {
	v.id = newID;
}
void updateSingleRegionSignals(Region& r) {
	for (size_t i=0; i<r.signals.size(); i++) {
		r.signals[i].id = r.signals[i].id; //Trivial. Will update signal's other properties.
	}
}
void updateSingleShortestPath(Agent& a) {
	a.id = a.id; //Trivial. Will update shortest path later.
}


//Load trip chains in parallel
void loadTripChains(const vector<Agent>& agents, vector<TripChain>& trips)
{
	//Fill with empty objects.
	trips.resize(agents.size());

	//Make workers. When they go out of scope, they will "join" automatically.
	vector<worker> workers;
	for (size_t i=0; i<agents.size(); i++) {
		worker w = boost::bind(&loadSingleTripChain, agents[i], boost::ref(trips[i]));
		workers.push_back(w);
	}
}


//Load agents, choice sets, and vehicles in parallel
void loadAgentsChoiceSetsAndVehicles(vector<Agent>& agents, vector<ChoiceSet>& choiceSets, vector<Vehicle>& vehicles) {
	//Fill with empty objects.
	choiceSets.resize(agents.size());
	vehicles.resize(agents.size());    //Note: The number of vehicles is just assumed to be the number of agents for now.

	//Make workers. When they go out of scope, they will "join" automatically.
	vector<worker> workers;

	//Create all agents
	for (size_t i=0; i<agents.size(); i++) {
		worker w = boost::bind(&createSingleAgent, boost::ref(agents[i]));
		workers.push_back(w);
	}

	//Create all choice sets
	for (size_t i=0; i<choiceSets.size(); i++) {
		worker w = boost::bind(&createSingleChoiceSet, boost::ref(choiceSets[i]), i);
		workers.push_back(w);
	}

	//Create all vehicles
	for (size_t i=0; i<vehicles.size(); i++) {
		worker w = boost::bind(&createSingleVehicle, boost::ref(vehicles[i]), i);
		workers.push_back(w);
	}
}


//Quick double-check
bool checkIDs(const vector<Agent>& agents, const vector<TripChain>& trips, const vector<ChoiceSet>& choiceSets, const vector<Vehicle>& vehicles) {
	std::string error = "";
	for (size_t i=0; i<agents.size(); i++) {
		if (agents[i].id != i)
			error = "Agent ID";
	}
	for (size_t i=0; i<trips.size(); i++) {
		if (trips[i].id != i)
			error = "Trip Chain ID";
	}
	for (size_t i=0; i<choiceSets.size(); i++) {
		if (choiceSets[i].id != i)
			error = "Choice Set ID";
	}
	for (size_t i=0; i<vehicles.size(); i++) {
		if (vehicles[i].id != i)
			error = "Vehicle ID";
	}

	if (error.empty())
		return true;
	else {
		cout <<"Error, invalid " <<error <<endl;
		return false;
	}
}


void updateSignalStatus(vector<Region>& regions) {
	//Make workers. When they go out of scope, they will "join" automatically.
	vector<worker> workers;
	for (size_t i=0; i<regions.size(); i++) {
		worker w = boost::bind(&updateSingleRegionSignals, boost::ref(regions[i]));
		workers.push_back(w);
	}
}


void updateTrafficInfo(vector<Region>& regions) {
	//Marked as not threadable.
	for (size_t i=0; i<regions.size(); i++) {
		regions[i].id = regions[i].id; //Trivial. Update other properties later.
	}
}

void calculateTimeDependentShortestPath(vector<Agent>& agents) {
	//Make workers. When they go out of scope, they will "join" automatically.
	vector<worker> workers;
	for (size_t i=0; i<agents.size(); i++) {
		worker w = boost::bind(&updateSingleShortestPath, boost::ref(agents[i]));
		workers.push_back(w);
	}
}


int main(int argc, char* argv[])
{
  //Initialization: Scenario definition
  vector<Agent> agents;
  vector<Region> regions;
  loadUserConf(agents, regions);   //Note: Agent "shells" are loaded here.

  //Initialization: Server configuration
  setConfiguration();

  //Initialization: Network decomposition among multiple machines.
  loadNetwork();

  //Time-based cycle.
  const unsigned int TOTAL_TIME = 100; //Temp.
  const unsigned int TIME_STEP = 10;   //Not sure what this should be.
  for (unsigned int currTime=0; currTime<TOTAL_TIME; currTime+=TIME_STEP) {
	  //NOTE:
	  //  This is supposed to be the "objectMgmtTimeStep for loop", but I am
	  //  not sure exactly how to wrap this in a loop. For now, I just checked if this
	  //  is the first time step
	  if (currTime==0) {
		  //Create object from DB; for long time spans objects must be created on demand.
		  vector<TripChain> trips;
		  loadTripChains(agents, trips);

		  //Agents, choice sets, and vehicles
		  vector<ChoiceSet> choiceSets;
		  vector<Vehicle> vehicles;
		  loadAgentsChoiceSetsAndVehicles(agents, choiceSets, vehicles);

		  //Sanity check (simple)
		  if (!checkIDs(agents, trips, choiceSets, vehicles))
			  return 1;
	  }

	  //Update the signal logic and plans for every intersection grouped by region
	  updateSignalStatus(regions);

	  //Update weather, traffic conditions, etc.
	  updateTrafficInfo();

	  //NOTE:
	  //  The "shortestPathLoopTimeStep for loop" and others are unclear to me.
	  //  For now, I am just performing their tasks if the currTime is evenly
	  //  divisible by the time-tick for that loop.

	  //Longer Time-based cycle
	  if (currTime%shortestPathLoopTimeStep == 0) {
		  calculateTimeDependentShortestPath(agents);
	  }


  }



                                //update the signal logic and plans for every intersection grouped by region
		intersection for loop by region
		{
			*updateSignalStatus();      // return Signal Plan
	}

                                updateTrafficInfo();        // weather info,  traffic condition etc.

                                //Longer Time-based cycle
		shortestPathLoopTimeStep for loop
		{
			*calculateTime-dependentShortestPath();
		}

		//Longer Time-based cycle
		agentDecompositionTimeStep for loop
		{
			agentDecomposition();   // thread controller / processor affinity / Load Balancer
	                               	 updateVehicleQueue();   //One Queue is created for each core
		}

                                //Agent-based cycle
                                agent-based for loop
                                {     updateAndAdvancePhase();
			{
				if (Agent.CurrentMode == 'DRIVER')
					vehiclePosition = drivingBehaviorModel();

				if(Agent. CurrentMode == 'PEDESTRIAN' or 'CYCLIST' )
					pedestrianPosition = pedestrianBehaviorModel();

				if(Agent. CurrentMode == 'PASSENGER')
					vehiclePosition = passengerModel();

				if (end of Link = TRUE)
					pathChoice();

			}
                                }

                                	updateSurveillanceData();
		// check if warm-up has ended
                                	if (t >= simulationStartTime)
                               	 {
                                   		 updateGUI();
			 saveStatistics();
                                	}

		//Longer Time-based cycle
		objectMgmtTimeStep for loop
		{
			saveStatisticsToDB();
		}


                 }

  return 0;
}























boost::mutex root_mutex;


//Container for our threaded task.
struct workerFunc {
  int id;
  workerFunc(int id) : id(id) {}

  void operator() () {
    int ID = pthread_self();

    boost::mutex::scoped_lock temp_lock(root_mutex);
    std::cout <<"PThread" <<std::endl;
    std::cout <<"  ID: " <<id <<std::endl;
    std::cout <<"  PID/TID: " <<getpid() <<"/" <<std::hex <<ID <<std::dec <<std::endl;
    std::cout <<"  CPU: " <<sched_getcpu() <<std::endl;
  }
};


//Change the CPU affinity for the current process
void setCurrAffinity(int procID)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(procID, &cpuset);
  sched_setaffinity(0, sizeof(cpuset), &cpuset);
}


int main(int argc, char* argv[])
{
  //Put this process onto CPU 0
  setCurrAffinity(0);

  int sz = 10;
  boost::thread allThreads[sz];
  for (int i=0; i<sz; i++) {
    //Threads 5 to 10 will run on CPU 1
    if (i==sz/2)
      setCurrAffinity(1);

    //In Linux, pthreads inherit the creator's CPU affinity mask.
    allThreads[i] = (boost::thread(workerFunc(i)));
  }

  for (int i=0; i<sz; i++)
    allThreads[i].join();

  return 0;
}


