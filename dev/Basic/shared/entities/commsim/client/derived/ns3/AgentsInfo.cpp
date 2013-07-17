/*
 * AgentsInfo.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: vahid
 */

#include "AgentsInfo.hpp"
#include <json/json.h>
#include "entities/Agent.hpp"
#include "entities/commsim/service/services.hpp"
#include "entities/commsim/serialization/Serialization.hpp"

namespace sim_mob {

AgentsInfo::AgentsInfo() {
	// TODO Auto-generated constructor stub
}

void AgentsInfo::insertInfo(std::string type, std::vector<sim_mob::Entity*> values)
{
	std::vector<sim_mob::Entity*> &t = all_agents[type];
	t.insert(t.end(), values.begin(), values.end());
}

void AgentsInfo::insertInfo(std::string type, sim_mob::Entity* value) {
	all_agents[type].push_back(value);
}
std::string AgentsInfo::ToJSON()
{
	Json::Value jPacket,jHeader,jArray,jElement;
	std::map<std::string , std::vector<sim_mob::Entity*> >::iterator
	it(all_agents.begin()), it_end(all_agents.end());

	for(; it != it_end; it++)
	{
		std::vector<sim_mob::Entity*>::iterator itv(it->second.begin()),
				itv_end(it->second.end());
		for(; itv != itv_end; itv++)
		{
			jElement.clear();
			jElement["AGENT_ID"] = (*itv)->getId();
			jElement["AGENT_TYPE"] = it->first;
			jArray.append(jElement);
		}
	}

	pckt_header pHeader_("1");
	jHeader = JsonParser::createPacketHeader(pHeader_);
	jElement.clear();//to make a message
	msg_header mHeader_("0", "SIMMOBILITY", "AGENTS_INFO");
	jElement = JsonParser::createMessageHeader(mHeader_);
	jElement["DATA"].append(jArray);
	jPacket["PACKET_HEADER"] = jHeader;
	jPacket["DATA"].append(jElement);

	//convert the jsoncpp packet to a json string
	Json::FastWriter writer;
	return writer.write(jPacket);
}
AgentsInfo::~AgentsInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
