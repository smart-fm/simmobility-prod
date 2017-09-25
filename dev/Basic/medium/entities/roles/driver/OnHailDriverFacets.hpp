//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "DriverFacets.hpp"

namespace sim_mob
{
namespace medium
{

class OnHailDriver;

class OnHailDriverMovement : public DriverMovement
{
private:
	/**The on hail driver to which this movement object belongs*/
	const OnHailDriver *onHailDriver;

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

	const OnHailDriver* getOnHailDriver() const
	{
		return onHailDriver;
	}

	void setOnHailDriver(const OnHailDriver *driver)
	{
		onHailDriver = driver;
	}
};

class OnHailDriverBehaviour : public DriverBehavior
{
public:
	OnHailDriverBehaviour()
	{}

	virtual ~OnHailDriverBehaviour()
	{}
};

}
}

