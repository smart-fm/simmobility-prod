/*
 * Customer.hpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Entity.hpp"

namespace sim_mob {

namespace amod {

class Customer: public Entity {
public:
	enum Status {
		FREE,
		WAITING_FOR_PICKUP,
		IN_VEHICLE,
		WAITING_FOR_DROPOFF,
		WAITING_FOR_ASSIGNMENT,
		TELEPORTING,
	};

public:
    /**
     * Constructor
     * @param id - Customer id
     * @param name - Customer name
     * @param pos - Position
     * @param locationId - Location ID
     * @param assignedVeh - Assigned vehicle
     * @param inVeh - is already in vehicle
     * @param status_ - Customer status
     */
    Customer(int id = 0, const std::string& name = "", Position pos = Position(), int locationId = 0,
            int assignedVeh = 0, bool inVeh = false,
            Customer::Status status_ = Customer::Status::FREE);

    /**
     * Destructor
     */
	virtual ~Customer();

    /**
     * setAssignedVehicleId
     * sets assigned vehicle id
     * @param vehId_ - vehicle id
     */
    virtual void setAssignedVehicleId(int vehId_);

    /**
     * clearAssignedVehicleId
     * clears Assigned vehicle id
     */
	virtual void clearAssignedVehicleId();

    /**
     * getAssignedVehicleId
     * Retrieves assigned vehicle id
     * @return assigned vehicle id
     */
	virtual int getAssignedVehicleId();

    /**
     * setStatus
     * Sets the customer status
     * @param s customer status to be set
     */
	virtual void setStatus(Status s);

    /**
     * getStatus
     * Retrieves the current customer status
     * @return customer status
     */
	virtual Status getStatus() const;

    /**
     * getLocationId
     * Retrives the current location id
     * @return location id
     */
    virtual int getLocationId(); 

    /**
     * setLocationId
     * sets the location id
     * @param locId_ - location id
     */
    virtual void setLocationId(int locId_);
    
    /// convenience functions
    /**
     * setInVehicle
     * sets in vehicle status
     */
	virtual void setInVehicle();

    /**
     * isInVehicle
     * Looks for in vehicle status
     * @return true if status is IN_VEHICLE
     */
	virtual bool isInVehicle();
    
private:
    /// Customer status
    Status status;

    /// Vehicle id
    int vehId;
    
    /// Location Id
    int locId;
};

typedef Customer::Status CustomerStatus;

}

}
