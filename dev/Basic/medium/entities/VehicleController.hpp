/*
 * VehicleController.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#ifndef VehicleController_HPP_
#define VehicleController_HPP_
#include <map>
#include <vector>

#include "entities/Agent.hpp"
#include "entities/roles/driver/TaxiDriverFacets.hpp"
#include "entities/roles/driver/TaxiDriver.hpp"

namespace sim_mob
{
namespace medium
{

struct Request
{
	const sim_mob::Node* startNode;
	const sim_mob::Node* destinationNode;
};

struct Response
{
	bool success;
};

class VehicleController: public sim_mob::Agent {
protected:
	explicit VehicleController(int id = -1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) :
			Agent(mtxStrat, id)
	{
			startTime = 0; // vehicle controllers are alive for the entire duration of the simulation
	}

public:
	/**
	 * Initialize a single VehicleController with the given start time and MutexStrategy.
	 */
	static bool RegisterVehicleController(int id = -1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);

	virtual ~VehicleController();
	
	/**
	 * get global singleton instance of VehicleController
	 * @return pointer to the global instance of VehicleController
	 */
	static VehicleController* GetInstance();

	/**
	 * checks if the vehicle controller instance exists
	 */
	static bool HasVehicleController();

	/**
	 * Initialize all vehicle controller objects based on the parameters.
	 */
	void initializeVehicleController(std::set<sim_mob::Entity*>& agentList);

	/**
	 * [addTaxiDriver description]
	 * @param driver [description]
	 */
	void addTaxiDriver(Person_MT* person);

	/**
	 * [removeTaxiDriver description]
	 * @param driver [description]
	 */
	void removeTaxiDriver(Person_MT* person);

	// TODO: Create a list of requests?

	/**
	 * [assignVehicleToRequest description]
	 * @param  request [description]
	 * @return         [description]
	 */
	virtual Response assignVehicleToRequest(Request request);

	/**
	 * Signals are non-spatial in nature.
	 */
	virtual bool isNonspatial();

protected:
	/**
	 * inherited from base class agent to initialize parameters for vehicle controller
	 */
	virtual Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * inherited from base class to update this agent
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * inherited from base class to output result
	 */
	virtual void frame_output(timeslice now);

private:
	/**store driver information*/
	std::vector<Person_MT*> taxiDrivers;
	/**store self instance*/
	static VehicleController* instance;
};
}
}
#endif /* VehicleController_HPP_ */

