/*
 * FrazzoliController.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#include "FrazzoliController.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob {



FrazzoliController::~FrazzoliController() {
	// TODO Auto-generated destructor stub
}


void FrazzoliController::generateRV_Graph()
{
	// https://stackoverflow.com/a/1824900/2110769
	// We check the shareability of every possible pair of requests
	for (std::list<TripRequestMessage>::const_iterator r1 = requestQueue.begin(); r1!=requestQueue.end(); r1++)
	for (std::list<TripRequestMessage>::const_iterator r2 = r1; ++r2 != requestQueue.end(); )
	{
		if (canBeShared(*r1,*r2, additionalDelayThreshold, waitingTimeThreshold) )
			rdGraph.addEdge(*r1,*r2);
	}



	for (const TripRequestMessage& request : requestQueue)
	{
		for (const std::pair<const Person*, const Schedule>& p : driverSchedules )
		{
			const Person* driver = p.first;
			const Schedule& currentSchedule = p.second;
			const Node *driverNode = getCurrentNode(driver);
			Group<TripRequestMessage> additionalRequests; additionalRequests.insert(request);
			Schedule newSchedule;
			bool optimalityRequired = false;
			double travelTime = computeSchedule(driverNode, currentSchedule, additionalRequests, newSchedule, optimalityRequired);
			if (travelTime>=0)
			{
				rdGraph.addEdge(request,driver );
			}
		}

	}
}

void FrazzoliController::generateRGD_Graph()
{
	Group< Group<TripRequestMessage> > overallRequestGroups;
	for (const std::pair<const Person*, const Schedule>& p : driverSchedules)
	{
		const Person* driver = p.first;

		// requestGroupsPerOccupancy[i] will contain all the request groups of i+1 requests
		std::vector< Group< Group<TripRequestMessage> > > requestGroupsPerOccupancy(maxVehicleOccupancy);

		// Add request groups of size one
		unsigned occupancy = 1;
		for ( const RD_Edge& rdEdge : rdGraph.getRD_Edges(driver) )
		{
			const TripRequestMessage& request = rdEdge.first;
			Group<TripRequestMessage> requestGroup; requestGroup.insert(request);
			requestGroupsPerOccupancy[occupancy-1].insert(requestGroup);
			rgdGraph.addEdge(request, requestGroup);
			rgdGraph.addEdge(requestGroup, driver);
		}

		// Add request groups of size 2

		occupancy++;
		auto requestGroupsIterator1 = requestGroupsPerOccupancy[occupancy-1].getElements().begin();
		auto end_ = requestGroupsPerOccupancy[occupancy-1].getElements().end();
		for ( ; requestGroupsIterator1 != end_; requestGroupsIterator1++)
		for (	auto requestGroupsIterator2 = requestGroupsIterator1;
				++requestGroupsIterator2 != end_;
		){
#ifndef NDEBUG
			if (requestGroupsIterator1->size()!=1 || requestGroupsIterator2->size()!=1)
			{
				std::stringstream msg; msg<<"rg1 and rg2 must have both size 1. Instead they are "<<
					 "requestGroupsIterator1:"<<*requestGroupsIterator1<<", requestGroupsIterator2:"<<*requestGroupsIterator2;
				throw std::runtime_error(msg.str() );
			}
#endif
			const TripRequestMessage& r1 = requestGroupsIterator1->pop_front();
			const TripRequestMessage& r2 = requestGroupsIterator2->pop_front();
			if (rdGraph.doesEdgeExists(r1,r2 ) )
			{
				bool optimalityRequired = false;
				const Node* driverNode = driver->exportServiceDriver()->getCurrentNode();
				const Schedule& currentSchedule = driverSchedules.at(p.first);
				Group<TripRequestMessage> requestGroup; requestGroup.insert(r1); requestGroup.insert(r2);
				Schedule newSchedule;
				double travelTime = computeSchedule(driverNode, currentSchedule, requestGroup, newSchedule, optimalityRequired);
				if (travelTime >= 0)
				{
					// It is feasible that the driver serves this requestGroup
					requestGroupsPerOccupancy[occupancy-1].insert(requestGroup);
					rgdGraph.addEdge(r1,requestGroup);rgdGraph.addEdge(r2,requestGroup);
					rgdGraph.addEdge(requestGroup,driver);
				}
			}
		}

	}
}


void FrazzoliController::computeSchedules()
{	throw std::runtime_error("Implement it"); }

const std::vector< RD_Edge>& RD_Graph::getRD_Edges(const Person* driver) const
{	throw std::runtime_error("Implement it"); }
void RD_Graph::addEdge(const TripRequestMessage& r1, const TripRequestMessage& r2)
{	throw std::runtime_error("Implement it"); }
void RD_Graph::addEdge(const TripRequestMessage& request, const Person* mobilityServiceDriver)
{	throw std::runtime_error("Implement it"); }
bool RD_Graph::doesEdgeExists(const TripRequestMessage& r1, const TripRequestMessage& r2) const
{	throw std::runtime_error("Implement it"); }

void RGD_Graph::addEdge(const TripRequestMessage& request, const Group<TripRequestMessage>& requestGroup)
{	throw std::runtime_error("Implement it"); }
void RGD_Graph::addEdge(const Group<TripRequestMessage>& requestGroup, const Person* mobilityServiceDriver)
{	throw std::runtime_error("Implement it"); }

} /* namespace sim_mob */
