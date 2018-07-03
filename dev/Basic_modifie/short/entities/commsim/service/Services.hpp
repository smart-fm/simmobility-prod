//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * services.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once

//include publishing services that you provide in simmobility
#include <map>
#include <string>

namespace sim_mob {


/**
 * Wrapper class created to hold the otherwise-top-level enums/maps in Services.hpp.
 * This class is only named Services as it is the name of the file. Feel free to change it. ~Seth
 */
class Services {
public:
	enum SIM_MOB_SERVICE {
		//Request the bound Agent's location at each time tick.
		//NOTE: This message type will be ignored for non-Android clients.
		SIMMOB_SRV_LOCATION,

		//Request all Agent locations at each time tick.
		//NOTE: This message type will be ignored for non-ns-3 clients.
		SIMMOB_SRV_ALL_LOCATIONS,

		//Sim Mobility will provide the set of all regions and the Agent's current path with this service.
		//Triggers: Every time the Region set changes, it will be re-sent. Every time the Agent's path
		//          (listed as a series of Regions it will travel through) changes, it is re-sent.
		//Practically, this means that the Region set AND Path are sent during time tick 0 (or 1), the Region
		//          set is never re-sent, and the Path set is re-sent only in the case of re-routing.
		//NOTE: There is probably a more efficient way to do this, with the Agent requesting the Region/Path set only
		//      when it needs it. For now, I am trying to do this within the "Services" framework we provide. ~Seth
		//NOTE: This message type will be ignored for non-Android clients.
		SIMMOB_SRV_REGIONS_AND_PATH
	};

	static std::map<std::string, SIM_MOB_SERVICE> ServiceMap;

	static SIM_MOB_SERVICE GetServiceType(std::string type);
};

}


