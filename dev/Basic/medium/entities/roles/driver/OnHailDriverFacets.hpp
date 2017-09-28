//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"

namespace sim_mob
{
namespace medium
{

namespace
{

enum class BehaviourDecision
{
	CRUISE, DRIVE_TO_TAXISTAND, TAKE_A_BREAK
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
	default:
	std::stringstream msg;
		msg << "Unknown enum value for BehaviourDecision " << (int)decision;
		throw std::runtime_error(msg.str());
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
};

class OnHailDriverBehaviour : public DriverBehavior
{
private:
	/**The on hail driver to which this movement object belongs*/
	OnHailDriver *onHailDriver;

public:
	OnHailDriverBehaviour()
	{}

	virtual ~OnHailDriverBehaviour()
	{}

	/**
	 * This method calls the behaviour models that enable the driver to make the decision
	 * for the next step
	 * @return The chosen decision: CRUISE, DRIVE_TO_TAXISTAND or TAKE_A_BREAK
	 */
	BehaviourDecision makeBehaviourDecision();

	/**
	 * This method chooses the taxi stand to drive to
	 * @return the chosen TaxiStand
	 */
	const TaxiStand* chooseTaxiStand();

	/**
	 *This method chooses the node to cruise to
	 * @return the chosen Node
	 */
	const Node* chooseNode();

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

