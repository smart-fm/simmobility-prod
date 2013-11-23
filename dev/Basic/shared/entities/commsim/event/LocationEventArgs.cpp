//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * LocationEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "LocationEventArgs.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;

sim_mob::LocationEventArgs::LocationEventArgs(const sim_mob::Agent * agent_) :agent(agent_)
{
}

sim_mob::LocationEventArgs::~LocationEventArgs()
{
}

Json::Value sim_mob::LocationEventArgs::ToJSON() const
{
	return JsonParser::makeLocationMessage(agent->xPos.get(), agent->yPos.get());
}


