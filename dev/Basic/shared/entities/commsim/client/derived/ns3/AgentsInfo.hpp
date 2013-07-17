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
	std::map<std::string , std::vector<sim_mob::Entity*> > all_agents; //map<type(active,pending) , std::vector<agents>
public:
	AgentsInfo();
	void insertInfo(std::string type, std::vector<sim_mob::Entity*> values);
	void insertInfo(std::string type, sim_mob::Entity*);
	std::string ToJSON();
	virtual ~AgentsInfo();
};

} /* namespace sim_mob */
#endif /* AGENTSINFO_HPP_ */
