
#pragma once
#include "PT_RouteChoiceLuaModel.hpp"

namespace sim_mob
{
        class PT_RouteChoiceLuaProvider
        {
			public:
				/**
				 * Gets the public transit route choice lua model.
				 *
				 * Attention: you should not hold this instance.
				 * This provider will give you an instance based on current thread context.
				 *
				 * @return Lua PT route choice model reference.
				 */
				static PT_RouteChoiceLuaModel& getPTRC_Model();
        };
}

