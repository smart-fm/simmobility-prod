//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>
//#include <boost/thread/detail/thread_group.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>
#include "util/LangHelpers.hpp"

namespace sim_mob
{
	const unsigned int MAX_THREAD_GROUP_SIZE = 20; //minimum number of registered agents
//Forward Declaration
class AgentCommUtilityBase;
class Agent;

class AgentInfo {
public:
	Agent * agent;
	AgentCommUtilityBase * comm;
	bool valid;
	bool done;
	explicit AgentInfo(Agent* a_=nullptr, AgentCommUtilityBase* b_=nullptr, bool valid=true);
};


class AgentsList {
public:
	//easy reading only
	typedef typename std::map<const sim_mob::Agent*, AgentInfo> type;//this one is for public use

private:
	//the main storage
	type data;

public:
	typedef typename  boost::recursive_mutex Mutex;
	typedef typename boost::unique_lock<Mutex> Lock;
	mutable Mutex mutex;

	///function to insert into the container (only checkes the unqueness of agent)
	void insert(Agent * a, AgentCommUtilityBase * b, bool valid = true);

	///returns the size of the container
	int size() const;

	///returns if the container is empty
	bool empty() const;

	///erases an elment from the container given the agent's reference
	void erase(Agent * agent);

	///erase all elements whose value of 'valid' is set to false
	void eraseInvalids();

	///returns the container . not safe if used/iterated without mutex
	AgentsList::type &getAgents();

	///returns the mutex used to operate on the container of this class. use it along with getAgents(()
	///as it is dangerous to work without locking in a multithreaded environment
	AgentsList::Mutex *getMutex();

	//iterates safely through the valid agents executing the requested Function
	void for_each_agent(boost::function<void(sim_mob::Agent*)> Fn);

	///sets valid value of an element in the container, give an agent
	void setValid(Agent * agent , bool value);

	///checks if an agent is done
	bool isDone(Agent * agent) const;

	///Check if any agent exists which is not done.
	bool hasNotDone() const;

	//	sets Done value of an element in the container, give an agent
	bool setDone(const Agent* agent , bool value);
};
//boost::thread_group threads;
//int NumberOfThreads_sum = 0;
//int NumberOfThreads_cnt = 0;
///////////////////////////////////////////////////////////////////////////
//template<class C,class Fn>
//void doByThread(C &container, Fn function_){
//	boost::thread_group threads;
//	int numberOfThreads = ( container.size() > /*MAX_THREAD_GROUP_SIZE ? MAX_THREAD_GROUP_SIZE*/ 20 ? 20 : container.size() );
//	int nofMsgsPerThread = container.size() / numberOfThreads;
//	for(int i = 0; i < numberOfThreads; i++)
//	{
//		int firstMessageIndex = nofMsgsPerThread * i;
//		int lastMessageIndex = nofMsgsPerThread * (i + 1);
//		//one small check for remaining messages that not going to be assigned if we use the above simple assignment formula
//		if(nofMsgsPerThread * (i + 2) > container.size()){
//			lastMessageIndex = container.size();
//		}
//
//		threads.create_thread(boost::bind(boost::ref(function_), firstMessageIndex,lastMessageIndex));
//	}
//	threads.join_all();
//}
}//namespace sim_mob
