//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "IntersectionManager.hpp"
#include "geospatial/RoadSegment.hpp"

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
			const IntAccessRequest& msg = MSG_CAST(IntAccessRequest, message);
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
	//Sort the request according to the earliest arrival times
	CompareArrivalTimes compare;
	receivedRequests.sort(compare);
	
	vector<const IntAccessRequest *> processedRequests;
	
	//Iterate through all the requests (sorted by increasing arrival time)
	for(list<const IntAccessRequest *>::const_iterator itReq = receivedRequests.begin(); itReq != receivedRequests.end(); ++itReq)
	{
		//Get the id of the turning on which the requesting vehicle will be driving
		unsigned int turningId = (*itReq)->getTurning()->getDbId();
		
		//Get the last access time for the turning, and compute the access time for the vehicle
		double accessTime = max((*itReq)->getArrivalTime(), mapOfPrevAccessTimes[turningId] + tailgateSeparationTime);
		
		//The requests made by vehicles that have conflicts with the vehicles whose request have been processed
		vector<const IntAccessRequest *> incompatibleRequests;
		
		//Get the vehicles, whose requests have been processed, that conflict with the current requesting vehicle		
		getIncompatibleRequests(*itReq, processedRequests, incompatibleRequests);
		
		//Filter out the incompatible requests which have been allocated access times less than the 
		//access time for current request
		filterOutNonConflictingReq(accessTime, incompatibleRequests);
	}
	
	//Intersection managers are not removed
	return UpdateStatus::Continue;
}

void IntersectionManager::frame_output(timeslice now)
{	
}

bool IntersectionManager::isNonspatial()
{
	return true;
}

void IntersectionManager::load(const std::map<std::string,std::string>& configProps)
{
}

void IntersectionManager::getIncompatibleRequests(const IntAccessRequest *request, vector<const IntAccessRequest *> &processedReq,
												  vector<const IntAccessRequest *> &incompatibleReq)
{
	//The turning section of the current requesting vehicle
	const TurningSection *currTurning = request->getTurning();

	//Iterate through the processed requests and find the requests that are incompatible with the current request
	for (vector<const IntAccessRequest *>::const_iterator itProcessedReq = processedReq.begin(); itProcessedReq != processedReq.end(); ++itProcessedReq)
	{
		//The turning section of the vehicle whose request has been processed
		const TurningSection *processedTurning = (*itProcessedReq)->getTurning();

		//Check if there is a conflict between the turnings
		if (currTurning->getTurningConflict(processedTurning))
		{
			//Add the request into the vector of incompatible requests
			incompatibleReq.push_back(request);
		}
	}
}

void IntersectionManager::filterOutNonConflictingReq(double accessTime, vector<const IntAccessRequest *> &incompatibleReq)
{
	for(vector<const IntAccessRequest *>::const_iterator itReq = incompatibleReq.begin(); itReq != incompatibleReq.end(); ++itReq)
	{
		//if(accessTime < (*itReq)->getArrivalTime())
	}
}