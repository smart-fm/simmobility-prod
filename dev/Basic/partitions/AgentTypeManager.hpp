#pragma once
#include "GenConfig.h"

#ifndef SIMMOB_DISABLE_MPI
#include "entities/Agent.hpp"

namespace sim_mob {

/**
 * for receiver to know what is the class structure
 */
enum role_modes {
	No_Role = 0, Driver_Role, Pedestrian_Role, Passenger_Role, Signal_Role
};

/**
 * \author Xu Yan
 */
class AgentTypeManager {
public:
	/**
	 *many classes need the help of AgentTypeManager, just for easy access
	 */
	static AgentTypeManager &
	instance() {
		return instance_;
	}

	sim_mob::role_modes getAgentRoleType(Agent const* agent);

private:
	static AgentTypeManager instance_;
	AgentTypeManager() {
	}
};

}
#endif
