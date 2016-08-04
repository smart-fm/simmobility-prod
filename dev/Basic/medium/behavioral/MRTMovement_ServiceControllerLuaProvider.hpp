/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   PT_ServiceController.hpp
 * Author: Jabir <jabir@smart.mit.edu>
 *
 */

#pragma once
#include "ServiceController.hpp"

namespace sim_mob
{
      namespace medium
	  {
      class MRTMovement_ServiceControllerLuaProvider
        {
			public:
				/**
				 * Gets the ServiceController reference
				 *
				 * Attention: you should not hold this instance.
				 * This provider will give you an instance based on current thread context.
				 *
				 * @return Lua PT service controller reference.
				 */
				static ServiceController * getPTRC_Model();
        };
	  }
}

