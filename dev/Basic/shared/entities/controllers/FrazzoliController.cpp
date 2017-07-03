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
#include "entities/controllers/OnCallController.hpp"
#include <algorithm>

namespace sim_mob {



FrazzoliController::~FrazzoliController() {
	// TODO Auto-generated destructor stub
}


RD_Graph FrazzoliController::generateRD_Graph()
{
	RD_Graph rdGraph;

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
	return rdGraph;
}

RGD_Graph FrazzoliController::generateRGD_Graph(const RD_Graph& rdGraph)
{
	RGD_Graph rgdGraph;
	Group< Group<TripRequestMessage> > overallRequestGroups;
	bool optimalityRequired = true;
	for (const std::pair<const Person*, const Schedule>& p : driverSchedules)
	{
		const Person* driver = p.first;
		const Node* driverNode = driver->exportServiceDriver()->getCurrentNode();
		const Schedule& currentSchedule = driverSchedules.at(p.first);

#ifndef NDEBUG
		if (!currentSchedule.empty())
		{
			std::stringstream msg; msg<<"Trying to assign a schedule to a driver whose current schedule is not empty. This is not supported yet";
			throw std::runtime_error(msg.str());
		}
#endif


		// Add request groups of size one
		unsigned occupancy = 1;
		for ( const RD_Edge& rdEdge : rdGraph.getRD_Edges(driver) )
		{

			const TripRequestMessage& request = rdEdge.first;
			Group<TripRequestMessage> requestGroup; requestGroup.insert(request);
			Schedule newSchedule;
			double travelTime = computeSchedule(driverNode, currentSchedule, requestGroup, newSchedule, optimalityRequired);
			if (travelTime>=0)
			{
				requestGroupsPerOccupancy[occupancy-1].insert(requestGroup);
				rgdGraph.addEdge(request, requestGroup);
				rgdGraph.addEdge(requestGroup, driver, travelTime, newSchedule);
			}
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
			const TripRequestMessage& r1 = requestGroupsIterator1->front();
			const TripRequestMessage& r2 = requestGroupsIterator2->front();
			if (rdGraph.doesEdgeExists(r1,r2 ) )
			{
				bool optimalityRequired = true;

				Group<TripRequestMessage> requestGroup; requestGroup.insert(r1); requestGroup.insert(r2);
				Schedule newSchedule;
				double travelTime = computeSchedule(driverNode, currentSchedule, requestGroup, newSchedule, optimalityRequired);
				if (travelTime >= 0)
				{
					// It is feasible that the driver serves this requestGroup
					requestGroupsPerOccupancy[occupancy-1].insert(requestGroup);
					rgdGraph.addEdge(r1,requestGroup);rgdGraph.addEdge(r2,requestGroup);
					rgdGraph.addEdge(requestGroup,driver, travelTime, newSchedule);
				}
			}
		}

	}
	return rgdGraph;
}

void FrazzoliController::greedyAssignment(RD_Graph& rdGraph, RGD_Graph& rgdGraph)
{
	std::set<TripRequestMessage> assignedRequests;
	std::set<const Person*> assignedDrivers;

	for (unsigned occupancy = maxVehicleOccupancy; occupancy>0; occupancy--)
	{
		rgdGraph.sortGD_Edges();
		while (rgdGraph.hasGD_Edges() )
		{
			const GD_Edge gdEdge = rgdGraph.popGD_Edge();
			for (const TripRequestMessage request : gdEdge.requestGroup.getElements() )
			{
				if ( 	assignedRequests.find(request) != assignedRequests.end() &&
						assignedDrivers.find(gdEdge.driver) != assignedDrivers.end()
				){
					assignedRequests.insert(request);
					assignedDrivers.insert(gdEdge.driver);
					assignSchedule(gdEdge.driver,gdEdge.schedule);
				} // else, if the request or the driver have been already assigned, we have nothing to do
			}
		}

	}
}

void FrazzoliController::computeSchedules()
{
	RD_Graph rdGraph = generateRD_Graph() ;
	RGD_Graph rgdGraph = generateRGD_Graph(rdGraph);
	greedyAssignment(rdGraph, rgdGraph);
}

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

void RGD_Graph::addEdge(const Group<TripRequestMessage>& requestGroup, const Person* mobilityServiceDriver,
		double cost, const Schedule& schedule)
{	throw std::runtime_error("Implement it"); }

bool RGD_Graph::hasGD_Edges() const
{
	return !gdEdges.empty();
}

void RGD_Graph::sortGD_Edges()
{	throw std::runtime_error("Implement it"); }

GD_Edge RGD_Graph::popGD_Edge()
{	throw std::runtime_error("Implement it"); }

} /* namespace sim_mob */
