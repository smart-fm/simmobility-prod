/*
 * SharedController.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "SharedController.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob
{
std::vector<MobilityServiceController::MessageResult> SharedController::computeSchedules()
{
	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	std::vector<sim_mob::Schedule> schedules; // We will fill this schedules and send it to the best driver
	std::vector<MobilityServiceController::MessageResult> results;



	// aa: 	The i-th element of validRequests is the i-th valid request
	//		The i-th element of desiredTravelTimes is the desired travelTime for the
	//		i-th request. For the moment, the desired travel time is the travel
	//		time we would need if the trip is performed without sharing
	//		Each request is uniquely identified by its requestIndex, i.e, its position
	//		in the validRequests vector.
	std::vector<TripRequestMessage> validRequests;
	std::vector<double> desiredTravelTimes;

	std::vector<unsigned int> badRequests;

	// 1. Calculate times for direct trips
	unsigned int requestIndex = 0;
	auto request = requestQueue.begin();
	while (request != requestQueue.end())
	{
		std::map<unsigned int, Node*>::const_iterator itStart = nodeIdMap.find((*request).startNodeId);
		std::map<unsigned int, Node*>::const_iterator itEnd = nodeIdMap.find((*request).startNodeId);
		//{ SANITY CHECK
#ifndef NDEBUG
		if (itStart == nodeIdMap.end())
		{
			std::stringstream msg; msg << "Request contains bad start node " << (*request).startNodeId ;
			ControllerLog() << msg <<std::endl;
			throw std::runtime_error(msg.str() );
			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}

		if (itEnd == nodeIdMap.end())
		{
			std::stringstream msg; msg << "Request contains bad destination node " << (*request).startNodeId;
			ControllerLog() << msg << std::endl;

			badRequests.push_back(requestIndex);

			request++;
			requestIndex++;
			continue;
		}
#endif
		//} SANITY CHECK
		const Node* startNode = itStart->second;
		Node* destinationNode = itEnd->second;

		validRequests.push_back(*request);

		double tripTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			startNode->getNodeId(), destinationNode->getNodeId(), DailyTime(currTick.ms()));
		desiredTravelTimes.push_back(tripTime);

		request++;
		requestIndex++;
	}

#ifndef NDEBUG
	//{ CONSISTENCY CHECK
	if (validRequests.size() != desiredTravelTimes.size() )
		throw std::runtime_error("validRequests and desiredTravlelTimes should be equivalent");
	//} CONSISTENCY CHECK
#endif

	if (!validRequests.empty() )
	{
		// 2. Add valid shared trips to graph
		// We construct a graph in which each node represents a trip. We will later draw and edge between two
		// two trips if they can be shared
		boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> graph(validRequests.size());
		std::vector<boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::vertex_descriptor> mate(validRequests.size());

		std::map<std::pair<unsigned int, unsigned int>, std::pair<double, std::string>> bestTrips;

		auto request1 = validRequests.begin();
		unsigned int request1Index = 0;
		while (request1 != validRequests.end())
		{
			// We check if request1 can be shared with some of the following requests in the queue
			auto request2 = request1 + 1;
			unsigned int request2Index = request1Index + 1;
			while (request2 != validRequests.end())
			{
				ControllerLog() << "(" << request1Index << ", " << request2Index << ")" << std::endl;
				std::map<unsigned int, Node*>::const_iterator it = nodeIdMap.find((*request1).startNodeId);
				const Node* startNode1 = it->second;

				it = nodeIdMap.find((*request1).destinationNodeId);
				Node* destinationNode1 = it->second;

				it = nodeIdMap.find((*request2).startNodeId);
				Node* startNode2 = it->second;

				it = nodeIdMap.find((*request2).destinationNodeId);
				Node* destinationNode2 = it->second;


				// We now check if we can combine trip 1 and trip 2. They can be combined in different ways.
				// For example, we can
				// 		i) pick up user 1, ii) pick up user 2, iii) drop off user 1, iv) drop off user 2
				// (which we indicate with o1 o2 d1 d2), or we can
				//		i) pick up user 2, ii) pick up user 1, iii) drop off user 2, iv) drop off user 1
				// and so on. When trip 1 is combined with trip 2, user 1 experiences some additional delays
				// w.r.t. the case when each user travels alone. A combination is feasible if this extra-delay
				// induced by sharing is below a certain threshold.
				// In the following line, we check what are the feasible combination and we select the
				// "best", i.e., the one with the minimum induce extra-delay.

				//{ o1 o2 d1 d2
				// We compute the travel time that user 1 would experience in this case
				double tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(currTick.ms()))
					+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
						startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(currTick.ms()));

				// We also compute the travel time that user 2 would experience
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
				//} o1 o2 d1 d2

				//{ o2 o1 d2 d1
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
				//} o2 o1 d2 d1

				//{ o1 o2 d2 d1
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
				//} o1 o2 d2 d1

				//{ o2 o1 d1 d2
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
				//} o2 o1 d1 d2

				request2++;
				request2Index++;
			}
			request1++;
			request1Index++;
		}

		ControllerLog() << "About to perform matching, wish me luck" << std::endl;

		// 3. Perform maximum matching
		// aa: 	the following algorithm finds the maximum matching, a set of edges representing
		//		a matching such that no other matching with more edges is possible
		//		Edges constituting the matching are returned in mate.
		bool success = boost::checked_edmonds_maximum_cardinality_matching(graph, &mate[0]);

		if (success & !availableDrivers.empty())
		{
			ControllerLog() << "Found matching of size " << matching_size(graph, &mate[0])
					<< " for request list size of " << validRequests.size() << std::endl;

			boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::vertex_iterator vi, vi_end;


			for (tie(vi,vi_end) = vertices(graph); vi != vi_end; ++vi)
			{
				if (mate[*vi] != boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::null_vertex() && *vi < mate[*vi])
				{

					//aa{
					const unsigned request1Index = *vi;
					const unsigned request2Index = mate[*vi];
					const TripRequestMessage& request1 = validRequests.at(request1Index);
					const TripRequestMessage& request2 = validRequests.at(request2Index);
					const std::pair<double, std::string> sharedTripInfo =  bestTrips.at( std::make_pair(request1Index, request2Index) );
					const unsigned& totalTime = sharedTripInfo.first;
					const string& sequence = sharedTripInfo.second;

					TripRequestMessage firstPickUp, secondPickUp, firstDropOff, secondDropOff;
					if (sequence == "o1o2d1d2")
					{
						firstPickUp = validRequests.at(request1Index);
						secondPickUp = validRequests.at(request2Index);
						firstDropOff = validRequests.at(request1Index);
						secondDropOff = validRequests.at(request2Index);
					}else if (sequence == "o2o1d2d1")
					{
						firstPickUp = validRequests.at(request2Index);
						secondPickUp = validRequests.at(request1Index);
						firstDropOff = validRequests.at(request2Index);
						secondDropOff = validRequests.at(request1Index);
					}else if (sequence == "o1o2d2d1")
					{
						firstPickUp = validRequests.at(request1Index);
						secondPickUp = validRequests.at(request2Index);
						firstDropOff = validRequests.at(request2Index);
						secondDropOff = validRequests.at(request1Index);
					}else if (sequence == "o2o1d1d2")
					{
						firstPickUp = validRequests.at(request2Index);
						secondPickUp = validRequests.at(request1Index);
						firstDropOff = validRequests.at(request1Index);
						secondDropOff = validRequests.at(request2Index);
					}else
					{
						std::stringstream msg; msg<<"Sequence "<<sequence<<" is not recognized";
						throw std::runtime_error(msg.str() );
					}

					Schedule schedule;
					schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, firstPickUp) );
					schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, secondPickUp) );
					schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, firstDropOff) );
					schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, secondDropOff) );
					schedules.push_back(schedule);
					//aa}
				}
				//aa{
				else // request vi is not matched with any other
				{
					const TripRequestMessage& request = validRequests.at(*vi);
					Schedule schedule;
					schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, request) );
					schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, request) );
					schedules.push_back(schedule);
				}
				//aa}
			}
		}
		else
		{
			std::stringstream msg; msg << "Did not find matching" << std::endl;
			std::runtime_error(msg.str() );
		}

		// 4. Send assignments for requests
		for (const Schedule& schedule : schedules)
		{
			const ScheduleItem& firstScheduleItem = schedule.front();
			const TripRequestMessage& firstRequest = firstScheduleItem.tripRequest;

			const unsigned startNodeId = firstRequest.startNodeId;
			const Node* startNode = nodeIdMap.find(startNodeId)->second;

			const Person* bestDriver = findClosestDriver(startNode);
			sendScheduleProposition(bestDriver, schedule);

			ControllerLog() << "Schedule for the " << firstRequest.personId << " at time " << currTick.frame()
				<< ". Message was sent at " << firstRequest.currTick.frame() << " with startNodeId " << firstRequest.startNodeId
				<< ", destinationNodeId " << firstRequest.destinationNodeId << ", and driverId "<< bestDriver->getDatabaseId();

			const ScheduleItem& secondScheduleItem = schedule.at(1);
			if ( secondScheduleItem.scheduleItemType == ScheduleItemType::PICKUP )
			{

#ifndef NDEBUG
				//aa{ CONSISTENCY CHECKS
				if (secondScheduleItem.tripRequest.personId ==  firstRequest.personId)
				{
					std::stringstream msg; msg<<"Malformed schedule. Trying to pick up twice the same person "<<firstRequest.personId;
					throw ScheduleException(msg.str() );
				}
				//aa} CONSISTENCY CHECKS
#endif
				ControllerLog()<<". The trip is shared with person " << secondScheduleItem.tripRequest.personId;
			}
			ControllerLog()<<std::endl;

			results.push_back(MESSAGE_SUCCESS);
		}


		for (std::vector<unsigned int>::iterator it = badRequests.begin(); it != badRequests.end(); it++)
		{
			results.insert(results.begin() + (*it), MESSAGE_ERROR_BAD_NODE);
		}
	}
	return results;
}

bool SharedController::isCruising(Person* p)
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

const Node* SharedController::getCurrentNode(Person* p)
{
    MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        return currDriver->getCurrentNode();
    }
    return nullptr;
}
}

