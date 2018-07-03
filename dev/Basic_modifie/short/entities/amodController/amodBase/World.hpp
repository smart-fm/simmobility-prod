/*
 * World.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <vector>
#include <map>
#include <unordered_map>

#include "Vehicle.hpp"
#include "Location.hpp"
#include "Customer.hpp"
#include "Types.hpp"
#include "Event.hpp"

namespace sim_mob{

namespace amod {

/**
 * Class to hold locations, vehicles, customers in the amod world
 */
class World {
public:
    /**
     * Constructor
     */
	World();

    /**
     * Destructor
     */
	virtual ~World();

    /**
     * loads the locations, vehicles and customers into the amod world
     * @param locations
     * @param vehicles
     * @param customers
     */
    virtual void populate(std::vector<Location> &locations,
                          std::vector<Vehicle> &vehicles,
                          std::vector<Customer> &customers);

    /**
     * getCurrentTime
     * @return current time
     */
    virtual double getCurrentTime();
    
    /**
     * retrieves vehicle corresponding to the vehicle id
     * @param vehId - Id of the vehicle to be retrived
     * @return - Vehicle matching the given id
     */
    virtual Vehicle getVehicle(int vehId);

    /**
     * retrieves vehicle corresponding to the vehicle id
     * @param vehId - Id of the vehicle to be retrived
     * @return - Vehicle matching the given id
     */
    virtual Vehicle * getVehiclePtr(int vehId);

    /**
     * retrieves all the vehicles in the amod world
     * @param vehs - container to place the vehicles
     */
    virtual void getVehicles(std::vector<Vehicle> *vehs) const;

    /**
     * retrieves iterator to vehicles container
     * @param bitr - begin iterator
     * @param eitr - end iterator
     */
    virtual void getVehicles(std::unordered_map<int, Vehicle>::const_iterator* bitr,
    		std::unordered_map<int, Vehicle>::const_iterator* eitr) const;

    /**
     * get number of vehicles in the amod world
     * @return - num of vehicles
     */
    virtual int getNumVehicles() const;
    
    /**
     * retrieves a location given its id
     * @param locId - location id
     * @return location corresponding to the location id
     */
    virtual Location getLocation(int locId);

    /**
     * retrieves a location given its id
     * @param locId - location id
     * @return location corresponding to the location id
     */
    virtual Location * getLocationPtr(int locId);

    /**
     * retrieves all the locations in the amod world
     * @param locs - container to place the locations
     */
    virtual void getLocations(std::vector<Location> *locs);

    /**
     * retrieves iterator to locations container
     * @param bitr - begin iterator
     * @param eitr - end iterator
     */
    virtual void getLocations(std::unordered_map<int, Location>::const_iterator* bitr,
    		std::unordered_map<int, Location>::const_iterator* eitr);

    /**
     * get number of locations in the amod world
     * @return - num of locations
     */
    virtual int getNumLocations();
    
    /**
     * retrieves a customer given the id
     * @param custId - customer id
     * @return customer corresponding to the id
     */
    virtual Customer getCustomer(int custId);

    /**
     * retrieves a customer given the id
     * @param custId - customer id
     * @return customer corresponding to the id
     */
    virtual Customer* getCustomerPtr(int custId);

    /**
     * retrieves all the locations in the amod world
     * @param custs - container to place the customers
     */
    virtual void getCustomers(std::vector<Customer> *custs);

    /**
     * retrieves iterator to customers container
     * @param bitr - begin iterator
     * @param eitr - end iterator
     */
    virtual void getCustomers(std::unordered_map<int, Customer>::const_iterator* bitr,
    		std::unordered_map<int, Customer>::const_iterator* eitr);

    /**
     * get number of customers in the amod world
     * @return - num of customers
     */
    virtual int getNumCustomers();
    
    /**
     * getEvents
     * @param evnts
     */
    virtual void getEvents(std::vector<Event> *evnts);

    /**
     * getNumEvents
     * @return
     */
    virtual int getNumEvents();
    
    /**
     * add a vehicle to amod world
     * @param veh - vehicle to be added
     */
	virtual void addVehicle(const Vehicle &veh);

    /**
     * add a vehicle to amod world
     * @param veh - vehicle to be added
     */
	virtual void setVehicle(const Vehicle &veh);

    /**
     * add vehicles to the amod world
     * @param vehs - vehicles to be added
     */
	virtual void addVehicles(const std::vector<Vehicle> &vehs);

    /**
     * remove a vehicle from amod world
     * @param vehId - vehicle id to be removed
     */
    virtual void removeVehicle(int vehId);

    /**
     * remove a list of vehicles from amod world
     * @param vehIds - Id's of vehicles' to be removed
     */
    virtual void removeVehicles(std::vector<int> &vehIds);

    /**
     * add a location to the amod world
     * @param loc - location to be added
     */
    virtual void addLocation(const Location &loc);

    /**
     * add a location to the amod world
     * @param loc - location to be added
     */
    virtual void setLocation(const Location &loc);

    /**
     * add locations to the amod world
     * @param locs - list of locations to be added
     */
    virtual void addLocations(const std::vector<Location> &locs);

    /**
     * remove a location from amod world
     * @param locId - id of the location to be removed
     */
    virtual void removeLocation(int locId);

    /**
     * remove a list of locations from amod world
     * @param locIds - list of locations to be removed
     */
    virtual void removeLocations(std::vector<int> &locIds);
    
    /**
     * add a customer to the amod world
     * @param cust - customer to be added
     */
    virtual void addCustomer(const Customer &cust);

    /**
     * add a customer to the amod world
     * @param cust - customer to be added
     */
    virtual void setCustomer(const Customer &cust);

    /**
     * add customers to the amod world
     * @param custs - customers to be added
     */
    virtual void addCustomers(const std::vector<Customer> &custs);

    /**
     * remove a customer from the amod world
     * @param custId - id of the customer to be removed
     */
    virtual void removeCustomer(int custId);

    /**
     * remove a list of customer from the amod world
     * @param custIds - Id's of customers' to be removed
     */
    virtual void removeCustomers(std::vector<int> &custIds);

    /**
     * add an event
     * @param event
     */
	virtual void addEvent(Event &event);

    /**
     * set an event
     * @param event
     */
	virtual void setEvent(Event &event);

    /**
     * add a list of events
     * @param events_
     */
    virtual void addEvents(const std::vector<Event> &events_);

    /**
     * removes an event given its id
     * @param evntId
     */
    virtual void removeEvent(int evntId);

    /**
     * clears all the events
     */
	virtual void clearEvents();

    /**
     * sets the current simulation time
     * @param currTime
     */
    virtual void setCurrentTime(double currTime);

private:
    /// Container to hold the vehicles in amod world
    std::unordered_map<int, Vehicle> vehicles;

    /// Container to hold the locations in amod world
    std::unordered_map<int, Location> locations;

    /// Container to hold the customers in amod world
    std::unordered_map<int, Customer> customers;

    /// Container to store the events
    std::multimap<int, Event> events;

    /// Current simulation time
    double currentTime;
};

}

}
