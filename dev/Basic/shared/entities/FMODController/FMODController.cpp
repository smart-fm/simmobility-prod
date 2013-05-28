/*
 * FMODController.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "FMODController.hpp"
#include "Message.hpp"

namespace sim_mob {

namespace FMOD
{

FMODController::~FMODController() {
	// TODO Auto-generated destructor stub
}

bool FMODController::frame_init(timeslice now)
{
	return true;
}

Entity::UpdateStatus FMODController::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void FMODController::frame_output(timeslice now)
{

}

bool FMODController::Initialzie(std::vector<sim_mob::Entity*>& all_agents)
{
	Msg_Request request;
	std::string msg = request.BuildToString();
	std::cout << msg << std::endl;
	return true;
}

}

} /* namespace sim_mob */
