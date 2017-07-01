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
#include <sstream>

namespace sim_mob {




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
			if (rdGraph.doesEdgeExist(r1,r2 ) )
			{
				bool optimalityRequired = true;

				Group<TripRequestMessage> requestGroup; requestGroup.insert(r1); requestGroup.insert(r2);
				Schedule newSchedule;
				double travelTime = computeSchedule(driverNode, currentSchedule, requestGroup, newSchedule, optimalityRequired);
				if (travelTime >= 0)
				{
					// It is feasible that the driver serves this requestGroup
					requestGroupsPerOccupancy[occupancy-1].insert(requestGroup);
					rgdGraph.addEdge(r1,requestGroup);
					rgdGraph.addEdge(r2,requestGroup);
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
			for (const TripRequestMessage request : gdEdge.getRequestGroup().getElements() )
			{
				if ( 	assignedRequests.find(request) != assignedRequests.end() &&
						assignedDrivers.find(gdEdge.getDriver()) != assignedDrivers.end()
				){
					assignedRequests.insert(request);
					assignedDrivers.insert(gdEdge.getDriver() );
					assignSchedule(gdEdge.getDriver(),gdEdge.getSchedule());
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


bool GD_Edge::operator<(const GD_Edge& other) const
{
	if (cost < other.cost) return true;
	if (cost > other.cost) return false;

	if (driver->getDatabaseId() < other.driver->getDatabaseId() )  return true;
	if (driver->getDatabaseId() > other.driver->getDatabaseId() )  return false;

	if (requestGroup < other.requestGroup) return true;

	return false;
}

Group<TripRequestMessage> GD_Edge::getRequestGroup() const{return requestGroup;};
const Person* GD_Edge::getDriver() const{return driver;};
double GD_Edge::getCost() const{return cost;};
Schedule GD_Edge::getSchedule() const{return schedule;};


/*********************
 *
 *
 *
 * RD_Graph implementation
 *
 *
 *
 */

void RD_Graph::addEdge(const TripRequestMessage& r1, const TripRequestMessage& r2)
{
	rrEdges[r1].insert(r2);
}

bool RD_Graph::doesEdgeExist(const TripRequestMessage& r1, const TripRequestMessage& r2) const
{
	std::map< TripRequestMessage, std::set<TripRequestMessage > >::const_iterator it =
			rrEdges.find(r1);
	if (it!=rrEdges.end() )
	{
		const std::set<TripRequestMessage>& requestsAssociatedToR1 = it->second;
		if ( requestsAssociatedToR1.find(r2)!= requestsAssociatedToR1.end() )
			return true;
	}
	return false;
}

void RD_Graph::addEdge(TripRequestMessage request, const Person* driver)
{
	RD_Edge rdEdge = std::make_pair(request,driver);
	std::map<const Person*, std::vector<RD_Edge>>::iterator p = rdEgdeMap.find(driver);
	if (p != rdEgdeMap.end() )
		(p->second).push_back(rdEdge);
	else{
		//std::vector<const TripRequestMessage> v; v.push_back(request);
		p->second.push_back(rdEdge);
	}
}

const std::vector< RD_Edge>& RD_Graph::getRD_Edges(const Person* driver) const
{	return rdEgdeMap.at(driver);}








/*********************
 *
 *
 *
 * RGD_Graph implementation
 *
 *
 *
 */
void RGD_Graph::addEdge(const TripRequestMessage& request, const Group<TripRequestMessage>& requestGroup)
{
	// Does nothing

#ifndef NDEBUG
	const RG_Edge rgEdge = std::make_pair(request, requestGroup);
	rgEdges.insert(rgEdge);
#endif
}

void RGD_Graph::addEdge(const Group<TripRequestMessage>& requestGroup, const Person* mobilityServiceDriver,
		double cost, const Schedule& schedule)
{
	gdEdges.push_back( GD_Edge(requestGroup, mobilityServiceDriver,cost, schedule)  );
}

bool RGD_Graph::hasGD_Edges() const
{
	return !gdEdges.empty();
}

void RGD_Graph::sortGD_Edges()
{
	std::sort(gdEdges.rbegin(),gdEdges.rend() );
#ifndef NDEBUG
	double minCostSoFar =  std::numeric_limits<double>::max();
	for (const GD_Edge e : gdEdges)
	{
		const double cost =e.getCost();
		if (cost >minCostSoFar)
			throw std::runtime_error("Sorting is incorrect");
		minCostSoFar = cost;
	}

	isGdEdgesSorted=true;
#endif
}

GD_Edge RGD_Graph::popGD_Edge()
{
#ifndef NDEBUG
	if (!isGdEdgesSorted)
		throw std::runtime_error("You must sort before popping edges");
#endif
	GD_Edge retValue = gdEdges.back();
	gdEdges.pop_back();
	return retValue;
}

void RGD_Graph::consistencyChecks() const
{
	for (const RG_Edge& rgEdge : rgEdges.getElements())
	{
		const Group<TripRequestMessage>& requestGroup = rgEdge.second;
		const TripRequestMessage& r = rgEdge.first;
		if ( !requestGroup.contains(r) )
		{
			std::stringstream msg; msg<<"Malformed RG_Edge. It connects request "<<r<<" to a request group "<<
				requestGroup << ", which, as you can see, does not contain that request";
			throw std::runtime_error(msg.str() );
		}

	}
}

} /* namespace sim_mob */


std::ostream& operator<<(std::ostream& strm, const sim_mob::RG_Edge& rgEdge)
{
	strm<<"RG_Edge[request:"<<rgEdge.first<<", group:"<<rgEdge.second<<"]";
	return strm;
}
