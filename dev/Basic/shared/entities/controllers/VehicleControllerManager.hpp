/*
 * VehicleControllerManager.hpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef VehicleControllerManager_HPP_
#define VehicleControllerManager_HPP_
#include <boost/shared_ptr.hpp>
#include <map>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "VehicleController.hpp"

namespace sim_mob
{

class VehicleControllerManager : public Agent {
protected:
	explicit VehicleControllerManager(
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered)
		  : Agent(mtxStrat, -1)
	{
	}

public:
	/**
	 * Initialize a single VehicleControllerManager with the given MutexStrategy
	 */
	static bool RegisterVehicleControllerManager(
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	~VehicleControllerManager();
	
	/**
	 * Get global singleton instance of VehicleControllerManager
	 * @return pointer to the global instance of VehicleControllerManager
	 */
	static VehicleControllerManager* GetInstance();

	/**
	 * Checks if the VehicleControllerManager instance exists
	 */
	static bool HasVehicleControllerManager();

	/**
	 * Adds a VehicleController to the list of controllers
	 * @param  id ID of controller
	 * @param  vc VehicleController
	 * @return    Success
	 */
	bool addVehicleController(unsigned int id, VehicleController* vc);

	/**
	 * Removes a VehicleController 
	 * @param  id ID of the VehicleController to remove
	 * @return    Success
	 */
	bool removeVehicleController(unsigned int id);

	/**
	 * Adds a vehicle driver to the appropriate controllers
	 * @param person Driver to be added
	 */
	void addVehicleDriver(Person* person);

	/**
	 * Removes the vehicle driver from the appropriate controllers
	 * @param person Driver to be removed
	 */
	void removeVehicleDriver(Person* person);

	/**
	 * Signals are non-spatial in nature.
	 */
	bool isNonspatial();

	/**
	 * Returns a list of enabled controllers
	 */
	std::map<unsigned int, VehicleController*> getControllers();

protected:
	/**
	 * Inherited from base class agent to initialize
	 * parameters for VehicleControllerManager
	 */
	Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Inherited from base class to output result
	 */
	void frame_output(timeslice now);

private:
	/** Store list of controllers */
	std::map<unsigned int, VehicleController*> controllers;

	/** Store self instance */
	static VehicleControllerManager* instance;
};
}
#endif /* VehicleControllerManager_HPP_ */


