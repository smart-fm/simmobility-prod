//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"

namespace sim_mob
{
namespace medium
{

class OnCallDriver;

class OnCallDriverMovement : public DriverMovement
{
private:
	/**The on call driver to which this movement object belongs*/
	OnCallDriver *onCallDriver;

	/**Indicates the current node of the vehicle.*/
	const Node *currNode;

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

	void setOnCallDriver(OnCallDriver *driver)
	{
		onCallDriver = driver;
	}

	const Node* getCurrentNode() const
	{
		return currNode;
	}
};

class OnCallDriverBehaviour : public DriverBehavior
{
public:
	OnCallDriverBehaviour()
	{
	}

	~OnCallDriverBehaviour()
	{
	}
};

}
}
