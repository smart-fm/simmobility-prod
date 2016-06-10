//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/params/PersonParams.hpp"
#include "behavioral/params/WithindayModeParams.hpp"
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
	 * Choose mode for a trip in withinday
	 *
	 * @param personParams object containing person and household related variables. logsums will be updated in this object
	 * @param wdModeParams parameters specific to withinday mode model
	 */
	int chooseMode(PersonParams& personParams, WithindayModeParams& wdModeParams) const;

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


