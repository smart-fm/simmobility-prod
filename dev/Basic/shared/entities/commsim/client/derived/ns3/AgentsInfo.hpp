/*
 * AgentsInfo.hpp
 *
 *  Created on: Jul 16, 2013
 *      Author: vahid
 */

#ifndef AGENTSINFO_HPP_
#define AGENTSINFO_HPP_
#include <map>
#include <set>
#include <iostream>
namespace sim_mob {
class Entity;

class AgentsInfo {

public:
	enum Mode {
		ADD_AGENT,
		REMOVE_AGENT
	};
	AgentsInfo();
	void insertInfo(std::map<Mode, std::set<sim_mob::Entity*> > &values);
	void insertInfo(Mode mode, std::set<sim_mob::Entity*>  &values);
	void insertInfo(Mode,sim_mob::Entity*);
	std::string toJson();
	virtual ~AgentsInfo();
private:
	std::map<Mode, std::set<sim_mob::Entity*> > all_agents; //map<type(active,pending) , std::vector<agents>
};

} /* namespace sim_mob */
#endif /* AGENTSINFO_HPP_ */
