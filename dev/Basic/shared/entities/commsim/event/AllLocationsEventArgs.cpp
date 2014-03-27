//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "AllLocationsEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"

using namespace sim_mob;

sim_mob::AllLocationsEventArgs::AllLocationsEventArgs(const AgentsList& registeredAgents) : registeredAgents(registeredAgents)
{
}

sim_mob::AllLocationsEventArgs::~AllLocationsEventArgs()
{
}

std::string sim_mob::AllLocationsEventArgs::serialize() const
{
	std::map<unsigned int, DPoint> allLocs;

	//NOTE: I am fairly sure that this is only ever called in a thread-safe context. ~Seth
	const AgentsList::type& aList = registeredAgents.getAgents();
	for (sim_mob::AgentsList::type::const_iterator it=aList.begin(); it!=aList.end(); it++) {
		allLocs[it->first->getId()] = DPoint(it->first->xPos.get(), it->first->yPos.get());
	}

	return CommsimSerializer::makeAllLocations(allLocs);
}

/*void sim_mob::AllLocationsEventArgs::TOJSON(sim_mob::Agent* agent,Json::Value &loc)const
{
	const Json::Value & t = JsonParser::makeLocationArrayElement(agent->getId(),agent->xPos.get(), agent->yPos.get());
	loc["LOCATIONS"].append(t);
}*/

/*Json::Value sim_mob::AllLocationsEventArgs::toJSON()const
{
	Json::Value loc;
	loc = JsonParser::createMessageHeader(msg_header("0", "SIMMOBILITY", "ALL_LOCATIONS_DATA", "SYS"));
	//for_each_agent comes from a self contained class (AgentsList) which is thread safe
	//in order to use for_each_agent(), we should send each agent to another function
	boost::function<void(sim_mob::Agent*)> Fn = boost::bind(&AllLocationsEventArgs::TOJSON, this,_1,boost::ref(loc));
	registered_Agents.for_each_agent(Fn);
	return loc;
}
*/
