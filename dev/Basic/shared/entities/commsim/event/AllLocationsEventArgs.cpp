//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AllLocationsEventArgs.cpp
 *
 *  Created on: Jul 5, 2013
 *      Author: vahid
 */

#include "AllLocationsEventArgs.hpp"

using namespace sim_mob;

sim_mob::AllLocationsEventArgs::AllLocationsEventArgs(AgentsList  &registered_Agents_):registered_Agents(registered_Agents_)
{
}

sim_mob::AllLocationsEventArgs::~AllLocationsEventArgs()
{
}

void sim_mob::AllLocationsEventArgs::TOJSON(sim_mob::Agent* agent,Json::Value &loc)const
{
	const Json::Value & t = JsonParser::makeLocationArrayElement(agent->getId(),agent->xPos.get(), agent->yPos.get());
	loc["LOCATIONS"].append(t);
}

Json::Value sim_mob::AllLocationsEventArgs::ToJSON()const
{
	Json::Value loc;
	loc = JsonParser::createMessageHeader(msg_header("0", "SIMMOBILITY", "ALL_LOCATIONS_DATA", "SYS"));
	//for_each_agent comes from a self contained class (AgentsList) which is thread safe
	//in order to use for_each_agent(), we should send each agent to another function
	boost::function<void(sim_mob::Agent*)> Fn = boost::bind(&AllLocationsEventArgs::TOJSON, this,_1,loc);
	registered_Agents.for_each_agent(Fn);
	return loc;
}


