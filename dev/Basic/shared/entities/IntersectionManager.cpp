//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "IntersectionManager.hpp"
#include "geospatial/RoadSegment.hpp"
#include "message/MessageBus.hpp"

using namespace sim_mob;

//Initialise static member
map<unsigned int, IntersectionManager *> IntersectionManager::intManagers;

IntersectionManager::IntersectionManager(const MutexStrategy& mutexStrategy, const MultiNode *node) :
Agent(mutexStrategy), intMgrId(node->getID()), multinode(node)
{
}

IntersectionManager::~IntersectionManager()
{
}

void IntersectionManager::HandleMessage(Message::MessageType type, const Message& message)
{
	switch(type) 
	{
		case MSG_REQUEST_INT_ARR_TIME:
		{
			const IntersectionAccess& msg = MSG_CAST(IntersectionAccess, message);
			receivedRequests.push_back(&msg);
		}
		break;
	
		default:
			break;
	}
}

IntersectionManager* IntersectionManager::getIntManager(unsigned int id)
{
	map<unsigned int, IntersectionManager *>::iterator itIntMgr = intManagers.find(id);
	
	if(itIntMgr != intManagers.end())
	{
		return itIntMgr->second;
	}
	else
	{
		return nullptr;
	}
}

bool IntersectionManager::frame_init(timeslice now)
{
	string modelName = "general_driver_model";

	//Get the parameter manager instance
	ParameterManager *parameterMgr = ParameterManager::Instance(false);

	//Read the parameter values
	parameterMgr->param(modelName, "tailgate_separation_time", tailgateSeparationTime, 1.0);
	parameterMgr->param(modelName, "conflict_separation_time", conflictSeparationTime, 2.5);
	
	//Get the road segments connected to the multi-node
	set<RoadSegment *>::const_iterator itRoadSegments = multinode->getRoadSegments().begin();
	
	//Iterate through all the road segments and get the turnings for each of them
	while(itRoadSegments != multinode->getRoadSegments().end())
	{
		//Get the turnings in the road segment
		set<TurningSection *>::const_iterator itTurnings = multinode->getTurnings(*itRoadSegments).begin();
		
		while(itTurnings != multinode->getTurnings(*itRoadSegments).end())
		{
			//Add the turning to the map of previous access times with initial value -T1
			mapOfPrevAccessTimes.insert(make_pair((*itTurnings)->getDbId(), -tailgateSeparationTime));
			
			++itTurnings;
		}
		
		++itRoadSegments;
	}
	
	return true;
}

Entity::UpdateStatus IntersectionManager::frame_tick(timeslice now)
{
	//Delete all the previously sent responses
	for(list<const IntersectionAccess *>::iterator itSent = sentResponses.begin(); itSent != sentResponses.end(); ++itSent)
	{
		delete (*itSent);
		(*itSent) = nullptr;
	}
	
	sentResponses.clear();
	
	//Sort the request according to the earliest arrival times
	CompareArrivalTimes compare;
	receivedRequests.sort(compare);
	
	//Iterate through all the requests (sorted by increasing arrival time)
	for(list<const IntersectionAccess *>::iterator itReq = receivedRequests.begin(); itReq != receivedRequests.end(); ++itReq)
	{
		//Get the id of the turning on which the requesting vehicle will be driving
		unsigned int turningId = (*itReq)->getTurning()->getDbId();
		
		//Get the last access time for the turning, and compute the access time for the vehicle
		double accessTime = max((*itReq)->getArrivalTime(), mapOfPrevAccessTimes[turningId] + tailgateSeparationTime);
		
		//The responses to the requests which are incompatible with the current request
		list<const IntersectionAccess *> conflicts;
		
		//Get the vehicles, whose requests have been processed, that conflict with the current requesting vehicle		
		getConflicts(*itReq, conflicts);
		
		//Filter out the incompatible requests which have been allocated access times less than the 
		//access time for current request
		filterConflicts(accessTime, conflicts);
		
		if(conflicts.front()->getArrivalTime() < (accessTime + conflictSeparationTime))
		{
			//Iterators to point to consecutive elements in the list
			list<const IntersectionAccess *>::iterator itConflicts, itConflictsNext;
			
			//Point the iterators to the beginning of the list and advance one of them to the next element
			itConflicts = itConflictsNext = conflicts.begin();			
			++itConflictsNext;
			
			while(itConflictsNext != conflicts.end())
			{
				if((*itConflictsNext)->getArrivalTime() - (*itConflicts)->getArrivalTime() >= (2 * conflictSeparationTime))
				{
					accessTime = (*itConflicts)->getArrivalTime() + conflictSeparationTime;
					break;
				}
				
				++itConflicts;
				++itConflictsNext;
			}
		}
		
		//Update the previous access time for this turning
		mapOfPrevAccessTimes[turningId] = accessTime;
		
		//Set the computed access time
		IntersectionAccess *response = new IntersectionAccess((*itReq)->getPerson(), accessTime, (*itReq)->getTurning());
		
		//Add to the sent responses list
		sentResponses.push_back(response);
		
		//Send the response
		Person *recipient = const_cast<Person *>((*itReq)->getPerson());
		MessageBus::PostMessage(recipient, MSG_RESPONSE_INT_ARR_TIME, MessageBus::MessagePtr(response));
	}
	
	//Intersection managers are not removed
	return UpdateStatus::Continue;
}

void IntersectionManager::frame_output(timeslice now)
{
	//Clear the received requests
	receivedRequests.clear();
}

bool IntersectionManager::isNonspatial()
{
	return true;
}

void IntersectionManager::load(const std::map<std::string,std::string>& configProps)
{
}

void IntersectionManager::getConflicts(const IntersectionAccess *request, list<const IntersectionAccess *> &conflicts)
{
	//The turning section of the current requesting vehicle
	const TurningSection *currTurning = request->getTurning();

	//Iterate through the processed requests and find the requests that are incompatible with the current request
	for (list<const IntersectionAccess *>::const_iterator itResponse = sentResponses.begin(); itResponse != sentResponses.end(); ++itResponse)
	{
		//The turning section of the vehicle whose request has been processed
		const TurningSection *processedTurning = (*itResponse)->getTurning();

		//Check if there is a conflict between the turnings
		if (currTurning->getTurningConflict(processedTurning))
		{
			//Add the request into the vector of conflicts
			conflicts.push_back(*itResponse);
		}
	}
}

void IntersectionManager::filterConflicts(double accessTime, list<const IntersectionAccess *> &conflicts)
{
	//Iterate through the list of conflicts
	for(list<const IntersectionAccess *>::iterator itConflicts = conflicts.begin(); itConflicts != conflicts.end(); ++itConflicts)
	{
		//If the access time of the current request is greater than that of the conflicts to which responses have already been 
		//sent, filter the conflict out
		if((*itConflicts)->getArrivalTime() < accessTime)
		{
			itConflicts = conflicts.erase(itConflicts);
		}
	}
	
	conflicts.sort();
}