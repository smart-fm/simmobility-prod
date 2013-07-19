/*
 * AgentsInfo.hpp
 *
 *  Created on: Jul 16, 2013
 *      Author: vahid
 */

#ifndef AGENTSINFO_HPP_
#define AGENTSINFO_HPP_
#include <map>
#include <vector>
#include <iostream>
namespace sim_mob {
class Entity;

class AgentsInfo {
	std::vector<sim_mob::Entity*> all_agents; //map<type(active,pending) , std::vector<agents>
public:
	AgentsInfo();
	void insertInfo(std::vector<sim_mob::Entity*> values);
	void insertInfo(sim_mob::Entity*);
	std::string toJson();
	virtual ~AgentsInfo();
};

} /* namespace sim_mob */
#endif /* AGENTSINFO_HPP_ */
