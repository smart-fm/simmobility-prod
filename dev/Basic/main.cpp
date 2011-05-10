#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

using std::cout;
using std::endl;

/**
 * A first approximation of the basic pseudo-code in C++
 */


//Constant parameters
const unsigned int shortestPathLoopTimeStep   =    10;
const unsigned int agentDecompositionTimeStep =   100;
const unsigned int objectMgmtTimeStep         =  1000;


//Function stubs
void loadUserConf() { cout <<"Configuration file loaded." <<endl; }
void setConfiguration() { cout <<"Server is configured." <<endl; }
void loadNetwork() { cout <<"Network has been loaded." <<endl; }


int main(int argc, char* argv[])
{
  //Initialization: Scenario definition
  loadUserConf();

  //Initialization: Server configuration
  setConfiguration();

  //Initialization: Network decomposition among multiple machines.
  loadNetwork();

  //Time-based cycle.
  const unsigned int TOTAL_TIME = 100; //Temp.
  const unsigned int TIME_STEP = 10;   //Not sure what this should be.
  for (unsigned int currTime=0; currTime<TOTAL_TIME; currTime+=TIME_STEP)
  {
	  //Create object from DB; for long time spans objects must be created on demand.


  }


                	//Time-based cycle
                *time-based for loop
                {
		//Create Object from DB (objects must be created on demand basis for a longer time span)
		objectMgmtTimeStep for loop
		{
			// Object Manager will create the object reading from persistent database
			*loadTripChains();
		                *createAgents(); *choiceSetGeneration(); *createVehicles();
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


