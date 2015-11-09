//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <list>
#include <map>
#include <set>

#include "Agent.hpp"
#include "conf/params/ParameterManager.hpp"
#include "geospatial/network/Node.hpp"
#include "Person.hpp"

using namespace std;
using namespace sim_mob::messaging;

namespace sim_mob
{

const Message::MessageType MSG_REQUEST_INT_ARR_TIME = 7000000;
const Message::MessageType MSG_RESPONSE_INT_ARR_TIME = 7000001;

class IntersectionAccessMessage;

class IntersectionManager : public Agent
{
private:
	/**Map holding the pointers to all the intersection manager objects*/
	static map<unsigned int, IntersectionManager *> intManagers;
	
	/**The id of the intersection manager. The id is the same as the node id*/
	unsigned int intMgrId;

	/**
	 * This map stores the most recent access time granted to a vehicle based on its turning id
	 * Key: Turning id, Value: Previous access time
	 */
	map<unsigned int, double> mapOfPrevAccessTimes;

	/**Stores the requests to be processed during the upcoming frame tick*/
	list<IntersectionAccessMessage> receivedRequests;

	/**Stores the responses sent in the current frame tick*/
	list<IntersectionAccessMessage> sentResponses;

	/**Separation time between vehicles following one another (also known as T1)*/
	double tailgateSeparationTime;

	/**Separation time between vehicles with conflicting trajectories (also known as T2)*/
	double conflictSeparationTime;

	/**Iterates through the processed requests to find the vehicles that are incompatible with the current request*/
	void getConflicts(IntersectionAccessMessage &request, list<IntersectionAccessMessage> &conflicts);

	/**Filters out the conflicts which have been allocated access times less than the access time for current request*/
	void filterConflicts(double accessTime, list<IntersectionAccessMessage> &conflicts);

protected:
	/**
	 * Indicates whether the agent is is non spatial in nature
     * @return true
     */
	virtual bool isNonspatial();

	/**
	 * Does nothing - compulsory override from agent class
	 *
     * @param configProps
     */
	virtual void load(const std::map<std::string, std::string> &configProps);

	/**
	 * Called during the first call to update() for a given agent.
	 *
     * @param now
	 *
     * @return true, if successful
     */
	virtual bool frame_init(timeslice now);

	/**
	 * Called during every call to update() for a given agent.
	 *
     * @param now
     *
	 * @return UpdateStatus::Continue
     */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Called after frame_tick() for every call to update() for a given agent
	 * 
     * @param now
     */
	virtual void frame_output(timeslice now);

public:
	IntersectionManager(const MutexStrategy &mutexStrategy, unsigned int id);
	virtual ~IntersectionManager();

	static IntersectionManager* getIntManager(unsigned int id);

	/**
	 * Creates an intersection manager for every node that doesn't have a traffic signal
     */
	static void CreateIntersectionManagers(const MutexStrategy &mutexStrategy);

	/**
	 * Handles the intersection access requests
	 *
     * @param type message type
     * @param message message
     */
	virtual void HandleMessage(Message::MessageType type, const Message &message);
};

class IntersectionAccessMessage : public Message
{
private:
	/**The arrival time of the person at the intersection*/
	double arrivalTime;

	/**The turning that will be used by the person*/
	int turningId;

public:
	IntersectionAccessMessage(const double arrivalTime, int turningId) :
	arrivalTime(arrivalTime), turningId(turningId)
	{
	}

	double getArrivalTime() const
	{
		return arrivalTime;
	}

	int getTurningId() const
	{
		return turningId;
	}
};

struct CompareArrivalTimes
{
	bool operator()(IntersectionAccessMessage first, IntersectionAccessMessage second)
	{
		return ( first.getArrivalTime() < second.getArrivalTime());
	}
};
}
