//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "AgentsInfo.hpp"
#include <json/json.h>
#include "entities/Agent.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"
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
	//Json::Value jPacket,jHeader,jArray_add,JArray_delete,jElement;
	//Json::Value* jArray_temp;
	//sim_mob::Entity* t;

	std::vector<unsigned int> addAgIds;
	std::vector<unsigned int> remAgIds;
	for(std::map<Mode, std::set<sim_mob::Entity*> >::iterator it = all_agents.begin(); it != all_agents.end(); it++) {
		std::vector<unsigned int>* currVec = nullptr;
		switch(it->first) {
			case ADD_AGENT: {
				currVec = &addAgIds;
				break;
			}
			case REMOVE_AGENT: {
				currVec = &remAgIds;
				break;
			}
			default: { throw std::runtime_error("Unknown add/rem agent enum type."); }
		}

		for (std::set<Entity*>::const_iterator eIt=it->second.begin(); eIt!=it->second.end(); eIt++) {
		//BOOST_FOREACH(t, it->second) {
			currVec->push_back((*eIt)->getId());
			//jElement.clear();
			//jElement["AGENT_ID"] = (*eIt)->getId();
			//jArray_temp->append(jElement);
		}
	}
	/*pckt_header pHeader_(1, "0");
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
	jPacket["DATA"].append(jElement);*/

	//convert the jsoncpp packet to a json string
	//Json::FastWriter writer;
	//std::string res =  writer.write(jPacket);
//std::cout << "AGENTS_INFO : ###" << res << "###" << std::endl;
	//return res;

	return CommsimSerializer::makeAgentsInfo(addAgIds, remAgIds);
}
AgentsInfo::~AgentsInfo() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
