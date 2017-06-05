/*
 * MobilityServiceControllerMessage.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha, Andrea Araldo
 */

#include "MobilityServiceControllerMessage.hpp"

namespace sim_mob
{
	const Schedule& SchedulePropositionMessage::getSchedule() const
	{
		return schedule;
	}

}
