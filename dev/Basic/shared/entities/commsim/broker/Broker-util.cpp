//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Broker-util.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;

sim_mob::AgentInfo::AgentInfo(Agent * a_, bool v)
	: agent(a_), valid(v), done(false)
{
}


/*******************************************************************
 * ********* Change Container ***************************************
 * ******************************************************************
 */
void sim_mob::AgentsList::insert(Agent * a, bool valid )
{
	Lock lock(mutex);
	data.insert(std::make_pair(a, AgentInfo(a,valid)));
}

void sim_mob::AgentsList::AgentsList::erase(Agent * agent)
{
	Lock lock(mutex);

	type::iterator it = data.find(agent);
	if (it!=data.end()) {
		//Print() << "AgentsList::erase test:  agent[" << agent << "] found for deletion" << std::endl;
		data.erase(it);
	} else {
		//Print() << "AgentsList::erase test:  agent[" << agent << "] NOT found for deletion" << std::endl;
	}
}


void sim_mob::AgentsList::eraseInvalids()
{
	Lock lock(mutex);
	Print() << "AgentsList::eraseInvalids test" << std::endl;

	for (type::iterator it=data.begin(); it!=data.end(); it++) {
		if (!it->second.valid) {
			data.erase(it); //FYI: map::erase doesn't invalidate iterators.
		}
	}
}

void sim_mob::AgentsList::for_each_agent(boost::function<void(sim_mob::Agent*)> Fn)
{
	Lock lock(mutex);

	for (type::iterator it=data.begin(); it!=data.end(); it++) {
		if (!it->second.valid) {
			Print() << "Warning, Working with an Invalid Agent" << std::endl;
		}
		Fn(it->second.agent);
	}
}

/*******************************************************************
 * ********* container Query operations ***************************************
 * ******************************************************************
 */

int sim_mob::AgentsList::size() const
{
	Lock lock(mutex);
	return data.size();
}


bool sim_mob::AgentsList::empty() const
{
	Lock lock(mutex);
	return data.empty();
}

/*******************************************************************
 * ********* get operations ***************************************
 * ******************************************************************
 */
AgentsList::type& sim_mob::AgentsList::getAgents()
{
	Lock lock(mutex);
	return data;
}

const AgentsList::type& sim_mob::AgentsList::getAgents() const
{
	Lock lock(mutex);
	return data;
}

AgentsList::Mutex* sim_mob::AgentsList::getMutex()
{
	return &mutex;
}



void sim_mob::AgentsList::setValid(Agent * agent , bool value)
{
	Lock lock(mutex);
	//bool success = false;

	type::iterator it = data.find(agent);
	if (it!=data.end()) {
		it->second.valid = value;
	}
}

/*******************************************************************
 * ********* Done ************************************************
 * ******************************************************************
*/
bool sim_mob::AgentsList::isDone(Agent * agent) const
{
	Lock lock(mutex);

	type::const_iterator it = data.find(agent);
	if (it!=data.end()) {
		return it->second.done;
	}

	return false;
}

bool sim_mob::AgentsList::hasNotDone() const {
	Lock lock(mutex);
	for (type::const_iterator it=data.begin(); it!=data.end(); it++) {
		if (!it->second.done) {
			return true;
		}
	}
	return false;
}


bool sim_mob::AgentsList::setDone(const Agent* agent)
{
	Lock lock(mutex);

	type::iterator it = data.find(agent);
	if (it!=data.end()) {
		it->second.done = true;
		return true;
	}

	return false;
}


