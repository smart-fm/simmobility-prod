//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/params/PersonParams.hpp"
#include "lua/LuaModel.hpp"

namespace sim_mob
{
/**
 * Class to interface with the behavioral models specified in Lua scripts
 * This class serves as an interface to possibly any lua model called from MT withinday code
 *
 * \author Harish Loganathan
 */
class WithindayLuaModel : public lua::LuaModel
{
public:
	WithindayLuaModel();
	virtual ~WithindayLuaModel();

	/**
	 * Computes logsums for work tours with fixed work location by invoking corresponding functions in tour mode (for work) model
	 *
	 * @param personParams object containing person and household related variables. logsums will be updated in this object
	 * @param tourModeDestinationParams parameters specific to tour mode model
	 */
	void chooseMode(PersonParams& personParams/*, LogsumTourModeParams& tourModeParams*/) const;

private:
    /**
     * Inherited from LuaModel
     */
    void mapClasses();
};

/**
 * Thread specific lua provider class for withinday models
 *
 * \author Harish Loganathan
 */
class WithindayLuaProvider
{
public:
	/**
	 * Fetches the Withinday lua model.
	 *
	 * NOTE: you should not hold this instance.
	 *       This provider will give you an instance based on current thread context.
	 *
	 * @return withinday Lua model reference.
	 */
	static const WithindayLuaModel& getWithindayModel();
};
} //end sim_mob


