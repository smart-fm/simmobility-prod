//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * \file main.cpp
 * A first approximation of the basic pseudo-code in C++. The main file loads several
 * properties from data/config.xml, and attempts a simulation run. Currently, the various
 * granularities and pedestrian starting locations are loaded.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 */
#include <vector>
#include <string>

//TODO: Replace with <chrono> or something similar.
#include <sys/time.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

//main.cpp (top-level) files can generally get away with including GenConfig.h
#include "GenConfig.h"

#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"
#include "workers/WorkGroupManager.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "util/DailyTime.hpp"
#include "util/StateSwitcher.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/commsim/Broker.hpp"
//temporary hardcode
//#include "entities/commsim/Broker.hpp"
#include "entities/commsim/broker/derived/RoadRunner.hpp"
#include "entities/commsim/broker/derived/STK.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ParseConfigFile.hpp"
#include "conf/ExpandAndValidateConfigFile.hpp"
#include "entities/AuraManager.hpp"
#include "entities/TrafficWatch.hpp"
#include "entities/Person.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRoleImpl.hpp"
#include "entities/roles/driver/BusDriver.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/driver/driverCommunication/DriverComm.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/roles/pedestrian/Pedestrian2.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "entities/fmodController/FMOD_Controller.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Roundabout.hpp"
#include "geospatial/Intersection.hpp"
#include "geospatial/Route.hpp"
#include "perception/FixedDelayed.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/Locked.hpp"
#include "buffering/Shared.hpp"
#include "network/CommunicationManager.hpp"
#include "network/ControlManager.hpp"
#include "logging/Log.hpp"
#include "util/Utils.hpp"


//add by xuyan
#include "partitions/PartitionManager.hpp"
#include "partitions/ShortTermBoundaryProcessor.hpp"
#include "partitions/ParitionDebugOutput.hpp"

//Note: This must be the LAST include, so that other header files don't have
//      access to cout if SIMMOB_DISABLE_OUTPUT is true.
#include <iostream>
//#include <tinyxml.h>

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace sim_mob;

//Start time of program
timeval start_time;


//Current software version.
const string SIMMOB_VERSION = string(SIMMOB_VERSION_MAJOR) + ":" + SIMMOB_VERSION_MINOR;



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


bool performMain(const std::string& configFileName, std::list<std::string>& resLogFiles, const std::string& XML_OutPutFileName) {
	cout <<"Starting SimMobility, version1 " <<SIMMOB_VERSION <<endl;

	//Parse the config file (this *does not* create anything, it just reads it.).
	ParseConfigFile parse(configFileName, ConfigManager::GetInstanceRW().FullConfig());

	//Enable or disable logging (all together, for now).
	//NOTE: This may seem like an odd place to put this, but it makes sense in context.
	//      OutputEnabled is always set to the correct value, regardless of whether ConfigParams()
	//      has been loaded or not. The new Config class makes this much clearer.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
		//Log::Init("out.txt");
		Warn::Init("warn.log");
		Print::Init("<stdout>");
		PassengerInfoPrint::Init("PassengerInfo.txt");
		HeadwayAtBusStopInfoPrint::Init("HeadwayAtBusStopInfo.txt");
	} else {
		//Log::Ignore();
		Warn::Ignore();
		Print::Ignore();
		PassengerInfoPrint::Ignore();
		HeadwayAtBusStopInfoPrint::Ignore();
	}

	ProfileBuilder* prof = nullptr;
	if (ConfigManager::GetInstance().CMakeConfig().ProfileOn()) {
		ProfileBuilder::InitLogFile("profile_trace.txt");
		prof = new ProfileBuilder();
	}

	//Register our Role types.
	//TODO: Accessing ConfigParams before loading it is technically safe, but we
	//      should really be clear about when this is not ok.

	//TODO: Why were we looping? ~Seth
	//for (int i=0; i<2; i++) {

		//Set for the old-style config first, new-style config second.
		//RoleFactory& rf = (i==0) ? ConfigParams::GetInstanceRW().getRoleFactoryRW() : Config::GetInstanceRW().getRoleFactoryRW();
		//const MutexStrategy& mtx = (i==0) ? ConfigParams::GetInstance().mutexStategy() : Config::GetInstance().mutexStrategy();

		RoleFactory& rf = ConfigManager::GetInstanceRW().FullConfig().getRoleFactoryRW();
		const MutexStrategy& mtx = ConfigManager::GetInstance().FullConfig().mutexStategy();


		//TODO: Check with Vahid if this is likely to cause problems. ~Seth
		if (ConfigManager::GetInstance().FullConfig().commSimEnabled()) {
//			androidBroker = new sim_mob::Broker(MtxStrat_Locked, 0);
			rf.registerRole("driver", new sim_mob::DriverComm(nullptr/*, androidBroker*/, mtx));
		} else {
			rf.registerRole("driver", new sim_mob::Driver(nullptr, mtx));
		}

		rf.registerRole("pedestrian", new sim_mob::Pedestrian2(nullptr));
		rf.registerRole("passenger",new sim_mob::Passenger(nullptr, mtx));
		rf.registerRole("busdriver", new sim_mob::BusDriver(nullptr, mtx));
		rf.registerRole("activityRole", new sim_mob::ActivityPerformer(nullptr));
		rf.registerRole("waitBusActivityRole", new sim_mob::WaitBusActivityRoleImpl(nullptr));
		//cannot allocate an object of abstract type
		//rf.registerRole("activityRole", new sim_mob::ActivityPerformer(nullptr));
		//rf.registerRole("buscontroller", new sim_mob::BusController()); //Not a role!
//	}

	//Loader params for our Agents
	WorkGroup::EntityLoadParams entLoader(Agent::pending_agents, Agent::all_agents);

	//Prepare our built-in models
	//NOTE: These can leak memory for now, but don't delete them because:
	//      A) If a built-in Construct is used then it will need these models.
	//      B) We'll likely replace these with Factory classes later (static, etc.), so
	//         memory management will cease to be an issue.
	/*Config::BuiltInModels builtIn;
	builtIn.carFollowModels["mitsim"] = new MITSIM_CF_Model();
	builtIn.laneChangeModels["mitsim"] = new MITSIM_LC_Model();
	builtIn.intDrivingModels["linear"] = new SimpleIntDrivingModel();*/

	//Load our user config file
	std::cout << "Expanding our user config file." << std::endl;
	ExpandAndValidateConfigFile expand(ConfigManager::GetInstanceRW().FullConfig(), Agent::all_agents, Agent::pending_agents);
	std::cout << "finish to Load our user config file." << std::endl;

	std::cout<<"performMain: trip chain pool size "<<
				ConfigManager::GetInstance().FullConfig().getTripChains().size()<<std::endl;

	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		// init path set manager
		time_t t = time(0);   // get time now
		struct tm * now = localtime( & t );
		std::cout<<"begin time:"<<std::endl;
		std::cout<<now->tm_hour<<" "<<now->tm_min<<" "<<now->tm_sec<< std::endl;
		PathSetManager* psMgr = PathSetManager::getInstance();
		std::string name=configFileName;
		psMgr->setScenarioName(name);
//		psMgr->setTravleTimeTmpTableName(ConfigParams::GetInstance().travelTimeTmpTableName);
//		psMgr->createTravelTimeTmpTable(psMgr->getTravleTimeTmpTableName());
//		psMgr->getDataFromDB();
		if(psMgr->isUseCatchMode())
		{
			psMgr->generateAllPathSetWithTripChain2();
		}
//		psMgr->saveDataToDB();
		t = time(0);   // get time now
		now = localtime( & t );
		std::cout<<now->tm_hour<<" "<<now->tm_min<<" "<<now->tm_sec<< std::endl;
		std::cout<<psMgr->size()<<std::endl;
	}

//	//DriverComms are only allowed if the communicator is enabled.
//	if (ConfigParams::GetInstance().commSimEnabled) {
//		androidBroker.enable();
//	}

	//Initialize the control manager and wait for an IDLE state (interactive mode only).
	sim_mob::ControlManager* ctrlMgr = nullptr;
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) {
		std::cout<<"load scenario ok, simulation state is IDLE"<<std::endl;
		ctrlMgr = ConfigManager::GetInstance().FullConfig().getControlMgr();
		ctrlMgr->setSimState(IDLE);
		while(ctrlMgr->getSimState() == IDLE) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
	}

	//Save a handle to the shared definition of the configuration.
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();

	//Start boundaries
	if (!config.MPI_Disabled() && config.using_MPI) {
		PartitionManager::instance().initBoundaryTrafficItems();
	}

	//bool NoDynamicDispatch = config.DynamicDispatchDisabled();

	PartitionManager* partMgr = nullptr;
	if (!config.MPI_Disabled() && config.using_MPI) {
		partMgr = &PartitionManager::instance();
	}

	{ //Begin scope: WorkGroups
	//TODO: WorkGroup scope currently does nothing. We need to re-enable WorkGroup deletion at some later point. ~Seth
	WorkGroupManager wgMgr;
	wgMgr.setSingleThreadMode(config.singleThreaded());

	//Work Group specifications
	WorkGroup* personWorkers = wgMgr.newWorkGroup(config.personWorkGroupSize(), config.totalRuntimeTicks, config.granPersonTicks, &AuraManager::instance(), partMgr);
	WorkGroup* signalStatusWorkers = wgMgr.newWorkGroup(config.signalWorkGroupSize(), config.totalRuntimeTicks, config.granSignalsTicks);

	//TODO: Ideally, the Broker would go on the agent Work Group. However, the Broker often has to wait for all Agents to finish.
	//      If an Agent is "behind" the Broker, we have two options:
	//        1) Have some way of specifying that the Broker agent goes "last" (Agent priority?)
	//        2) Have some way of telling the parent Worker to "delay" this Agent (e.g., add it to a temporary list) from *within* update.
	WorkGroup* communicationWorkers = wgMgr.newWorkGroup(config.commWorkGroupSize(), config.totalRuntimeTicks, config.granCommunicationTicks);

	//NOTE: I moved this from an #ifdef into a local variable.
	//      Recompiling main.cpp is much faster than recompiling everything which relies on
	//      PerformanceProfile.hpp   ~Seth
#if 0
	bool doPerformanceMeasurement = false; //TODO: From config file.
	bool measureInParallel = true;
	PerformanceProfile perfProfile;
	if (doPerformanceMeasurement) {
		perfProfile.init(config.personWorkGroupSize(), measureInParallel);
	}
#endif

	//Initialize the aura manager
	AuraManager::instance().init(config.aura_manager_impl()
			//,(doPerformanceMeasurement ? &perfProfile : nullptr)
	);

	//Initialize all work groups (this creates barriers, and locks down creation of new groups).
	wgMgr.initAllGroups();

	//Initialize each work group individually
	personWorkers->initWorkers(&entLoader);
	signalStatusWorkers->initWorkers(nullptr);
	communicationWorkers->initWorkers(nullptr);

	//TODO: We shouldn't add the Broker unless Communication is enabled in the config file.
//	//..and Assign all communication agents(we have one ns3 communicator for now)
//	communicationWorkers->assignAWorker(&(sim_mob::NS3_Communicator::GetInstance()));
//	if(ConfigManager::GetInstance().FullConfig().commSimEnabled())
//	{
//		const std::string & name = ConfigManager::GetInstance().FullConfig().getAndroidClientType();
//		Broker *androidBroker = new sim_mob::Broker(MtxStrat_Locked, 0);
//		Broker::addExternalCommunicator(name, androidBroker);
//		Print() << "main.cpp:: android broker[" << androidBroker << "] of type[" << name << "] retrieved" << std::endl;
//		communicationWorkers->assignAWorker(androidBroker);
//		androidBroker->enable();
//	}

	if(ConfigManager::GetInstance().FullConfig().commSimEnabled())
	{
		const std::map<std::string,sim_mob::SimulationParams::CommsimElement> & elements =
				ConfigManager::GetInstance().FullConfig().getCommSimElements();
		std::map<std::string,sim_mob::SimulationParams::CommsimElement>::const_iterator it = elements.begin();
		for(; it != elements.end(); it++){
			//todo: to be automated later,(if required at all)
			if(!(it->second.enabled)) {
				continue;
			}
			Broker *broker =  nullptr;
			if(it->first == "roadrunner"){
				broker = new sim_mob::Roadrunner_Broker(MtxStrat_Locked, 0);
			}
			else if(it->first == "stk"){
				broker = new sim_mob::STK_Broker(MtxStrat_Locked, 0, it->second.name, it->second.mode);
			}

			Broker::addExternalCommunicator(it->first, broker);
			communicationWorkers->assignAWorker(broker);
			broker->enable();
		}
	}


	//Anything in all_agents is starting on time 0, and should be added now.
	for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
		personWorkers->assignAWorker(*it);

		//put them to AuraManager
		//Agent* an_agent = dynamic_cast<Agent*>(*it);
		//if(an_agent) AuraManager::instance().registerNewAgent(an_agent);
	}

	//Assign all BusStopAgents
	std::cout << "BusStopAgent::all_BusstopAgents_.size(): " << BusStopAgent::AllBusStopAgentsCount() << std::endl;
	BusStopAgent::AssignAllBusStopAgents(*personWorkers);

	//Assign all signals too
	for (vector<Signal*>::iterator it = Signal::all_signals_.begin(); it != Signal::all_signals_.end(); it++) {
		signalStatusWorkers->assignAWorker(*it);
	}


	if(sim_mob::FMOD::FMOD_Controller::instanceExists()){
		personWorkers->assignAWorker( sim_mob::FMOD::FMOD_Controller::instance() );
	}

	//..and Assign communication agent(currently a singleton


	cout << "Initial Agents dispatched or pushed to pending." << endl;

	//Initialize the aura manager
	AuraManager::instance().init(config.aura_manager_impl()
#if 0
			,(doPerformanceMeasurement ? &perfProfile : nullptr)
#endif
			);


	///
	///  TODO: Do not delete this next line. Please read the comment in TrafficWatch.hpp
	///        ~Seth
	///
//	TrafficWatch& trafficWatch = TrafficWatch::instance();

	//Start work groups and all threads.
	wgMgr.startAllWorkGroups();

	//
	if (!config.MPI_Disabled() && config.using_MPI) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.setEntityWorkGroup(personWorkers, signalStatusWorkers);

		std::cout << "partition_solution_id in main function:" << partitionImpl.partition_config->partition_solution_id << std::endl;
	}

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
	int loop_start_offset = ProfileBuilder::diff_ms(loop_start_time, start_time);

	ParitionDebugOutput debug;


	StateSwitcher<int> numTicksShown(0); //Only goes up to 10
	StateSwitcher<int> lastTickPercent(0); //So we have some idea how much time is left.
	int endTick = config.totalRuntimeTicks;
	for (unsigned int currTick = 0; currTick < endTick; currTick++) {
		if (ConfigManager::GetInstance().FullConfig().InteractiveMode()) {
			if(ctrlMgr->getSimState() == STOP) {
				while (ctrlMgr->getEndTick() < 0) {
					ctrlMgr->setEndTick(currTick+2);
				}
				endTick = ctrlMgr->getEndTick();
			}
		}

//		std::cout << "Time:" << currTick << std::endl;

		//xuyan:measure simulation time
		//NOTE: It is much better to filter performance results via a script after the fact.
		/*if (currTick == 600 * 10 + 1)
		{ // mins
			if (doPerformanceMeasurement) {
				perfProfile.startMeasure();
				perfProfile.markStartSimulation();
			}
		}
		if (currTick == endTick - 1)
		{ // mins
			if (doPerformanceMeasurement) {
				perfProfile.markEndSimulation();
				perfProfile.endMeasure();
			}
		}*/

		//Flag
		bool warmupDone = (currTick >= config.totalWarmupTicks);

		//Save the maximum number of agents at any given time
		maxAgents = std::max(maxAgents, Agent::all_agents.size());

		//Output. We show the following:
		//   The first 10 time ticks. (for debugging purposes)
		//   Every 1% change after that. (to avoid flooding the console.)
		//   In "OutputDisabled" mode, every 10% change. (just to give some indication of progress)
		int currTickPercent = (currTick*100)/config.totalRuntimeTicks;
		if (ConfigManager::GetInstance().CMakeConfig().OutputDisabled()) {
			currTickPercent /= 10; //Only update 10%, 20%, etc.
		}

		//Determine whether to print this time tick or not.
		bool printTick = lastTickPercent.update(currTickPercent);
		if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && !printTick) {
			//OutputEnabled also shows the first 10 ticks.
			printTick = numTicksShown.update(std::min(numTicksShown.get()+1, 10));
		}

		//Note that OutputEnabled also affects locking.
		if (printTick) {
			if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled()) {
				std::stringstream msg;
				msg << "Approximate Tick Boundary: " << currTick << ", ";
				msg << (currTick * config.baseGranMS()) << " ms   [" <<currTickPercent <<"%]" << endl;
				if (!warmupDone) {
					msg << "  Warmup; output ignored." << endl;
				}
				PrintOut(msg.str());
			} else {
				//We don't need to lock this output if general output is disabled, since Agents won't
				//  perform any output (and hence there will be no contention)
				std::cout <<currTickPercent <<"0%" << ",agents:" << Agent::all_agents.size() <<std::endl;
			}
		}

		///
		///  TODO: Do not delete this next line. Please read the comment in TrafficWatch.hpp
		///        ~Seth
		///
//		trafficWatch.update(currTick);

		//Agent-based cycle, steps 1,2,3,4 of 4
		wgMgr.waitAllGroups();

		//Check if the warmup period has ended.
		if (warmupDone) {
		}
	}


	timeval loop_end_time;
	gettimeofday(&loop_end_time, nullptr);
	int loop_time = ProfileBuilder::diff_ms(loop_end_time, loop_start_time);
	std::ostringstream out("");
	out << "loop_time:" << std::dec << loop_time  <<std::endl;
	std::cout << out.str();
	//Finalize partition manager
	if (!config.MPI_Disabled() && config.using_MPI) {
		PartitionManager& partitionImpl = PartitionManager::instance();
		partitionImpl.stopMPIEnvironment();
	}

	if(sim_mob::FMOD::FMOD_Controller::instanceExists()){
		sim_mob::FMOD::FMOD_Controller::instance()->finalizeMessageToFMOD();
	}

	std::cout <<"Database lookup took: " <<loop_start_offset <<" ms" <<std::endl;

	cout << "Max Agents at any given time: " <<maxAgents <<std::endl;
	cout << "Starting Agents: " << numStartAgents;
	cout << ",     Pending: ";
	//if (NoDynamicDispatch) {
		cout <<"<Disabled>";
	//} else {
		cout <<numPendingAgents;
	//}
	cout << endl;

	//xuyan:show measure time
	//ProfileBuilder does this automatically on destruction.
	//if (doPerformanceMeasurement) {
	//	perfProfile.showPerformanceProfile();
	//}

	if (Agent::all_agents.empty()) {
		cout << "All Agents have left the simulation.\n";
	} else {
		size_t numPerson = 0;
		size_t numDriver = 0;
		size_t numPedestrian = 0;
		size_t numPassenger = 0;
		for (std::set<Entity*>::iterator it = Agent::all_agents.begin(); it != Agent::all_agents.end(); it++) {
			Person* p = dynamic_cast<Person*> (*it);
			if (p) {
				numPerson++;
				if (p->getRole() && dynamic_cast<Driver*> (p->getRole())) {
					numDriver++;
				}
				if (p->getRole() && dynamic_cast<Pedestrian*> (p->getRole())) {
					numPedestrian++;
				}
				if (p->getRole() && dynamic_cast<Passenger*> (p->getRole())) {
					numPassenger++;
								}
			}
		}
		cout << "Remaining Agents: " << numPerson << " (Person)   "
				<< (Agent::all_agents.size() - numPerson) << " (Other)" << endl;
		cout << "   Person Agents: " << numDriver << " (Driver)   "
				<< numPedestrian << " (Pedestrian)   " << numPassenger << " (Passenger) " << (numPerson
				- numDriver - numPedestrian) << " (Other)" << endl;
//		cout << "Created: " << Agent::createdAgents << "\nDied: "<< Agent::diedAgents << "\nDied For Broker: "<< Broker::diedAgents
//				<< "\nSubscribed For Broker: "<< Broker::subscribedAgents <<endl;
	}

	if (ConfigManager::GetInstance().FullConfig().numAgentsSkipped>0) {
		cout <<"Agents SKIPPED due to invalid route assignment: " <<ConfigManager::GetInstance().FullConfig().numAgentsSkipped <<endl;
	}

	if (!Agent::pending_agents.empty()) {
		cout << "WARNING! There are still " << Agent::pending_agents.size()
				<< " Agents waiting to be scheduled; next start time is: "
				<< Agent::pending_agents.top()->getStartTime() << " ms\n";
		/*if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
			throw std::runtime_error("ERROR: pending_agents shouldn't be used if Dynamic Dispatch is disabled.");
		}*/
	}

	//Save our output files if we are merging them later.
	if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && ConfigManager::GetInstance().FullConfig().mergeLogFiles()) {
		resLogFiles = wgMgr.retrieveOutFileNames();
	}

	//Here, we will simply scope-out the WorkGroups, and they will migrate out all remaining Agents.
	}  //End scope: WorkGroups. (Todo: should move this into its own function later)
	//WorkGroup::FinalizeAllWorkGroups();

	//At this point, it should be possible to delete all Signals and Agents.
	//TODO: For some reason, clear_delete_vector() does (may?) not work in INTERACTIVE mode.
	//      We can address this later, but it should *definitely* be possible to cleanly
	//      exit (even early) from the simulation.
	//TODO: I think that the WorkGroups and Workers need to have the "endTick" value propagated to
	//      them from the main loop, in the event that the simulator is shutting down early. This is
	//      probably causing the Workers to hang if clear_delete_vector is called. ~Seth
	//EDIT: Actually, Worker seems to handle the synchronization fine too.... but I still think the main
	//      loop should propagate this value down. ~Seth
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) {
		Signal::all_signals_.clear();
		Agent::all_agents.clear();
	} else {
		clear_delete_vector(Signal::all_signals_);
		clear_delete_vector(Agent::all_agents);
	}

	cout << "Simulation complete; closing worker threads." << endl;

	//Delete our profiler, if it exists.
	safe_delete_item(prof);
	return true;
}

/**
 * Run the main loop of Sim Mobility, using command-line input.
 * Returns the value of the last completed run of performMain().
 */
int run_simmob_interactive_loop(){
	sim_mob::ControlManager *ctrlMgr = ConfigManager::GetInstance().FullConfig().getControlMgr();
	std::list<std::string> resLogFiles;
	int retVal = 1;
	for (;;) {
		if(ctrlMgr->getSimState() == LOADSCENARIO)
		{
			ctrlMgr->setSimState(RUNNING);
			std::map<std::string,std::string> paras;
			ctrlMgr->getLoadScenarioParas(paras);
			std::string configFileName = paras["configFileName"];
			retVal = performMain(configFileName,resLogFiles, "XML_OutPut.xml") ? 0 : 1;
			ctrlMgr->setSimState(STOP);
			ConfigManager::GetInstanceRW().reset();
			std::cout<<"scenario finished"<<std::cout;
		}
		if(ctrlMgr->getSimState() == QUIT)
		{
			std::cout<<"Thank you for using SIMMOB. Have a good day!"<<std::endl;
			break;
		}
	}

	return retVal;
}

int main(int ARGC, char* ARGV[])
{
	std::vector<std::string> args = Utils::parseArgs(ARGC, ARGV);

	//Currently needs the #ifdef because of the way threads initialize.
#ifdef SIMMOB_INTERACTIVE_MODE
	CommunicationManager *dataServer = new CommunicationManager(13333, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread dataWorkerThread(boost::bind(&CommunicationManager::start, dataServer));
	CommunicationManager *cmdServer = new CommunicationManager(13334, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread cmdWorkerThread(boost::bind(&CommunicationManager::start, cmdServer));
	CommunicationManager *roadNetworkServer = new CommunicationManager(13335, ConfigManager::GetInstance().FullConfig().getCommDataMgr(), *ConfigManager::GetInstance().FullConfig().getControlMgr());
	boost::thread roadNetworkWorkerThread(boost::bind(&CommunicationManager::start, roadNetworkServer));
	boost::thread workerThread2(boost::bind(&ControlManager::start, ConfigManager::GetInstance().FullConfig().getControlMgr()));
#endif

	//Save start time
	gettimeofday(&start_time, nullptr);

	/**
	 * Check whether to run SimMobility or SimMobility-MPI
	 * TODO: Retrieving ConfigParams before actually loading the config file is dangerous.
	 */
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	config.using_MPI = false;
#ifndef SIMMOB_DISABLE_MPI
	if (args.size()>2 && args[2]=="mpi") {
		config.using_MPI = true;
	}
#endif

	/**
	 * set random be repeatable
	 * TODO: Retrieving ConfigParams before actually loading the config file is dangerous.
	 */
	config.is_simulation_repeatable = true;

	/**
	 * Start MPI if using_MPI is true
	 */
#ifndef SIMMOB_DISABLE_MPI
	if (config.using_MPI)
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		std::string mpi_result = partitionImpl.startMPIEnvironment(ARGC, ARGV); //NOTE: MPI_Init needs the raw argc/argv.
		if (mpi_result.compare("") != 0)
		{
			Warn() << "MPI Error:" << mpi_result << endl;
			exit(1);
		}

		ShortTermBoundaryProcessor* boundary_processor = new ShortTermBoundaryProcessor();
		partitionImpl.setBoundaryProcessor(boundary_processor);
	}
#endif

	//Argument 1: Config file
	//Note: Don't change this here; change it by supplying an argument on the
	//      command line, or through Eclipse's "Run Configurations" dialog.
	std::string configFileName = "data/config.xml";
	std::string XML_OutPutFileName = "private/SimMobilityInput.xml";
	if (args.size() > 1) {
		configFileName = args[1];
	} else {
		cout << "No config file specified; using default." << endl;
	}
	cout << "Using config file: " << configFileName << endl;

	//Perform main loop (this differs for interactive mode)
	int returnVal = 1;
	std::list<std::string> resLogFiles;
	if (ConfigManager::GetInstance().CMakeConfig().InteractiveMode()) {
		returnVal = run_simmob_interactive_loop();
	} else {
		returnVal = performMain(configFileName, resLogFiles, "XML_OutPut.xml") ? 0 : 1;
	}

	//Concatenate output files?
	if (!resLogFiles.empty()) {
		resLogFiles.insert(resLogFiles.begin(), ConfigManager::GetInstance().FullConfig().outNetworkFileName);
		Utils::printAndDeleteLogFiles(resLogFiles);
	}

	cout << "Done" << endl;
	return returnVal;
}

