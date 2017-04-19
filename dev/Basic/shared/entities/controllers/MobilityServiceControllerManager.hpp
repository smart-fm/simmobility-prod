/*
 * MobilityServiceControllerManager.hpp
 *
 *  Created on: Apr 12, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef MobilityServiceControllerManager_HPP_
#define MobilityServiceControllerManager_HPP_
#include <boost/shared_ptr.hpp>
#include <map>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "MobilityServiceController.hpp"

namespace sim_mob
{

class MobilityServiceControllerManager : public Agent {
protected:
	explicit MobilityServiceControllerManager(
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered)
		  : Agent(mtxStrat, -1)
	{
	}

public:
	/**
	 * Initialize a single MobilityServiceControllerManager with the given MutexStrategy
	 */
	static bool RegisterMobilityServiceControllerManager(
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	~MobilityServiceControllerManager();
	
	/**
	 * Get global singleton instance of MobilityServiceControllerManager
	 * @return pointer to the global instance of MobilityServiceControllerManager
	 */
	static MobilityServiceControllerManager* GetInstance();

	/**
	 * Checks if the MobilityServiceControllerManager instance exists
	 */
	static bool HasMobilityServiceControllerManager();

	/**
	 * Adds a MobilityServiceController to the list of controllers
	 * @param  id			ID of controller
	 * @param  controller	MobilityServiceController
	 * @return    			Success
	 */
	bool addMobilityServiceController(unsigned int id, MobilityServiceController* controller);

	/**
	 * Removes a MobilityServiceController 
	 * @param  id ID of the MobilityServiceController to remove
	 * @return    Success
	 */
	bool removeMobilityServiceController(unsigned int id);

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
	std::map<unsigned int, MobilityServiceController*> getControllers();

protected:
	/**
	 * Inherited from base class agent to initialize
	 * parameters for MobilityServiceControllerManager
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
	std::map<unsigned int, MobilityServiceController*> controllers;

	/** Store self instance */
	static MobilityServiceControllerManager* instance;
};
}
#endif /* MobilityServiceControllerManager_HPP_ */



