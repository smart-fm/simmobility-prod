//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#include "models/IntersectionDrivingModel.hpp"
#include "DriverUpdateParams.hpp"
#include "Driver.hpp"
#include "entities/IntersectionManager.hpp"
#include "message/MessageBus.hpp"

using namespace std;
using namespace sim_mob;
using namespace messaging;

SlotBased_IntDriving_Model::SlotBased_IntDriving_Model() : 
isRequestSent(false), isAccelerationComputed(false), uniformAcceleration(0)
{
	modelType = Int_Model_SlotBased;
}

SlotBased_IntDriving_Model::~SlotBased_IntDriving_Model()
{
}

double SlotBased_IntDriving_Model::makeAcceleratingDecision(DriverUpdateParams& params)
{
	double acc = params.maxAcceleration;
	
	//The intersection manager responds with a time. We must adjust our speed such that we arrive at the intersection at the given
	//time.	
	if(params.isResponseReceived)
	{
		//Time remaining to reach the intersection
		double timeToReachInt = params.accessTime - (params.now.ms() / 1000);
		
		if(timeToReachInt > 0)
		{
			//Calculate the acceleration required to reach the intersection at the given time
			acc = 2 * (params.driver->distToIntersection_.get() - (params.currSpeed * timeToReachInt)) / (timeToReachInt * timeToReachInt);
			
			//Set the uniform acceleration
			uniformAcceleration = acc;
			isAccelerationComputed = true;
		}
		
		params.isResponseReceived = false;
	}
	else if(isAccelerationComputed)
	{
		acc = uniformAcceleration;
	}
	
	return acc;
}

void SlotBased_IntDriving_Model::sendAccessRequest(DriverUpdateParams& params)
{
	//We send a request to the intersection manager of the approaching intersection, asking for 'permission' to enter the 
	//intersection
	
	//Send a request only if we have not sent a request previously
	if(currTurning && isRequestSent == false)
	{
		//Get the approaching intersection manager
		IntersectionManager *intMgr = IntersectionManager::getIntManager(currTurning->getFromSeg()->getEnd()->getID());
		
		//Calculate the arrival time according to the current speed and the distance to the intersection
		double arrivalTime = calcArrivalTime(params);		
		
		if(arrivalTime > 0)
		{
			//Add the current time, to calculate the actual time the vehicle should reach the intersection
			arrivalTime += (params.now.ms() / 1000);
			
			//Pointer to the access request sent by the driver
			IntersectionAccess *accessRequest = new IntersectionAccess(arrivalTime, currTurning->getDbId());
			accessRequest->SetSender(params.driver->getParent());

			//Send the request message
			MessageBus::PostMessage(intMgr, MSG_REQUEST_INT_ARR_TIME, MessageBus::MessagePtr(accessRequest));

			isRequestSent = true;			
		}
	}
}

bool SlotBased_IntDriving_Model::isUniformAcceleration()
{
	return isAccelerationComputed;
}