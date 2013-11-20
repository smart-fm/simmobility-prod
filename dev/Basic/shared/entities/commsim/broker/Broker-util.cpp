//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Broker-util.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"

using namespace sim_mob;

sim_mob::AgentInfo::AgentInfo(Agent * a_, AgentCommUtilityBase * b_, bool v)
	: agent(a_), comm(b_), valid(v), done(false)
{
}


/*******************************************************************
 * ********* Change Container ***************************************
 * ******************************************************************
 */
void sim_mob::AgentsList::insert(Agent * a, AgentCommUtilityBase * b, bool valid )
{
	Lock lock(mutex);
	Agents &aa = data.get<agent_tag>();
	aa.insert(AgentInfo(a,b,valid));
}

void sim_mob::AgentsList::AgentsList::erase(Agent * agent)
{
	Lock lock(mutex);
	Agents &agents = data.get<agent_tag>();
	if(agents.find(agent) != agents.end())
	{
		Print() << "AgentsList::erase test:  agent[" << agent << "] found for deletion" << std::endl;
	} else {
		Print() << "AgentsList::erase test:  agent[" << agent << "] NOT found for deletion" << std::endl;
	}

	agents.erase(agent);
}

void sim_mob::AgentsList::erase(AgentCommUtilityBase * comm)
{
	Lock lock(mutex);
	AgentCommUtilities &comms = data.get<agent_util>();
	comms.erase(comm);
}

void sim_mob::AgentsList::eraseInvalids()
{
	Lock lock(mutex);
	Print() << "AgentsList::eraseInvalids test" << std::endl;
	Valids & valids = data.get<agent_valid>();
	Valids::iterator it_begin, it_end;
	boost::tuples::tie(it_begin, it_end) = valids.equal_range(false);
	while(it_begin != it_end) {
		Valids::iterator it_erase = it_begin++;
		valids.erase(it_erase);
	}
}

void sim_mob::AgentsList::for_each_agent(boost::function<void(sim_mob::Agent*)> Fn)
{
	Lock lock(mutex);
	AgentsList::type & agents = data.get<0>();
	for(AgentsList::iterator it= agents.begin(), it_end(agents.end()); it != it_end; it++ ) {
		if(it->valid == false){
			Print() << "Warning, Working with an Invalid Agent" << std::endl;
		}
		Fn(it->agent);
	}
}

/*******************************************************************
 * ********* container Query operations ***************************************
 * ******************************************************************
 */

int sim_mob::AgentsList::size( )
{
	Lock lock(mutex);
	Agents &aa = data.get<agent_tag>();
	return aa.size();
}


bool sim_mob::AgentsList::empty()
{
	Lock lock(mutex);
	Agents &aa = data.get<agent_tag>();
	return aa.empty();
}

/*******************************************************************
 * ********* get operations ***************************************
 * ******************************************************************
 */
AgentsList::type& sim_mob::AgentsList::getAgents()
{
	Lock lock(mutex);
	return data.get<0>();
}

AgentsList::Mutex* sim_mob::AgentsList::getMutex()
{
	return &mutex;
}


/*******************************************************************
 * ********* VALID ************************************************
 * ******************************************************************
 */

int sim_mob::AgentsList::Validate(Agent * agent)
{
	Lock lock(mutex);
	Agents &agents = data.get<agent_tag>();
	Agents::iterator it = agents.find(agent);
	if((it = agents.find(agent)) != agents.end()) {
		if(it->valid) {
			return 1;
		} else {
			return 0;
		}
	}
	return -1;
}

int sim_mob::AgentsList::Validate(AgentCommUtilityBase * comm)
{
	Lock lock(mutex);
	AgentCommUtilities &comms = data.get<agent_util>();
	AgentCommUtilities::iterator it;
	if((it = comms.find(comm)) != comms.end()) {
		if(it->valid) {
			return 1;
		} else {
			return 0;
		}
	}
	return -1;
}


sim_mob::AgentsList::change_valid::change_valid(bool new_value):new_value(new_value)
{
}

void sim_mob::AgentsList::change_valid::operator()(AgentInfo& e)
{
	 e.valid=new_value;
}

bool sim_mob::AgentsList::setValid(AgentCommUtilityBase * comm , bool value)
{
	Lock lock(mutex);
	bool success = false;
	AgentCommUtilities &comms = data.get<agent_util>();
	AgentCommUtilities::iterator it;
	if((it = comms.find(comm)) != comms.end()) {
		comms.modify(it,change_valid(value));
	}
	return -1;
}

bool sim_mob::AgentsList::setValid(Agent * agent , bool value)
{
	Lock lock(mutex);
	bool success = false;
	Agents &agents = data.get<agent_tag>();
	Agents::iterator it;
	if((it = agents.find(agent)) != agents.end()) {
		agents.modify(it,change_valid(value));
	}
	return -1;
}

/*******************************************************************
 * ********* Done ************************************************
 * ******************************************************************
*/
bool sim_mob::AgentsList::isDone(Agent * agent)
{
	Lock lock(mutex);
	Agents &agents = data.get<agent_tag>();
	Agents::iterator it = agents.find(agent);
	if((it = agents.find(agent)) != agents.end()) {
		if(it->done) {
			return true;
		}
	}
	return false;
}

bool sim_mob::AgentsList::isDone(AgentCommUtilityBase * comm)
{
	Lock lock(mutex);
	AgentCommUtilities &comms = data.get<agent_util>();
	AgentCommUtilities::iterator it;
	if((it = comms.find(comm)) != comms.end()) {
		if(it->done) {
			return true;
		}
	}
	return false;
}

AgentsList::done_range sim_mob::AgentsList::getNotDone()
{
	Lock lock(mutex);
	Dones &dones = data.get<agent_done>();
	return dones.equal_range(false);;
}



sim_mob::AgentsList::change_done::change_done(bool new_value):new_value(new_value)
{
}

void sim_mob::AgentsList::change_done::operator()(AgentInfo& e)
{
	e.done=new_value;
}

bool sim_mob::AgentsList::setDone(AgentCommUtilityBase * comm , bool value)
{
	Lock lock(mutex);
	bool success = false;
	AgentCommUtilities &comms = data.get<agent_util>();
	AgentCommUtilities::iterator it;
	if((it = comms.find(comm)) != comms.end()) {
		comms.modify(it,change_done(value));
		return true;
	}
	return false;
}

bool sim_mob::AgentsList::setDone(const Agent* agent , bool value)
{
	Lock lock(mutex);
	bool success = false;
	Agents &agents = data.get<agent_tag>();
	Agents::iterator it = agents.find(const_cast<Agent*>(agent));
	if(it != agents.end()) {
		agents.modify(it,change_done(value));
		return true;
	}
	return false;
}


