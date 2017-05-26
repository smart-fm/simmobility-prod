/*
 * SharedTaxiController.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "SharedTaxiController.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob
{
std::vector<MobilityServiceController::MessageResult> SharedTaxiController::computeSchedules()
{
	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

	std::vector<TripRequest> validRequests;
	std::vector<double> desiredTravelTimes;
	std::vector<unsigned int> badRequests;

	// 1. Calculate times for direct trips
	unsigned int requestIndex = 0;
	auto request = requestQueue.begin();
	while (request != requestQueue.end())
	{
		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end())
		{
			ControllerLog() << "Request contains bad start node " << (*request).startNodeId << std::endl;

			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end())
		{
			ControllerLog() << "Request contains bad destination node " << (*request).startNodeId << std::endl;

			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}
		Node* destinationNode = it->second;

		validRequests.push_back(*request);

		double tripTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			startNode->getNodeId(), destinationNode->getNodeId(), DailyTime(currTick.ms()));
		desiredTravelTimes.push_back(tripTime);

		request++;
		requestIndex++;
	}

	// 2. Add valid shared trips to graph
	boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph(validRequests.size());
	std::vector<boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::vertex_descriptor> mate(validRequests.size());

	std::map<std::pair<unsigned int, unsigned int>, std::pair<double, std::string>> bestTrips;

	auto request1 = validRequests.begin();
	unsigned int request1Index = 0;
	while (request1 != validRequests.end())
	{
		auto request2 = request1 + 1;
		unsigned int request2Index = request1Index + 1;
		while (request2 != validRequests.end())
		{
			ControllerLog() << "(" << request1Index << ", " << request2Index << ")" << std::endl;
			std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request1).startNodeId); 
			Node* startNode1 = it->second;

			it = nodeIdMap.find((*request1).destinationNodeId); 
			Node* destinationNode1 = it->second;

			it = nodeIdMap.find((*request2).startNodeId); 
			Node* startNode2 = it->second;

			it = nodeIdMap.find((*request2).destinationNodeId); 
			Node* destinationNode2 = it->second;

			// o1 o2 d1 d2
			double tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()));

			double tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTick.ms()));

			if ((tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
				&& (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold))
			{
				bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o1o2d1d2");
				add_edge(request1Index, request2Index, graph);
			}

			// o2 o1 d2 d1
			tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()));

			tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTick.ms()));

			if ((tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
				&& (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold))
			{
				if (bestTrips.count(std::make_pair(request1Index, request2Index)) > 0)
				{
					std::pair<double, std::string> currBestTrip = bestTrips[std::make_pair(request1Index, request2Index)];

					if (tripTime1 + tripTime2 < currBestTrip.first)
					{
						bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d2d2");
					}
				}
				else
				{
					bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d2d2");
				}

				add_edge(request1Index, request2Index, graph);
			}

			// o1 o2 d2 d1
			tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()));

			if (tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
			{
				if (bestTrips.count(std::make_pair(request1Index, request2Index)) > 0)
				{
					std::pair<double, std::string> currBestTrip = bestTrips[std::make_pair(request1Index, request2Index)];

					if (tripTime1 + tripTime2 < currBestTrip.first)
					{
						bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o1o2d2d1");
					}
				}
				else
				{
					bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o1o2d2d1");
				}

				add_edge(request1Index, request2Index, graph);
			}

			// o2 o1 d1 d2
			tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTick.ms()));

			if (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold)
			{
				if (bestTrips.count(std::make_pair(request1Index, request2Index)) > 0)
				{
					std::pair<double, std::string> currBestTrip = bestTrips[std::make_pair(request1Index, request2Index)];

					if (tripTime1 + tripTime2 < currBestTrip.first)
					{
						bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d1d2");
					}
				}
				else
				{
					bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d1d2");
				}

				add_edge(request1Index, request2Index, graph);
			}

			request2++;
			request2Index++;
		}
		request1++;
		request1Index++;
	}

	ControllerLog() << "About to perform matching, wish me luck" << std::endl;

	// 3. Perform maximum matching
	std::vector<TripRequest> privateCarRequests;
	bool success = boost::checked_edmonds_maximum_cardinality_matching(graph, &mate[0]);

	if (success)
	{
		ControllerLog() << "Found matching of size " << matching_size(graph, &mate[0])
				<< " for request list size of " << validRequests.size() << std::endl;

		boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::vertex_iterator vi, vi_end;

		std::set<unsigned int> sharedTrips;

		for (tie(vi,vi_end) = vertices(graph); vi != vi_end; ++vi)
		{
			if (mate[*vi] != boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::null_vertex() && *vi < mate[*vi])
			{
				// TODO: Send messages for shared trips
				sharedTrips.insert(*vi);
				sharedTrips.insert(mate[*vi]);
			}
		}

		unsigned int requestIndex = 0;

		for (std::vector<TripRequest>::iterator it = validRequests.begin(); it != validRequests.end(); it++)
		{
			// if (sharedTrips.count(requestIndex) == 0)
			// {
			// 	privateCarRequests.push_back(*it);
			// }

			privateCarRequests.push_back(*it);
			requestIndex++;
		}

	}
	else
	{
		ControllerLog() << "Did not find matching" << std::endl;

		for (std::vector<TripRequest>::iterator it = validRequests.begin(); it != validRequests.end(); it++)
		{
			privateCarRequests.push_back(*it);
		}
	}

	// 4. Send assignments for requests
	std::vector<MobilityServiceController::MessageResult> results;

	for (std::vector<TripRequest>::iterator request = privateCarRequests.begin(); request != privateCarRequests.end(); request++)
	{
		Person* bestDriver;
		double bestDistance = -1;
		double bestX, bestY;

		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		Node* destinationNode = it->second;

		auto person = availableDrivers.begin();

		while (person != availableDrivers.end())
		{
			if (!isCruising(*person))
			{
				person++;
				continue;
			}

			if (bestDistance < 0)
			{
				bestDriver = *person;

				const Node* node = getCurrentNode(bestDriver);
				bestDistance = std::abs(startNode->getPosX()
					- node->getPosX());
				bestDistance += std::abs(startNode->getPosY()
					- node->getPosY());
			}

			else
			{
				const Node* node = getCurrentNode(*person);
				double currDistance = std::abs(startNode->getPosX()
					- node->getPosX());
				currDistance += std::abs(startNode->getPosY()
					- node->getPosY());

				if (currDistance < bestDistance)
				{
					bestDriver = *person;
					bestDistance = currDistance;
					bestX = node->getPosX();
					bestY = node->getPosY();
				}
			}

			person++;
		}

		if (bestDistance == -1)
		{
			ControllerLog() << "No available vehicles" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
			continue;
		}

		ControllerLog() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;

		messaging::MessageBus::PostMessage((messaging::MessageHandler*) bestDriver, MSG_SCHEDULE_PROPOSITION, messaging::MessageBus::MessagePtr(
			new SchedulePropositionMessage(currTick, (*request).personId, (*request).startNodeId,
				(*request).destinationNodeId, (*request).extraTripTimeThreshold)));

		ControllerLog() << "Assignment sent for " << (*request).personId << " at time " << currTick.frame()
			<< ". Message was sent at " << (*request).currTick.frame() << " with startNodeId " << (*request).startNodeId
			<< ", destinationNodeId " << (*request).destinationNodeId << ", and driverId null" << std::endl;

		results.push_back(MESSAGE_SUCCESS);
	}

	for (std::vector<unsigned int>::iterator it = badRequests.begin(); it != badRequests.end(); it++)
	{
		results.insert(results.begin() + (*it), MESSAGE_ERROR_BAD_NODE);
	}

	return results;
}

bool SharedTaxiController::isCruising(Person* p) 
{
    MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        if (currDriver->getServiceStatus() == MobilityServiceDriver::SERVICE_FREE) 
        {
            return true;
        }
    }
    return false;
}

const Node* SharedTaxiController::getCurrentNode(Person* p) 
{
    MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        return currDriver->getCurrentNode();
    }
    return nullptr;
}
}

