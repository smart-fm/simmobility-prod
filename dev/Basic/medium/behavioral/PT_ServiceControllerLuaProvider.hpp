
#pragma once
#include "entities/roles/driver/TrainDriver.hpp"


namespace sim_mob
{
      namespace medium
	  {
      class PT_ServiceControllerLuaProvider
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
				static ServiceController * getPTRC_Model();
        };
	  }
}

