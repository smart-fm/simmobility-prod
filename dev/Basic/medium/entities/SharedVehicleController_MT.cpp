/*
 * SharedVehicleController_MT.cpp
 *
 *  Created on: Apr 13, 2017
 *      Author: Akshay Padmanabha
 */

#include "SharedVehicleController_MT.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

#include "entities/roles/driver/TaxiDriver.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "message/MessageBus.hpp"
#include "message/VehicleControllerMessage.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob
{
std::vector<VehicleController::MessageResult> SharedVehicleController_MT::assignVehiclesToRequests()
{
	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

	std::vector<VehicleRequest> validRequests;
	std::vector<double> desiredTravelTimes;
	std::vector<unsigned int> badRequests;

	// 1. Calculate times for direct trips
	unsigned int requestIndex = 0;
	auto request = requestQueue.begin();
	while (request != requestQueue.end())
	{
		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad start node " << (*request).startNodeId << std::endl;

			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad destination node " << (*request).startNodeId << std::endl;

			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}
		Node* destinationNode = it->second;

		validRequests.push_back(*request);

		double tripTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			startNode->getNodeId(), destinationNode->getNodeId(), DailyTime(currTimeSlice.ms()));
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
			Print() << "(" << request1Index << ", " << request2Index << ")" << std::endl;
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
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTimeSlice.ms()));

			double tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTimeSlice.ms()));

			if ((tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
				&& (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold))
			{
				bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o1o2d1d2");
				add_edge(request1Index, request2Index, graph);
			}

			// o2 o1 d2 d1
			tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTimeSlice.ms()));

			tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTimeSlice.ms()));

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
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTimeSlice.ms()));

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
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTimeSlice.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(currTimeSlice.ms()));

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

	Print() << "About to perform matching, wish me luck" << std::endl;

	// 3. Perform maximum matching
	std::vector<VehicleRequest> privateCarRequests;
	bool success = boost::checked_edmonds_maximum_cardinality_matching(graph, &mate[0]);

	if (success) {
		Print() << "Found matching of size " << matching_size(graph, &mate[0])
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

		for (std::vector<VehicleRequest>::iterator it = validRequests.begin(); it != validRequests.end(); it++)
		{
			// if (sharedTrips.count(requestIndex) == 0)
			// {
			// 	privateCarRequests.push_back(*it);
			// }

			privateCarRequests.push_back(*it);
			requestIndex++;
		}

	} else {
		Print() << "Did not find matching" << std::endl;

		for (std::vector<VehicleRequest>::iterator it = validRequests.begin(); it != validRequests.end(); it++)
		{
			privateCarRequests.push_back(*it);
		}
	}

	// 4. Send assignments for requests
	std::vector<VehicleController::MessageResult> results;

	for (std::vector<VehicleRequest>::iterator request = privateCarRequests.begin(); request != privateCarRequests.end(); request++)
	{
		medium::TaxiDriver* bestDriver;
		double bestDistance = -1;
		double bestX, bestY;

		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		Node* destinationNode = it->second;

		auto person = vehicleDrivers.begin();

		while (person != vehicleDrivers.end())
		{
			medium::Person_MT* person_MT = dynamic_cast<medium::Person_MT*>(*person);

			if (person_MT) {
				if (person_MT->getRole())
				{
					medium::TaxiDriver* currDriver = dynamic_cast<medium::TaxiDriver*>(person_MT->getRole());
					if (currDriver)
					{
						if (currDriver->getDriverMode() != medium::CRUISE)
						{
							person++;
							continue;
						}

						if (bestDistance < 0)
						{
							bestDriver = currDriver;

							medium::TaxiDriverMovement* movement = bestDriver
								->getMovementFacet();
							const Node* node = movement->getCurrentNode();

							bestDistance = std::abs(startNode->getPosX()
								- node->getPosX());
							bestDistance += std::abs(startNode->getPosY()
								- node->getPosY());
						}

						else
						{
							double currDistance = 0.0;

							medium::TaxiDriverMovement* movement = currDriver
								->getMovementFacet();
							const Node* node = movement->getCurrentNode();

							currDistance = std::abs(startNode->getPosX()
								- node->getPosX());
							currDistance += std::abs(startNode->getPosY()
								- node->getPosY());

							if (currDistance < bestDistance)
							{
								bestDriver = currDriver;
								bestDistance = currDistance;
								bestX = node->getPosX();
								bestY = node->getPosY();
							}
						}
					}
				}
			}

			person++;
		}

		if (bestDistance == -1)
		{
			Print() << "No available vehicles" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
			continue;
		}

		Print() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;

		messaging::MessageBus::PostMessage(bestDriver->getParent(), MSG_VEHICLE_ASSIGNMENT, messaging::MessageBus::MessagePtr(
			new VehicleAssignmentMessage(currTick, (*request).personId, (*request).startNodeId,
				(*request).destinationNodeId, (*request).extraTripTimeThreshold)));

		Print() << "Assignment sent for " << (*request).personId << " at time " << currTick.frame()
			<< ". Message was sent at " << (*request).currTick.frame() << " with startNodeId " << (*request).startNodeId
			<< ", destinationNodeId " << (*request).destinationNodeId << ", and driverId null" << std::endl;

		results.push_back(MESSAGE_SUCCESS);
		continue;
	}

	for (std::vector<unsigned int>::iterator it = badRequests.begin(); it != badRequests.end(); it++)
	{
		results.insert(results.begin() + (*it), MESSAGE_ERROR_BAD_NODE);
	}

	return results;
}
}


