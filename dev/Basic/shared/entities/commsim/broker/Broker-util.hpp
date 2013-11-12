//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
//#include "entities/Agent.hpp"

//structure of each data element in the storage

namespace sim_mob
{
//Forward Declaration
class AgentCommUtilityBase;
class Agent;

class AgentInfo {
public:
	Agent * agent;
	AgentCommUtilityBase * comm;
	bool valid;
	bool done;
	AgentInfo(Agent * a_, AgentCommUtilityBase * b_, bool valid = true);
	AgentInfo();
};


//helper tags used in multi_index_container
struct agent_tag{};
struct agent_util{};
struct agent_valid{};
struct agent_done{};

class AgentsList {
public:
private:
	//main definition of the container
	typedef boost::multi_index_container<
			AgentInfo, boost::multi_index::indexed_by<
			boost::multi_index::random_access<>//0
		,boost::multi_index::hashed_unique<boost::multi_index::tag<agent_tag>,boost::multi_index::member<AgentInfo, Agent *, &AgentInfo::agent> >//1
		,boost::multi_index::hashed_unique<boost::multi_index::tag<agent_util>,boost::multi_index::member<AgentInfo, AgentCommUtilityBase *, &AgentInfo::comm> >//2
		,boost::multi_index::hashed_non_unique<boost::multi_index::tag<agent_valid>,boost::multi_index::member<AgentInfo, bool, &AgentInfo::valid> >//3
		,boost::multi_index::hashed_non_unique<boost::multi_index::tag<agent_done>,boost::multi_index::member<AgentInfo, bool, &AgentInfo::done> >//4
		>
	> ContainerType;

	//the main storage
	ContainerType data;

	//easy reading only
	typedef typename boost::multi_index::nth_index<ContainerType, 1>::type Agents;
	typedef typename boost::multi_index::nth_index<ContainerType, 2>::type AgentCommUtilities;
	typedef typename boost::multi_index::nth_index<ContainerType, 3>::type Valids;
	typedef typename boost::multi_index::nth_index<ContainerType, 4>::type Dones;

public:
	//easy reading only
	typedef typename boost::multi_index::nth_index<ContainerType, 0>::type type;//this one is for public use
	typedef typename type::iterator iterator;
	typedef std::pair<Valids::iterator, Valids::iterator> valid_range;
	typedef std::pair<Dones::iterator, Dones::iterator> done_range;
	//the mutex used for this class
	typedef typename  boost::recursive_mutex Mutex;
	typedef typename boost::unique_lock<Mutex> Lock;
	mutable Mutex mutex;


	//	function to insert into the container (only checkes the unqueness of agent)
	void insert(Agent * a, AgentCommUtilityBase * b, bool valid = true);

	//	erases an elment from the container given the agent's reference
	void erase(Agent * agent);

	//returns the size of the container
	int size( ) ;

	//returns if the container is empty
	bool empty();

	//	erases an elment from the container given its communication equipment
	void erase(AgentCommUtilityBase * comm);

	//erase all elements whose value of 'valid' is set to false
	void eraseInvalids();

	//returns the container . not safe if used/iterated without mutex
	AgentsList::type &getAgents();

	//returns the mutex used to operate on the container of this class. use it along with getAgents(()
	//as it is dangerous to work without locking in a multithreaded environment
	AgentsList::Mutex *getMutex();

	//iterates safely through the valid agents executing the requested Function
	void for_each_agent(boost::function<void(sim_mob::Agent*)> Fn);

	//	given an agent's reference, returns a const version of a single element from the container.
	const AgentInfo  &getAgentInfo(Agent * agent, bool &success);
//	const AgentInfo  &getAgentInfo(const Agent * agent, bool &success)const;

	//	given a reference to an agent's comm equipment, returns a const version of a single element from the container.
	const AgentInfo  &getAgentInfo(AgentCommUtilityBase * comm, bool &success);

	/*******************************************************************
	 * ********* VALID ************************************************
	 * ******************************************************************
	 */

	//validates an element give an agent
	//if found and if valid, return 1
	//if found and not valid, return 0
	//if not found return -1
	int Validate(Agent * agent);

	//validates an element give an agent's comm equipment
	//if found and if valid, return 1
	//if found and not valid, return 0
	//if not found return -1
	int Validate(AgentCommUtilityBase * comm);

	//internal use only(for setting an element to valid/invalid) yep, it is hectic
	struct change_valid
	{
		change_valid(bool new_value);

	  void operator()(AgentInfo& e);

	private:
	  bool new_value;
	};

	//	sets valid value of an element in the container, give an agent's comm equipment
	bool setValid(AgentCommUtilityBase * comm , bool value);

	//	sets valid value of an element in the container, give an agent
	bool setValid(Agent * agent , bool value);
	/*******************************************************************
	 * ********* Done ************************************************
	 * ******************************************************************
	 */
//checks if an agent is done
	bool isDone(Agent * agent);

		//checks if an agent is done
	bool isDone(AgentCommUtilityBase * comm);

	done_range getNotDone();

	//internal use only(for setting an element to valid/invalid) yep, it is hectic
	struct change_done
	{
		change_done(bool new_value);

	  void operator()(AgentInfo& e);

	private:
	  bool new_value;
	};

	//	sets Done value of an element in the container, give an agent's comm equipment
	bool setDone(AgentCommUtilityBase * comm , bool value) ;

	//	sets Done value of an element in the container, give an agent
	bool setDone(const Agent* agent , bool value);
};

}//namespace sim_mob

//
//
//int main()
//{
//	AgentsList<char> aa;
//	Agent *a = new Agent(1);
//	Agent *a1;
//	AgentCommUtility<char> *b = new AgentCommUtility<char>('C');
//	aa.insert(a , b, true);
////	aa.erase(b);
//	AgentsList<char>::type &aaa = aa.getAgents();
////	if(aaa.begin() != aaa.end())
////		std::cout << (*(aaa.begin()->b)).i << std::endl;
////	bool success;
////	const AgentsList<char>::AgentInfo ag = aa.getAgentInfo(a1, success);
////	if(success)
////	std::cout << ag.a->i << std::endl;
////
////	const AgentsList<char>::AgentInfo ag1 = aa.getAgentInfo(b, success);
////	if(success)
////	std::cout << ag1.a->i << std::endl;
////
////	std::cout << aa.Validate(a1) << std::endl;
//	aa.setValid(b,false);
//	std::cout << aa.Validate(b) << std::endl;
//	aa.eraseInvalids();
//	std::cout << aa.Validate(b) << std::endl;
//
//}
