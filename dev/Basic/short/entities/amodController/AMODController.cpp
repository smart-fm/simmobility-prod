/*
 * AMODController.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#include <memory>
#include <string>
#include <utility>

#include <entities/amodController/AMODController.hpp>

#include "config/ST_Config.hpp"

namespace sim_mob {
namespace amod {

AMODController* AMODController::instance = nullptr;

AMODController::~AMODController() {
	// TODO Auto-generated destructor stub
}


void AMODController::registerController(int id, const sim_mob::MutexStrategy& mtxStrat)
{
	if (getInstance()) {
        delete instance;
	}

    instance = new AMODController(id, mtxStrat);
}

AMODController* AMODController::getInstance()
{
    if (!instance) {
        instance = new AMODController();
	}

    return instance;
}


void AMODController::unregisteredChild(sim_mob::Entity* child)
{
	// TODO called when agent is unregistered
}



void AMODController::deleteInstance()
{
    safe_delete_item(instance);
}

bool AMODController::instanceExists()
{
	return getInstance();
}

AMODController::AMODController(int id,
        const sim_mob::MutexStrategy& mtxStrat)
: Agent(mtxStrat, id),frameTicks(0), currentTime(0),
  amodSim(nullptr), amodManager(nullptr)
{

}

bool AMODController::randomInit() {
    // TODO Initialization for AMODController
    bool initGood = true;

    // init random number generator
    srand(0);

    // create a new simulator (give it my pointer)
    amodSim = new AMODSimulatorSimMobility(this); //

    // create a sample world
    // create vehicles
    int nhours = 4;
    int numCust = 25000*nhours;
    int numVeh = 10000;
    std::vector<amod::Vehicle> vehicles;
    for (int id=1; id<=numVeh; id++) {
        amod::Vehicle veh(id); // all vehicles must have a UNIQUE id
        std::stringstream ss;
        ss << id;
        veh.setName(ss.str());
        veh.setPosition(genRandomPosSingapore());
        veh.setStatus(amod::VehicleStatus::FREE);
        vehicles.push_back(veh);
    }

    // create customer (just one for now)
    std::vector<amod::Customer> customers;
    for (int id=1; id<numCust; id++) {
        amod::Customer cust(id, "John", genRandomPosSingapore());
        customers.push_back(cust);
    }

    // Locations are nodes (we leave this empty because the simulator
    // will populate the locations
    std::vector<amod::Location> locations;

    // populate the world
    amodWorld.populate(locations, vehicles, customers);

    // initialize the simulator
    amodSim->setPickupDistributionParams(20.0, 10.0, 0.0, 0.0); // in seconds
    amodSim->setDropoffDistributionParams(10.0, 1.0, 0.0, 0.0); // in seconds
    amodSim->setTeleportDistributionParams(10.0, 2.0, 0.0, 0.0); // in seconds

    amodSim->init(&amodWorld);
    
//     
//     std::cout.flush();
//     sim_->preComputeDrivingDistances();
//     std::cout << "done..." << std::endl;
//     std::cout.flush();

    Print() << "Load Precomputed distances...";
    amodSim->loadPreComputedDistances("ecbd_shortest_distances.txt");
    Print() << "Done!" << std::endl;
    // create some simple booking
    std::vector<amod::Booking> bookings;
    amod::Booking booking;

    int numBookings = numCust;
    for (int i=1; i<numBookings; i++) {
        booking.id = i; // unique booking id
        booking.bookingTime = rand()%(nhours*60*60) + 20; // in seconds
        booking.custId = customers[i-1].getId(); // which customer to pick up
        booking.source = customers[i-1].getPosition();
        booking.vehId = 0; // veh_id is 0 (the manager will decide this)
        booking.destination = genRandomPosSingapore(); //where the customer wants to go
        bookings.push_back(booking);
    }

    
    amod::ManagerMatchRebalance *matchManager= new amod::ManagerMatchRebalance();
    double distCostFactor = 1.0;
    double waitingCostFactor = 1.0;
    matchManager->setCostFactors(distCostFactor, waitingCostFactor);
    matchManager->setMatchingInterval(5); //30 seconds

    matchManager->init(&amodWorld);
    matchManager->setSimulator(amodSim); // set simulator
    matchManager->loadBookings(bookings); // load the bookings
    matchManager->setMatchMethod(amod::ManagerMatchRebalance::ASSIGNMENT); // ASSIGNMENT
    
    //setup our demand estimator and rebalancing
//    int stationids[] = {19675,10180,17100};
    int stationids[] = {19313, 
        1380030435,
        1380023717,
        1380002862,
        16539,
        20365,
        17033,
        10242,
        19975,
        19585
   };
    std::vector<amod::Location> stations;
    for (int i=0; i<10; ++i) {
       stations.emplace_back(amodWorld.getLocation(stationids[i]));
    }
    
    amod::SimpleDemandEstimator *sde = new amod::SimpleDemandEstimator();
    sde->loadLocations(stations);
    sde->loadBookings(bookings);
    matchManager->setDemandEstimator(sde); // set the demand estimator (for rebalancing)
    matchManager->loadStations(stations, amodWorld);
    matchManager->setRebalancingInterval(5*60);
    matchManager->setVerbose(false);
    // select which manager we want
    amodManager = matchManager; //simple_manager
    
    amodLogger.openLogFile("mrSimLog.txt");
    amodLogger.setMoveEventLogInterval(30.0); //output car movements every 30 seconds
    
    // manager_ = simple_manager;
    //logger_.openLogFile("spSimLog.txt");
    return initGood;
}


// loads the vehicles. Format expected is lines with the values id, x, y
void AMODController::loadStationsFile(std::string filename, std::vector<amod::Location> *locs) {
    std::ifstream fin(filename.c_str());
    if (!fin.good()) {
        throw std::runtime_error("Cannot load from station config file");
    } 
    
    // load the file
    int k = 0;
    while (fin.good()) {
        int id, capacity;
        double x, y;
        id = 0;
        capacity = 0; //we ignore capacity for now
        fin >> id >> x >> y; 
        if (id != 0) {
            std::stringstream ss;
            ss << id;
            locs->emplace_back( id, ss.str(), amod::Position(x,y), capacity);  
        }
    }
    
    Print() << locs->size() << " stations loaded" << std::endl;
}

void AMODController::initVehicles(const std::vector<amod::Location> & locs, std::vector<amod::Vehicle> *vehs, int nvehs) {
    
    int k=0;
    int nstations = locs.size();
    if (nstations == 0 || nvehs == 0) return;
    
    int evenNVehs = nvehs/nstations;
    for (auto itr=locs.begin(); itr!=locs.end(); ++itr) {
        for (int i=0; i< evenNVehs; ++i) {
            // create a new vehicle
            std::stringstream ss;
            ss << ++k;
            vehs->emplace_back(k, ss.str(), itr->getPosition(), 1, amod::VehicleStatus::FREE);
        }
    }
    
    int nExtraVehs = nvehs - evenNVehs*nstations;
    for (auto itr=locs.begin(); itr!=locs.end(); ++itr) {
        if (nExtraVehs > 0) {
            // create a new vehicle
            std::stringstream ss;
            ss << ++k;
            vehs->emplace_back(k, ss.str(), itr->getPosition(), 1, amod::VehicleStatus::FREE);
            nExtraVehs--;
        } else {
            break;
        }
    }
    
    Print() << vehs->size() << " vehicles created" << std::endl;
}

void AMODController::loadCustomerFile(std::string filename, std::vector<amod::Customer> *custs) {
    std::ifstream fin(filename.c_str());
    if (!fin.good()) {
        throw std::runtime_error("Cannot load from customer config file");
    } 
    
    /// load the file
    while (fin.good()) {
        int id = 0;
        double x, y;
        
        fin >> id >> x >> y;
        if (id != 0) {
            /// create a new customer
            std::stringstream ss;
            ss << id;
            custs->emplace_back(id, ss.str(), amod::Position(x,y));
        }
    }
    
    Print() << custs->size() << " customers loaded" << std::endl;
}


bool AMODController::extCBDInit()
{
    bool initGood = true;
    /// load configuration file
    std::string configFileName = ST_Config::getInstance().amod.fileName;
    Print() << "Using configuration file: " << configFileName << std::endl;
    amodConfig.loadFile(configFileName); ///perhaps we can specify this in the simmobility XML
    std::string defaultString = "";
    /// init random number generator
    srand(0);

    /// =============================================================
    /// Initialize the world
    /// load the stations
    std::string stnCfgFileName = amodConfig.get("amod.station_cfg_filename", defaultString);
    Print() << "Initializing stations using: " << stnCfgFileName << std::endl;
    std::vector<amod::Location> stations;
    loadStationsFile(stnCfgFileName, &stations);
    
    /// create vehicles
    int nvehs =  amodConfig.get("amod.num_vehicles", 0);
    Print() << "Initializing " << nvehs << " vehicles "<< std::endl;
    std::vector<amod::Vehicle> vehicles;
    initVehicles(stations, &vehicles, nvehs);
    
    /// create customers
    std::string custConfigFileName = amodConfig.get("amod.customer_cfg_filename", defaultString);
    Print() << "Initializing customers using: " << custConfigFileName << std::endl;
    std::vector<amod::Customer> customers;
    loadCustomerFile(custConfigFileName, &customers);

    /// Locations are nodes (we leave this empty because the simulator
    /// will populate the locations
    std::vector<amod::Location> locations;

    /// populate the world
    amodWorld.populate(locations, vehicles, customers);

    /// =============================================================
    /// create a new simulator
    amodSim = new AMODSimulatorSimMobility(this);
    
    std::string saveNodesFileName = amodConfig.get("amod.save_nodes_filename", defaultString);
    Print() << "Saving nodes to: " << saveNodesFileName << std::endl;
    amodSim->setSaveNodesFilename(saveNodesFileName);
    
    /// setup distributions for simulation
    double pickupMean, pickupStd, pickupMax, pickupMin;
    pickupMean = amodConfig.get("amod.simulator_params.pickup_distribution.mean", 60.0);
    pickupStd = amodConfig.get("amod.simulator_params.pickup_distribution.std", 20.0);
    pickupMin = amodConfig.get("amod.simulator_params.pickup_distribution.min", 10.0);
    pickupMax = amodConfig.get("amod.simulator_params.pickup_distribution.max", 200.0);
    amodSim->setPickupDistributionParams(pickupMean, pickupStd, pickupMin, pickupMax); /// in seconds
    
    double teleportMean, teleportStd, teleportMax, teleportMin;
    teleportMean = amodConfig.get("amod.simulator_params.teleport_distribution.mean", 60.0);
    teleportStd = amodConfig.get("amod.simulator_params.teleport_distribution.std", 20.0);
    teleportMin = amodConfig.get("amod.simulator_params.teleport_distribution.min", 10.0);
    teleportMax = amodConfig.get("amod.simulator_params.teleport_distribution.max", 200.0);
    amodSim->setTeleportDistributionParams(teleportMean, teleportStd, teleportMin, teleportMax); /// in seconds
    
    double dropoffMean, dropoffStd, dropoffMax, dropoffMin;
    dropoffMean = amodConfig.get("amod.simulator_params.dropoff_distribution.mean", 60.0);
    dropoffStd = amodConfig.get("amod.simulator_params.dropoff_distribution.std", 20.0);
    dropoffMin = amodConfig.get("amod.simulator_params.dropoff_distribution.min", 10.0);
    dropoffMax = amodConfig.get("amod.simulator_params.dropoff_distribution.max", 200.0);
    amodSim->setDropoffDistributionParams(dropoffMean, dropoffStd, dropoffMin, dropoffMax); /// in seconds
    
    amodSim->init(&amodWorld);

    /// load precomputed shortest paths (for speedup - optional)
    bool loadDistances = amodConfig.get("amod.simulator_params.shortest_paths.load_distances", false);
    if (loadDistances) {
        std::string distancesFileName = amodConfig.get("amod.simulator_params.shortest_paths.distances_filename", std::string(""));
        Print() << "Loading Precomputed distances from " << distancesFileName << std::endl;
        bool updateDistanceFile = amodConfig.get("amod.simulator_params.shortest_paths.update_distances", false);
        amodSim->loadPreComputedDistances(distancesFileName, updateDistanceFile);
        if (updateDistanceFile) Print() << "Updating distance file" << std::endl;
        Print() << "Done!" << std::endl;
    }

    amodSim->initWaitingTimeFile(amodConfig.get("amod.waiting_time_file", std::string("")));

    /// =============================================================
    /// setup the matching manager
    amod::ManagerMatchRebalance *matchManager= new amod::ManagerMatchRebalance();
    
    /// are we doing greedy or assignment matching?
    std::string matchManagerStr = amodConfig.get("amod.matching_algorithm", defaultString);
    std::transform(matchManagerStr.begin(), matchManagerStr.end(), matchManagerStr.begin(), ::toupper);
    if (matchManagerStr == "GREEDY") {
        Print() << "Using Greedy Matching" << std::endl;
        matchManager->setMatchMethod(amod::ManagerMatchRebalance::GREEDY);
        double matchInterval = amodConfig.get("amod.assignment_params.matching_interval", 1.0);
        matchManager->setMatchingInterval(matchInterval);
    } else if (matchManagerStr == "ASSIGNMENT") {
        Print() << "Using Assignment Matching" << std::endl;
        matchManager->setMatchMethod(amod::ManagerMatchRebalance::ASSIGNMENT);
        double distanceCostFactor = amodConfig.get("amod.assignment_params.distance_cost_factor", 1.0);
        double waitingCostFactor = amodConfig.get("amod.assignment_params.waiting_cost_factor", 1.0);
        matchManager->setCostFactors(distanceCostFactor, waitingCostFactor);
        double matchInterval = amodConfig.get("amod.assignment_params.matching_interval", 1.0);
        matchManager->setMatchingInterval(matchInterval);
    } else {
        Print() << "ERROR! No such matching" << std::endl;
        throw std::runtime_error("No such assignment method supported! Check your amod_config xml file");
    }
    
    /// initialize the manager and load the bookings
    matchManager->init(&amodWorld);
    matchManager->setSimulator(amodSim); /// set simulator
    std::string bookingsFileName = amodConfig.get("amod.bookings_filename", defaultString);
    Print() << "Loading Bookings from " << bookingsFileName << std::endl;
    if (matchManager->loadBookingsFromFile(bookingsFileName) == amod::ERROR_READING_BOOKINGS_FILE) {
        throw std::runtime_error("AMODController: Cannot read bookings file");
    }

    /// set the demand estimator for demand estimation (used for rebalancing)
    amod::SimpleDemandEstimator *sde = new amod::SimpleDemandEstimator();
    sde->loadLocations(stations);
    
    std::string demandEstMethodStr = amodConfig.get("amod.rebalancing_params.demand_estimation_method", defaultString);
    std::transform(demandEstMethodStr.begin(), demandEstMethodStr.end(), demandEstMethodStr.begin(), ::toupper);
    matchManager->useCurrentQueueForEstimation(false);
    if (demandEstMethodStr == "ORACLE") {
        Print() << "Demand Oracle is Loading Bookings from " << bookingsFileName << std::endl;
        sde->loadBookingsFromFile(bookingsFileName);
    } else if (demandEstMethodStr == "FILE") {
        std::string demand_hist_filename = amodConfig.get("amod.rebalancing_params.demand_estimation_file", defaultString);
        Print() << "Demand Prediction is loading bookings histogram from " << demand_hist_filename << " ... ";
        sde->loadBookingsHistFromFile(demand_hist_filename);
        Print() << "Done!" << std::endl;
    } else if (demandEstMethodStr == "QUEUE") {
        Print() << "Rebalancing using current queue only" << std::endl;
        matchManager->useCurrentQueueForEstimation(true);
    }
    else {
        Print() << "No such demand estimation method" << std::endl;
        throw std::runtime_error("No such assignment method supported! Check your amod_config xml file");
    }
    
    double rebalancingInterval = amodConfig.get("amod.rebalancing_params.rebalancing_interval", 1800.0);
    Print() << "Setting Rebalancing Interval: " << rebalancingInterval << std::endl;
    matchManager->setDemandEstimator(sde); /// set the demand estimator (for rebalancing)
    matchManager->loadStations(stations, amodWorld);
    matchManager->setRebalancingInterval(rebalancingInterval);
    
    std::string verboseStr = amodConfig.get("amod.verbose", defaultString);
    std::transform(verboseStr.begin(), verboseStr.end(), verboseStr.begin(), ::toupper);
    if (verboseStr == "TRUE") {
        Print() << "Manager is VERBOSE" << std::endl;
        matchManager->setVerbose(true);
        
        Print() << "Simulator is VERBOSE" << std::endl;
        amodSim->setVerbose(true);
        
    } else {
        Print() << "Manager is QUIET" << std::endl;
        matchManager->setVerbose(false);
        
        Print() << "Simulator is QUIET" << std::endl;
        amodSim->setVerbose(false);
        
    }
    /// select which manager we want
    amodManager = matchManager; //simple_manager
    
    /// =============================================================
    /// setup the logger
    std::string logFileName = amodConfig.get("amod.log_filename", std::string("amodlog.txt"));
    amodLogger.openLogFile(logFileName);
    
    int moveLogInterval = amodConfig.get("amod.move_event_log_interval", 60.0);
    amodLogger.setMoveEventLogInterval(moveLogInterval); ///output car movements every 30 seconds
    
    return initGood;
}

bool AMODController::init()
{
    outputMoveEvents = true; /// do we output the move events to the log file?
    return extCBDInit();
}


sim_mob::Entity::UpdateStatus AMODController::frame_init(timeslice now)
{
	// TODO: Initial frame inits here
    int currTime = now.ms();

	// initialize the controller
	init();

	// set the simulator to the current time
    amodSim->setCurrentTimeMs( currTime ); // needs to be set for trip chains and dispatches

	return sim_mob::Entity::UpdateStatus::Continue;
}


sim_mob::Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
    currentTime = now.ms();
    //std::cout << "Current time: " << current_time_ << std::endl;
    amodSim->setCurrentTimeMs( currentTime ); /// needs to be set for trip chains and dispatches
    amodSim->update(&amodWorld);
    amodManager->update(&amodWorld);
    amodLogger.logEvents(&amodWorld, outputMoveEvents); ///writes to disk and erases
    frameTicks++;

    return sim_mob::Entity::UpdateStatus::Continue;
}

void AMODController::frame_output(timeslice now)
{
	// TODO: overridden but to implement
	// Ask simmobility team as to relevance
}


void AMODController::handleVHArrive(Person_ST* vh) {
	if (!amodSim) {
		throw std::runtime_error("Simulation has ended!");
	}
    int amodId = std::stoi(vh->amodId);
    amodSim->setArrival(amodId, currentTime);
}
void AMODController::handleVHDestruction(Person_ST *vh) {
	// TODO
    Print() << "Vehicle is to be destroyed" << std::endl;
}


void AMODController::handleVHPickup(Person_ST *vh) {
	// TODO

}

amod::Position AMODController::genRandomPosSingapore() {
    int xMin = 36555856;
    int xMax =  37678919;
    int yMin = 14027873;
    int yMax =  14243366;

    double xRand = (xMin + rand()%(xMax - xMin));
    double yRand = (yMin + rand()%(yMax - yMin));

    return amod::Position(xRand, yRand);

}

} /* namespace AMOD */
} /* namespace sim_mob */
