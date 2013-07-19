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

void AgentsInfo::insertInfo(std::vector<sim_mob::Entity*> values)
{
	all_agents.insert(all_agents.end(), values.begin(), values.end());
}

void AgentsInfo::insertInfo(sim_mob::Entity* value) {
	all_agents.push_back(value);
}
std::string AgentsInfo::toJson()
{
	Json::Value jPacket,jHeader,jArray,jElement;
	std::vector<sim_mob::Entity* >::iterator
	it(all_agents.begin()), it_end(all_agents.end());

	for(; it != it_end; it++)
	{
			jElement.clear();
			jElement["AGENT_ID"] = (*it)->getId();
			jArray.append(jElement);
	}

	pckt_header pHeader_("1");
	jHeader = JsonParser::createPacketHeader(pHeader_);
	jElement.clear();//to make a message
	msg_header mHeader_("0", "SIMMOBILITY", "AGENTS_INFO");
	jElement = JsonParser::createMessageHeader(mHeader_);
	jElement["DATA"] = jArray;
//	jElement["DATA"].append(jArray);
	jPacket["PACKET_HEADER"] = jHeader;
	jPacket["DATA"].append(jElement);

	//convert the jsoncpp packet to a json string
	Json::FastWriter writer;
	std::string res =  writer.write(jPacket);
	std::cout << "AGENTS_INFO : '" << res << "'" << std::endl;
	return res;
}
AgentsInfo::~AgentsInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
