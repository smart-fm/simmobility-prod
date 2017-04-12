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
#include <mutex>

#include "entities/Agent.hpp"
#include "message/Message.hpp"

namespace sim_mob
{

class VehicleController: public Agent {
protected:
	explicit VehicleController(int id = -1,
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		int tickRefresh = 0,
		double shareThreshold = 0) : Agent(mtxStrat, id)
	{
			startTime = 0; // vehicle controllers are alive for the entire duration of the simulation

			currIntTick = 0;
			tickThreshold = tickRefresh;
			timedelta = shareThreshold;
	}

public:
	struct VehicleRequest
	{
		const timeslice currTick;
		const std::string personId;
		const unsigned int startNodeId;
		const unsigned int destinationNodeId;
	};

	enum MessageParsingResult
	{
		PARSING_SUCCESS = 0,
		PARSING_FAILED,
		PARSING_RETRY
	};

	/**
	 * Initialize a single VehicleController with the given start time and MutexStrategy.
	 */
	static bool RegisterVehicleController(int id = -1,
		const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered,
		int tickRefresh = 0,
		double shareThreshold = 0);

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
	 * [addVehicleDriver description]
	 * @param person [description]
	 */
	void addVehicleDriver(Person* person);

	/**
	 * [removeVehicleDriver description]
	 * @param person [description]
	 */
	void removeVehicleDriver(Person* person);

	/**
	 * Signals are non-spatial in nature.
	 */
	bool isNonspatial();

protected:
	/**
	 * inherited from base class agent to initialize parameters for vehicle controller
	 */
	Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * inherited from base class to output result
	 */
	void frame_output(timeslice now);

    void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	virtual int assignVehicleToRequest(VehicleRequest request);

	virtual void assignSharedVehicles(std::vector<Person*> drivers, std::vector<VehicleRequest> requests, timeslice now);

private:
	int currIntTick;
	int tickThreshold;
	double timedelta;
	/**store driver information*/
	std::vector<Person*> vehicleDrivers;
	/**store message information*/
	std::vector<VehicleRequest> requestQueue;
	/**store self instance*/
	static VehicleController* instance;
};
}
#endif /* VehicleController_HPP_ */







