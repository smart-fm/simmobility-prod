/*
 * AMODController.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <utility>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "amodBase/Amod.hpp"
#include "AMODSimulatorSimMobility.hpp"
#include "amodBase/ManagerBasic.hpp"
#include "amodBase/ManagerMatchRebalance.hpp"
#include "entities/Agent.hpp"
#include "entities/Person_ST.hpp"
#include "event/args/EventArgs.hpp"
#include "event/EventPublisher.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob {

namespace amod {

namespace pt = boost::property_tree;

/**
 * Structure to hold the amod configuration read from config file
 */
struct AmodConfig {
public:
    AmodConfig () {}
    ~AmodConfig () {}

    void loadFile(std::string config_filename) {
        pt::read_xml(config_filename, tree);
    }

    template <typename T> 
    T get(std::string param_name, T def_val) {
        return tree.get(param_name, def_val);
    }

    pt::ptree tree;
};

class AMODController : public sim_mob::Agent {
public:
    /**
     * Destructor
     */
	virtual ~AMODController();

	/**
	 * for communication with simmobility
	 * retrieve a singleton object
	 * @return a pointer to AMODController .
	 */
    static AMODController* getInstance();

    /**
     * deletes the singleton instance
     */
	static void deleteInstance();

    /**
     * check whether the AMODController instance exists
     * @return true if singleton instance exists
     */
	static bool instanceExists();

    /**
     * registers the AMOD Controller
     * @param id - ID of the controller
     * @param mtxStrat - Mutex Stratergy
     */
    static void registerController(int id, const sim_mob::MutexStrategy& mtxStrat);

    /**
     * Initiatize AMODController
     * @return true if succeeds
     */
	virtual bool init();

    /**
     * randomInit
     * randomly initialize demand across nodes
     * @return true if succeeds
     */
    virtual bool randomInit();
    
    /**
     * extCBDInit
     * Initialization for the ECBD case study
     * @return true if succeeds
     */
    virtual bool extCBDInit();
    
    /**
     * Handler for vehicle arrival event
     * @param vh - Vehicle
     */
	void handleVHArrive(Person_ST* vh);

    /**
     * Handler for vehicle destruction event
     * @param vh - Vehicle
     */
	void handleVHDestruction(Person_ST* vh);

    /**
     * Handler for pickup event
     * @param vh - Vehicle
     */
	void handleVHPickup(Person_ST* vh);
    
protected:
    /**
     * override from the class agent, provide initilization chance to sub class
     * @param now current frame
     * @return true if succeeds
     */
	virtual sim_mob::Entity::UpdateStatus frame_init(timeslice now);

    /**
     * override from the class agent, do frame tick calculation
     * @param now current frame
     * @return Update status
     */
    virtual sim_mob::Entity::UpdateStatus frame_tick(timeslice now);

    /**
     * override from the class agent, print output information
     * @param now current frame
     */
	virtual void frame_output(timeslice now);

    ///May implement later
    /**
     * load
     * @param config_props
     */
	virtual void load(const std::map<std::string, std::string>& config_props){}

    /**
     * isNonspatial
     * @return true, since Signals are non-spatial in nature
     */
    virtual bool isNonspatial() { return true; }

    /**
     * buildSubscriptionList
     * @param subs_list
     */
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subs_list){}

    /**
     * override from the class agent, provide a chance to clear up a child
     * pointer when it will be deleted from system
     * @param child
     */
    virtual void unregisteredChild(sim_mob::Entity* child);

private:
    /**
     * Constructor
     * @param id - ID of the AMOD Controller (default is -1)
     * @param mtxStrat - Mutex Stratergy (default is Buffered)
     */
    explicit AMODController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

    /// Singleton instance
    static AMODController* instance;

    /// current simulation time
    int currentTime;

    /// Pointer to amod simulator
    AMODSimulatorSimMobility *amodSim;

    /// Pointer to amod manager
    Manager *amodManager;

    /// Pointer to amod logger
    Logger amodLogger;

    /// Pointer to Demand Estimator
    DemandEstimator *amodDE;

    /// Flag to check whether to output move events
    bool outputMoveEvents;

    /// configuration file
    AmodConfig amodConfig;
    
    /// number of frame ticks
    int frameTicks;

    /// temporary test variables
    World amodWorld;

    /**
     * generate random coordinate in singapore (for testing purposes)
     * @return random coordinate(Position) in singapore
     */
    Position genRandomPosSingapore();

    /**
     * loadStationsFile
     * @param filename - station file name
     * @param locs - container to load the stations
     */
    virtual void loadStationsFile(std::string filename, std::vector<amod::Location> *locs);

    /**
     * loadCustomerFile
     * @param filename - customers file name
     * @param custs - container to load the customers
     */
    virtual void loadCustomerFile(std::string filename, std::vector<amod::Customer> *custs);

    /**
     * Initialize vehicles
     * @param locs - stations in the network
     * @param vehs - available vehicles
     * @param nvehs - number of vehicles
     */
    virtual void initVehicles(const std::vector<amod::Location> & locs, std::vector<amod::Vehicle> *vehs, int nvehs);
};

}

}
