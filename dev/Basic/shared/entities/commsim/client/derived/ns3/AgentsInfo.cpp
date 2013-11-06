//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AgentsInfo.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: vahid
 */

#include "AgentsInfo.hpp"
#include <json/json.h>
#include "entities/Agent.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/serialization/Serialization.hpp"
#include <boost/foreach.hpp>

namespace sim_mob {

AgentsInfo::AgentsInfo() {
	// TODO Auto-generated constructor stub
}

void AgentsInfo::insertInfo(std::map<Mode, std::set<sim_mob::Entity*> > &values)
{
	std::map<Mode, std::set<sim_mob::Entity*> >::iterator it(values.begin()), it_end(values.end());
	for(; it != it_end; it++ )
	{
		all_agents[it->first].insert(it->second.begin(), it->second.end());
	}

}

void AgentsInfo::insertInfo(Mode mode, std::set<sim_mob::Entity*>  &values)
{
//	std::set<sim_mob::Entity*>::iterator it(values.begin()), it_end(values.end());
//	for(; it != it_end; it++ )
//	{
		all_agents[mode].insert(values.begin(), values.end());
//	}

}

void AgentsInfo::insertInfo(Mode mode,sim_mob::Entity* value) {
	all_agents[mode].insert(value);
}
std::string AgentsInfo::toJson()
{
	Json::Value jPacket,jHeader,jArray_add,JArray_delete,jElement;
	Json::Value *jArray_temp;
	std::map<Mode, std::set<sim_mob::Entity*> >::iterator
	it(all_agents.begin()), it_end(all_agents.end());
	sim_mob::Entity* t;
	for(; it != it_end; it++)
	{

			switch(it->first)
			{
			case ADD_AGENT:{
				jArray_temp = &jArray_add;
				break;
			}
			case REMOVE_AGENT:{
				jArray_temp = &JArray_delete;
				break;
			}
			}

			BOOST_FOREACH(t, it->second)
			{
				jElement.clear();
				jElement["AGENT_ID"] = t->getId();
				jArray_temp->append(jElement);
			}
	}
	pckt_header pHeader_("1");
	jHeader = JsonParser::createPacketHeader(pHeader_);
	jElement.clear();//to make a message
	msg_header mHeader_("0", "SIMMOBILITY", "AGENTS_INFO", "SYS");
	jElement = JsonParser::createMessageHeader(mHeader_);
	if(jArray_add.size())
	{
		jElement["ADD"] = jArray_add;
	}
	if(JArray_delete.size())
	{
		jElement["DELETE"] = JArray_delete;
	}

	jPacket["PACKET_HEADER"] = jHeader;
	jPacket["DATA"].append(jElement);

	//convert the jsoncpp packet to a json string
	Json::FastWriter writer;
	std::string res =  writer.write(jPacket);
//	std::cout << "AGENTS_INFO : '" << res << "'" << std::endl;
	return res;
}
AgentsInfo::~AgentsInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
