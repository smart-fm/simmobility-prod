
#pragma once
#include "behavioral/lua/PredayLuaModel.hpp"

namespace sim_mob {

namespace medium {

        class PredayLuaProvider {
			public:
				/**
				 * Gets the Preday lua model.
				 *
				 * Attention: you should not hold this instance.
				 * This provider will give you and instance based on
				 *  current thread context.
				 *
				 * @return Lua preday model reference.
				 */
				static const PredayLuaModel& getPredayModel();
        };
    }
}

