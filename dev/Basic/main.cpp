#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time.hpp>

using std::cout;
using std::endl;
using std::vector;
using boost::thread;

/**
 * A first approximation of the basic pseudo-code in C++
 */


//Constant parameters
const unsigned int shortestPathLoopTimeStep   =    10;
const unsigned int agentDecompositionTimeStep =   100;
const unsigned int objectMgmtTimeStep         =  1000;

//Driver modes
enum DRIVER_MODES {
	DRIVER,
	PEDESTRIAN,
	CYCLIST,
	PASSENGER
};

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
	unsigned int currMode;
	Agent(unsigned int id=0) : id(id) {
		int currMode = id%4;
		if (currMode==0)
			currMode = DRIVER;
		else if (currMode==1)
			currMode = PEDESTRIAN;
		else if (currMode==2)
			currMode = CYCLIST;
		else if (currMode==3)
			currMode = PASSENGER;
	}
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


//"Trivial" function does nothing, returns a trivial condition. Used to indicate future functionality.
bool trivial(unsigned int id) {
	return id%2==0;
}


//Simple worker class. Start a thread to perform this task on construction.
//   Ensure that function has completed upon destruction.
class worker {
private:
	thread workerThread;

public:
	worker(boost::function<void()> callable) {
		workerThread = thread(callable);
	}
	~worker() {
		workerThread.join();
	}
};


//Function stubs
void loadUserConf(vector<Agent>& agents, vector<Region>& regions) {
	for (size_t i=0; i<20; i++)
		agents.push_back(Agent(i));
	for (size_t i=0; i<5; i++)
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
	trivial(ag.id); //Trivial. Presumably, we'd set an agent's other properties here.
}
void createSingleChoiceSet(ChoiceSet& cs, unsigned int newID) {
	cs.id = newID;
}
void createSingleVehicle(Vehicle& v, unsigned int newID) {
	v.id = newID;
}
void updateSingleRegionSignals(Region& r) {
	for (size_t i=0; i<r.signals.size(); i++) {
		r.signals[i].id = r.signals[i].id; //Trivial. Will update signal's other properties.
	}
}
void updateSingleShortestPath(Agent& a) {
	trivial(a.id); //Trivial. Will update shortest path later.
}
void pathChoice(Agent& a) {
	trivial(a.id); //Trivial. Will update path choice later.
}
void updateDriverBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update driver behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);
	}
}
void updatePedestrianBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update pedestrian behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);
	}
}
void updatePassengerBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update passenger behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);  //NOTE: Do passengers need to do this?
	}
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

void agentDecomposition(vector<Agent>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		agents[i].id = agents[i].id; //Trivial. Possibly move agents later.
	}
}

void updateVehicleQueue(vector<Vehicle>& vehicles) {
	//Marked as not threadable.
	for (size_t i=0; i<vehicles.size(); i++) {
		vehicles[i].id = vehicles[i].id; //Trivial. Will update queues later.
	}
}

void updateAndAdvancePhase(vector<Agent>& agents) {
	//NOTE: This is marked as not threadable, but I am treating it as threadable for now.
	vector<worker> workers;
	for (size_t i=0; i<agents.size(); i++) {
		if (agents[i].currMode==DRIVER) {
			worker w = boost::bind(&updateDriverBehavior, boost::ref(agents[i]));
			workers.push_back(w);
		} else if (agents[i].currMode==PEDESTRIAN || agents[i].currMode==CYCLIST) {
			worker w = boost::bind(&updatePedestrianBehavior, boost::ref(agents[i]));
			workers.push_back(w);
		} else if (agents[i].currMode==PASSENGER) {
			worker w = boost::bind(&updatePassengerBehavior, boost::ref(agents[i]));
			workers.push_back(w);
		}

		//NOTE: I added "path choice" into each of the "worker" functions.
	}
}


void updateSurveillanceData(vector<Agent>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		agents[i].id = agents[i].id; //Trivial. Later we will collate data and send it to surveillance systems.
	}
}

void updateGUI(vector<Agent>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		agents[i].id = agents[i].id; //Trivial. Later we will update the GUI
	}
}

void saveStatistics(vector<Agent>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		agents[i].id = agents[i].id; //Trivial. Later we will log all agent data.
	}
}

void saveStatisticsToDB(vector<Agent>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		agents[i].id = agents[i].id; //Trivial. Later we will save all statistis to the database.
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
  //const unsigned int TIME_STEP = std::min(std::min(objectMgmtTimeStep, agentDecompositionTimeStep), shortestPathLoopTimeStep);
  const unsigned int TIME_STEP = 1; //NOTE: Is this correct?
  const unsigned int simulationStartTime = 10; //Temp.
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

	  //Longer Time-based cycle
	  if (currTime%agentDecompositionTimeStep == 0) {
		  //Thread controller / processor affinity / Load Balancer
		  agentDecomposition(agents);

		  //One Queue is created for each core
		  updateVehicleQueue(vehicles);
	  }

	  //Agent-based cycle
	  if (true) { //Seems to operate every time step?
		  updateAndAdvancePhase(agents);
	  }

	  //Surveillance update
	  updateSurveillanceData();

	  //Check if the warmup period has ended.
	  if (currTime >= simulationStartTime) {
		  updateGUI();
		  saveStatistics();
	  }


	  //Longer Time-based cycle
	  if (currTime%objectMgmtTimeStep == 0) {
		  saveStatisticsToDB(agents);
	  }
  }

  cout <<"Done" <<endl;

  return 0;
}



