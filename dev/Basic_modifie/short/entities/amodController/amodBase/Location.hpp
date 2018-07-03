/*
 * Location.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Types.hpp"
#include "Entity.hpp"

#include <unordered_set>
#include <vector>
#include <string>

namespace sim_mob{

namespace amod {

class Location : public Entity {
public:
    /**
     * Constructor
     */
	Location ();

    /**
     * Constructor
     * @param id - Location id
     * @param name - Location name
     * @param pos - Position of the location
     * @param capacity - capacity of the location/station
     */
    Location(int id, std::string name, Position pos, int capacity_);

    /**
     * Destructor
     */
	virtual ~Location();

    /// Functions to modify the vehicles at this location
    /// Location vehicles are free vehicles in this area.
    /// but have not been picked up from this location. It does not
    /// include the people who have been dropped off at this location.
    /**
     * getNumVehicles
     * Retrieves num of vehicles in the location
     * @return num vehicles
     */
	virtual int getNumVehicles() const;

    /**
     * addVehicleId
     * add a vehicle in the location
     * @param vehId_ - vehicle id
     */
    virtual void addVehicleId(int vehId_);

    /**
     * removeVehicleId
     * remove a vehicle from the location
     * @param vehId_ - vehicle id
     */
    virtual void removeVehicleId(int vehId_);

    /**
     * getVehicleIds
     * retrives the vehicles in the location
     * @param vehIds_ - container to store the vehicle ids
     */
    virtual void getVehicleIds(std::unordered_set<int> *vehIds_);

    /**
     * getVehicleIds
     * retrives iterator to vehicle containter in the location
     * @param bitr - begin iterator
     * @param eitr - end iterator
     */
	virtual void getVehicleIds(std::unordered_set<int>::const_iterator *bitr, std::unordered_set<int>::const_iterator *eitr);

    /**
     * clearVehicleIds
     */
	virtual void clearVehicleIds();

    /// Functions to modify the customers at this location
    /// Location customers are people who are currently at this node
    /// i.e., those instantiated here or have been dropped off.
    /**
     * getNumCustomers
     * retrieves the number of customers at the location
     * @return number of customers
     */
    virtual int getNumCustomers() const;

    /**
     * getCustomerIds
     * retrieves the list of ids of customers at the location
     * @param custIds_ - customer ids
     */
    virtual void getCustomerIds(std::unordered_set<int> *custIds_);

    /**
     * getCustomerIds
     * retrieves iterator to the customer containter at the location
     * @param bitr - begin iterator
     * @param eitr - end iterator
     */
	virtual void getCustomerIds(std::unordered_set<int>::const_iterator *bitr, std::unordered_set<int>::const_iterator *eitr);

    /**
     * addCustomerId
     * add a customer id at the location
     * @param custId_ - customer id
     */
    virtual void addCustomerId(int custId_);

    /**
     * removeCustomerId
     * remove a customer id from the location
     * @param custId_ - customer id
     */
    virtual void removeCustomerId(int custId_);

    /**
     * clearCustomerIds
     */
	virtual void clearCustomerIds();

    /**
     * setCapacity
     * sets the capacity of the location
     * @param capacity
     */
	virtual void setCapacity(int capacity);

    /**
     * getCapacity
     * retrieves the capacity of the location
     * @return capacity of the location
     */
	virtual int getCapacity() const;

    /// operators for KDTree operation
    /// inlined for convenience
    /**
     * dims
     * dimension of kdtree
     * @return 2 (2 dimension)
     */
    int dims() const { return 2; }

    /**
     * operator []
     * operator[] overloading
     * @param i location id
     * @return position of the location
     */
    double & operator[](int i) { return getPosition()[i]; }

    /**
     * operator []
     * operator[] overloading
     * @param i location id
     * @return position of the location
     */
    double operator[](int i) const { return getPosition()[i]; }

    /**
     * operator ==
     * operator== overloading
     * @param rhs - right hand side
     * @return true if rhs position equal to this position
     */
	bool operator==(Location &rhs) {
        /// two locations that have the same position are considered equal
		return (getPosition() == rhs.getPosition());
	}

private:
    /// Container to store the vehicle ids
    std::unordered_set<int> vehIds;

    /// Containter to store the customer ids
    std::unordered_set<int> custIds;

    /// Capacity of the location
    int capacity;
};

}
}
