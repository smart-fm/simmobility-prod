
#pragma once
#include "behavioral/lua/PredayLogsumLuaModel.hpp"

namespace sim_mob
{
class PredayLogsumLuaProvider
{
public:
	/**
	 * Gets the Preday lua model.
	 *
	 * Attention: you should not hold this instance.
	 * This provider will give you an instance based on
	 *  current thread context.
	 *
	 * @return Lua preday model reference.
	 */
	static const PredayLogsumLuaModel& getPredayModel();
};
}

