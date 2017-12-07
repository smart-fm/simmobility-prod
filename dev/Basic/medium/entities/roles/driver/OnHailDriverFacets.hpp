//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"
#include "util/Utils.hpp"

namespace sim_mob
{
namespace medium
{

namespace
{

enum class BehaviourDecision
{
	CRUISE, DRIVE_TO_TAXISTAND, TAKE_A_BREAK, END_SHIFT
};

std::ostream& operator<<(std::ostream &strm, BehaviourDecision &decision)
{
	switch (decision)
	{
	case BehaviourDecision::CRUISE:
		return strm << "CRUISE";

	case BehaviourDecision ::DRIVE_TO_TAXISTAND:
		return strm << "DRIVE_TO_TAXISTAND";

	case BehaviourDecision ::TAKE_A_BREAK:
		return strm << "TAKE_A_BREAK";

	case BehaviourDecision ::END_SHIFT:
		return strm << "END_SHIFT";
	}
}

}

class OnHailDriver;

class OnHailDriverMovement : public DriverMovement
{
private:
	/**The on hail driver to which this movement object belongs*/
	OnHailDriver *onHailDriver;

	/**Indicates the current node of the vehicle.*/
	const Node *currNode;

	/**The taxi stand most recently chosen by the driver*/
	const TaxiStand *chosenTaxiStand;

protected:
	/**
	 * Sets the current lane to lane infinity of the current segment. This method must be called when
	 * the driver is exiting a taxi stand
	 */
	void resetDriverLaneAndSegment();

public:
	OnHailDriverMovement();
	virtual ~OnHailDriverMovement();

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
	 * Handles the movement into the next segment
	 * @param params the driver update parameters
	 * @return true if successfully moved to next segment, false otherwise
	 */
	virtual bool moveToNextSegment(DriverUpdateParams &params);

	/**
	 * This method performs the actions required by the decison given by the behaviour models
	 * @param decision the decision from the driver behaviour models
	 */
	void performDecisionActions(BehaviourDecision decision);

	/**
	 * This method looks up the path for driving to the taxi stand from the current
	 * position and begins the drive towards it
	 * @param taxiStand the chosen taxi stand
	 */
	void beginDriveToTaxiStand(const TaxiStand *taxiStand);

	/**
	 * This method looks up the path for driving to the node from the current
	 * position and begins the drive towards it
	 * @param node the chosen node to cruise to
	 */
	void beginCruising(const Node *node);

	/**
	 * This method looks up the path for driving to the destination of the passenger
	 * from the current position and begins the drive towards it
	 * @param person the person who is the passenger
	 */
	void beginDriveWithPassenger(Person_MT *person);

	/**
	 * This method performs the steps required for the taxi driver to stay stationary
	 * at the taxi stand
	 * @param params the driver update parameters
	 */
	void beginQueuingAtTaxiStand(DriverUpdateParams &params);

	const OnHailDriver* getOnHailDriver() const
	{
		return onHailDriver;
	}

	void setOnHailDriver(OnHailDriver *driver)
	{
		onHailDriver = driver;
	}

	const Node* getCurrentNode() const
	{
		return currNode;
	}

	const TaxiStand* getChosenTaxiStand() const
	{
		return chosenTaxiStand;
	}
};

class OnHailDriverBehaviour : public DriverBehavior
{
private:
	/**The on hail driver to which this movement object belongs*/
	OnHailDriver *onHailDriver;

	/**Records the amount of time (in seconds) the driver has spent cruising in the current stint*/
	int currCruisingStintTime;

	/**The maximum time a driver can cruise at a stretch*/
	const int maxCruisingStintTime;

	/**Records the amount of time (in seconds) the driver has spent queuing at the taxi stand in the current stint*/
	int currQueuingStintTime;

	/**The maximum time a driver can queue for at a taxi stand at a stretch*/
	const int maxQueuingStintTime;

public:
	OnHailDriverBehaviour() : currCruisingStintTime(0), maxCruisingStintTime(Utils::generateInt(600,900)),
	                          currQueuingStintTime(0), maxQueuingStintTime(Utils::generateInt(300,600))
	{}

	virtual ~OnHailDriverBehaviour()
	{}

	/**
	 * This method calls the behaviour models that enable the driver to make the decision
	 * for the next step
	 * @return The chosen decision: CRUISE, DRIVE_TO_TAXISTAND or TAKE_A_BREAK
	 */
	BehaviourDecision makeBehaviourDecision() const;

	/**
	 * This method chooses the taxi stand to drive to
	 * @return the chosen TaxiStand
	 */
	const TaxiStand* chooseTaxiStand()const;

	/**
	 *This method chooses the node to cruise to
	 * @return the chosen Node
	 */
	const Node *chooseNode() const;

	/**
	 * Checks if the driver's shift has ended
	 * @return true if the driver's shift has ended, false otherwise
	 */
	bool hasDriverShiftEnded() const;

	/**
	 * Increments the time for which the driver has been cruising in the current stint
	 */
	void incrementCruisingStintTime();

	/**
	 * Increments the time for which the driver has been queuing at the taxi stand in the current stint
	 */
	void incrementQueuingStintTime();

	/**
	 * Resets the cruising stint time to 0
	 */
	void resetCruisingStintTime()
	{
		currCruisingStintTime = 0;
	}

	/**
	 * Resets the queuing stint time to 0
	 */
	void resetQueuingStintTime()
	{
		currQueuingStintTime = 0;
	}

	/**
	 * Indicates whether the cruising stint has been completed
	 * @return true if the stint is complete, else false
	 */
	bool isCruisingStintComplete() const
	{
		return (currCruisingStintTime >= maxCruisingStintTime);
	}

	/**
	 * Indicates whether the queuing stint has been completed
	 * @return true if the stint is complete, else false
	 */
	bool isQueuingStintComplete() const
	{
		return (currQueuingStintTime >= maxQueuingStintTime);
	}

	const OnHailDriver* getOnHailDriver() const
	{
		return onHailDriver;
	}

	void setOnHailDriver(OnHailDriver *driver)
	{
		onHailDriver = driver;
	}
};

}
}

