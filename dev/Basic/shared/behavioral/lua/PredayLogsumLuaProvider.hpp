
#pragma once
#include "behavioral/lua/PredayLogsumLuaModel.hpp"

namespace sim_mob
{
/**
 * Thread specific lua provider class for logsum computations
 *
 * \author Harish Loganathan
 */
class PredayLogsumLuaProvider
{
public:
	/**
	 * Gets the Preday lua model.
	 *
	 * NOTE: you should not hold this instance.
	 *       This provider will give you an instance based on current thread context.
	 *
	 * @return Lua preday model reference.
	 */
	static const PredayLogsumLuaModel& getPredayModel();
};
}

