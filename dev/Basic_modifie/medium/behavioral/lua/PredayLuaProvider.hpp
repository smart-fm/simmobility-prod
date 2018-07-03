//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include "behavioral/lua/PredayLuaModel.hpp"

namespace sim_mob
{
namespace medium
{
/**
 * Thread specific lua provider class
 *
 * \author Harish Loganathan
 */
class PredayLuaProvider
{
public:
	/**
	 * Gets the Preday lua model.
	 *
	 * NOTE: you should not hold this instance.
	 * This provider will give you an instance based on current thread context.
	 *
	 * @return Lua preday model reference.
	 */
	static const PredayLuaModel& getPredayModel();
};
}
}
