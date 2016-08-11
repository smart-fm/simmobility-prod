/*
 * Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
 * Licensed under the terms of the MIT License, as described in the file:
 * license.txt   (http://opensource.org/licenses/MIT)
 * File:   MRTMovement_ServiceController.hpp
 * Author: Jabir <jabir@smart.mit.edu>
 *
 */

#pragma once
#include "ServiceController.hpp"

namespace sim_mob
{
	namespace medium
	{
		/*Class:   MRTMovement_ServiceController
		 * Author: Jabir <jabir@smart.mit.edu>
		 */
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

