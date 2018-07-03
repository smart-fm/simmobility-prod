//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"

namespace sim_mob
{

class OnCallDriver;

class OnCallDriverMovement : public DriverMovement
{
private:
	/**The on call driver to which this movement object belongs*/
	OnCallDriver *onCallDriver;

	/**Indicates the current node of the vehicle.*/
	const Node *currNode;
    bool pickedUpPasssenger = false;
    bool droppedOffPassenger = false;
    bool pickedUpAnotherPasssenger = false;
    bool droppedOffAnotherPassenger = false;
    bool passengerWaitingtobeDroppedOff = false;

    bool leftPickupStopPoint=true;
    bool leftDropOffStopPoint=true;

protected:
	/**
	 * This method looks up the path for driving to the node from the current
	 * position and begins the drive towards it
	 * @param node the chosen node to cruise to
	 */
	void beginCruising(const Node *node);
	/**
	 * This method enables the driver to continue cruising, one link at a time
	 * @param fromNode the node the driver has reached when the defined path has been traversed
	 * and wants to continue cruising
	 */
	void continueCruising(const Node *fromNode);

public:
	OnCallDriverMovement();
	virtual ~OnCallDriverMovement();

	/**
	 * This method performs the initialisation required by the movement
	 */
	virtual void frame_init();

	/**
	 * Performs the movement related updates for the agent in the OnHailDriver role
	 * for the current tick
	 */
	virtual void frame_tick();

	/**
	 * Collects the movement related output for the agent in the OnHailDriver role
	 * for the current tick
	 * @return the output of the current tick
	 */
	virtual std::string frame_tick_output();

	/**
	 * This method allows us to perform the task in the assigned schedule
	 */
	virtual void performScheduleItem();
    /**
     * Handles the movement into the next segment
     * @param params the driver update parameters
     * @return true if successfully moved to next segment, false otherwise
     */
  //  virtual bool moveToNextSegment(DriverUpdateParams &params);

	void setOnCallDriver(OnCallDriver *driver)
	{
		onCallDriver = driver;
	}

	const Node* getCurrentNode() const
	{
		return currNode;
	}

        Vehicle* initialisePath(bool createVehicle);

    void beginDriveToPickUpPoint(const Node *pickupNode);
    /**
     * This method looks up the path for driving to the drop off node from the current
     * position and begins the drive towards it
     * @param dropOffNode the node at which the passenger is to be dropped off
     */
    void beginDriveToDropOffPoint(const Node *dropOffNode);

    const std::unordered_set<int>  blackListedNodes={16657,17515,23180,17517,23182,15035,23184,23185,18530,13691,11340,17593,17704,11341,11741,13280,23190,23191,10076,11986,14576,20277,13280,17078,23183,23189,13514,20657,14348,10917,13392,20659
    ,18938,10549,19690,11506,20401,21067,21636,21893,15942,1380011881,15492,20421,21058,13911,16169,10932,14056,21084,22267,13362,23127,1380012966};
                                                     //20659,18938,21067,13691,23180,21058,14104,14348,17517,18530,16657,10917,20657,15492,11506,15035,13910,13392,23191,10549,
                                                     //19690,13543,17593,10206,11986,19415,13514,23184,23185,14576,11340,11341,23182,11741,23190,13462,10076,17515,17704,20277,13362};

};

class OnCallDriverBehaviour : public DriverBehavior
{
private:
	/**The on call driver to which this behaviour object belongs*/
	OnCallDriver *onCallDriver;

public:
	OnCallDriverBehaviour()
	{
	}

	~OnCallDriverBehaviour()
	{
	}

	/**
	 * This method chooses a random downstream node to cruise to. This method is intended to be
	 * used when the driver is supposed to continue cruising (after arriving at the node specified
	 * by the controller) or from frame_init, where the driver has no initial path
	 * @param fromNode the node from which we want to move/continue moving
	 * @return the chosen Node
	 */
	const Node *chooseDownstreamNode(const Node *fromNode) const;

    const Node* chooseRandomNode() const;

	/**
	 * Checks if the driver's shift has ended
	 * @return true if the driver's shift has ended, false otherwise
	 */
	bool hasDriverShiftEnded() const;

	void setOnCallDriver(OnCallDriver *driver)
	{
		onCallDriver = driver;
	}
};

}