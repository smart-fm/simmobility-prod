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
#include <sys/time.h>


namespace sim_mob
{
double SharedController::getTT(const Node* node1, const Node* node2) const
{
#ifndef NDEBUG
	if (
			(node1 == node2 && node1->getNodeId() !=  node2->getNodeId() ) ||
			(node1 != node2 && node1->getNodeId() ==  node2->getNodeId() )
	){
		throw std::runtime_error("Pointers of nodes do not correspond to their IDs for some weird reason");
	}
#endif

	double retValue;
	if (node1 == node2)
		retValue = 0;
	else{
		//retValue = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			//	node1->getNodeId(), node2->getNodeId(), DailyTime(currTick.ms()));
		retValue = PrivateTrafficRouteChoice::getInstance()->getShortestPathTravelTime(
					node1, node2, DailyTime(currTick.ms()));
		if (retValue <= 0)
		{	// The two nodes are different and the travel time should be non zero, if valid
			retValue = std::numeric_limits<double>::max();
		}
	}
	return retValue;
}

void SharedController::computeSchedules()
{
	const std::map<unsigned int, Node*>& nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
	std::vector<sim_mob::Schedule> schedules; // We will fill this schedules and send it to the best driver

	ControllerLog()<<"Requests to be scheduled "<< requestQueue.size() << ", available drivers "<<availableDrivers.size() <<std::endl;


	// aa: 	The i-th element of validRequests is the i-th valid request
	//		The i-th element of desiredTravelTimes is the desired travelTime for the
	//		i-th request. For the moment, the desired travel time is the travel
	//		time we would need if the trip is performed without sharing
	//		Each request is uniquely identified by its requestIndex, i.e, its position
	//		in the validRequests vector.
	//		We need to introduce a vector of validRequests that replicates the requestQueue
	//		since we need to associate a unique index to each request. To this aim, it is not
	//		convenient to rely on requestQueue, as we will need to have fast access to non-adjacent
	//		request in the requestQueue (in case we aggregate, for example, the i-th request with the j-th
	//		request in a shared trip)
	std::vector<TripRequestMessage> validRequests;
	std::vector<double> desiredTravelTimes;
	std::set<unsigned int> satisfiedRequestIndices;

	int profilingTime_current, profilingTime_previous, profilingTime_directTripTime, profilingTime_graphConstruction, profilingTime_Matching,
		profilingTime_sendAssignement, profilingTime_removeServedRequests;
	profilingTime_current= profilingTime_previous= profilingTime_directTripTime= profilingTime_graphConstruction= profilingTime_Matching=
			profilingTime_sendAssignement= profilingTime_removeServedRequests = -1;
	profilingTime_current = clock(); profilingTime_previous = profilingTime_current;



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
			throw std::runtime_error(msg.str() );
		}else if (itEnd == nodeIdMap.end())
		{
			std::stringstream msg; msg << "Request contains bad destination node " << (*request).startNodeId;
			throw std::runtime_error(msg.str() );
		}
#endif
		//} SANITY CHECK
		const Node* startNode = itStart->second;
		Node* destinationNode = itEnd->second;

		validRequests.push_back(*request);

		double tripTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			request->startNodeId, request->destinationNodeId, DailyTime(currTick.ms()));
		desiredTravelTimes.push_back(tripTime);

		request++;
		requestIndex++;
	}

#ifndef NDEBUG
	//{ CONSISTENCY CHECK
	unsigned howManyRequests = requestQueue.size();

	if (validRequests.size() != desiredTravelTimes.size() )
		throw std::runtime_error("validRequests and desiredTravlelTimes must have the same length");

	if (validRequests.size() != requestQueue.size() )
	{
		std::stringstream msg; msg<<"validRequests.size()="<<validRequests.size()<<", requestQueue.size()="
			<<requestQueue.size()<<std::endl; throw std::runtime_error(msg.str() );
	}

	consistencyChecks("Before constructing the graph");
	//} CONSISTENCY CHECK
#endif

	profilingTime_current = clock();
	profilingTime_directTripTime = profilingTime_current - profilingTime_previous;
	profilingTime_previous = profilingTime_current;

	if (!validRequests.empty() && !availableDrivers.empty() )
	{
		// 2. Add valid shared trips to graph
		// We construct a graph in which each node represents a trip. We will later draw and edge between two
		// two trips if they can be shared. Nodes are numbered starting from 0 in the graph
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
#ifndef NDEBUG
				//ControllerLog() << "DebugPrint, Checking if we can combine request " << request1Index << " and request " << request2Index << std::endl;

#endif
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
				double tripTime1 = getTT(startNode1, startNode2) + getTT(startNode2, destinationNode1);

				// We also compute the travel time that user 2 would experience
				double tripTime2 = getTT(startNode2, destinationNode1) + getTT(destinationNode1, destinationNode2);

				if ((tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
					&& (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold))
				{
					bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o1o2d1d2");
					add_edge(request1Index, request2Index, graph);
				}
				//} o1 o2 d1 d2

				//{ o2 o1 d2 d1
				tripTime1 = getTT(startNode1, destinationNode2) + getTT(destinationNode2, destinationNode1);

				tripTime2 = getTT(startNode2, startNode1) + getTT(startNode1, destinationNode2);

				if ((tripTime1 <= desiredTravelTimes.at(request1Index) + (*request1).extraTripTimeThreshold)
					&& (tripTime2 <= desiredTravelTimes.at(request2Index) + (*request2).extraTripTimeThreshold))
				{
					if (bestTrips.count(std::make_pair(request1Index, request2Index)) > 0)
					{
						std::pair<double, std::string> currBestTrip = bestTrips[std::make_pair(request1Index, request2Index)];

						if (tripTime1 + tripTime2 < currBestTrip.first)
						{
							bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d2d1");
						}
					}
					else
					{
						bestTrips[std::make_pair(request1Index, request2Index)] = std::make_pair(tripTime1 + tripTime2, "o2o1d2d1");
					}

					add_edge(request1Index, request2Index, graph);
				}
				//} o2 o1 d2 d1

				//{ o1 o2 d2 d1
				tripTime1 = getTT(startNode1, startNode2) + getTT(startNode2, destinationNode2) +getTT(destinationNode2, destinationNode1);

				// tripTime2 is ok, because user 2 does the same path as she was alone in the car

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
				tripTime2 = getTT(startNode2, startNode1)+ getTT(startNode1, destinationNode1)+ getTT(destinationNode1, destinationNode2);

				// tripTime2 is ok, because user 1 does the same path as she was alone in the car

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
#ifndef NDEBUG
					if (request1Index >= requestQueue.size()  || request2Index >= requestQueue.size())
					{
						std::stringstream msg; msg<< __FILE__<<":"<<__LINE__<< ":"<<" request1Index="<< request1Index<<
							" and request2Index="<<request2Index<< ", while they should both be requestQueue.size()= < "<<requestQueue.size();
						throw std::runtime_error(msg.str() );
					}

					if (requestQueue.size() != howManyRequests)
					{
						std::stringstream msg; msg<<"requestQueue.size() changed. Before it was "<< howManyRequests
						<<", while now it is "<<requestQueue.size();
					}
#endif
				}
				//} o2 o1 d1 d2

				request2++;
				request2Index++;
			}
			request1++;
			request1Index++;
		}


		profilingTime_current = clock();
		profilingTime_graphConstruction = profilingTime_current - profilingTime_previous;
		profilingTime_previous = profilingTime_current;

		ControllerLog() << "About to perform matching on "<< requestQueue.size()<< " requests and "<< availableDrivers.size() <<
				" drivers. Wish me luck" << std::endl;

		// 3. Perform maximum matching
		// aa: 	the following algorithm finds the maximum matching, a set of edges representing
		//		a matching such that no other matching with more edges is possible
		//		Edges constituting the matching are returned in mate.
		bool success = boost::checked_edmonds_maximum_cardinality_matching(graph, &mate[0]);

#ifndef NDEBUG
		if (!success)
		throw std::runtime_error("checked_edmonds_maximum_cardinality_matching(..) failed. Why?");
#endif



			ControllerLog() << "Found matching of size " << matching_size(graph, &mate[0])
					<< " for request list size of " << validRequests.size() << std::endl;

			boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::vertex_iterator vi, vi_end;


			for (	tie(vi,vi_end) = vertices(graph);
					vi != vi_end && schedules.size() < availableDrivers.size(); // We cannot assign more schedules than available drivers
					++vi
			){
				if (mate[*vi] != boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>>::null_vertex() && *vi < mate[*vi])
				{

					//aa{
					const unsigned request1Idx = *vi;
					const unsigned request2Idx = mate[*vi];
					const TripRequestMessage& request1 = validRequests.at(request1Idx);
					const TripRequestMessage& request2 = validRequests.at(request2Idx);
					const std::pair<double, std::string> sharedTripInfo =  bestTrips.at( std::make_pair(request1Idx, request2Idx) );
					const unsigned& totalTime = sharedTripInfo.first;
					const string& sequence = sharedTripInfo.second;

					TripRequestMessage firstPickUp, secondPickUp, firstDropOff, secondDropOff;
					if (sequence == "o1o2d1d2")
					{
						firstPickUp = validRequests.at(request1Idx);
						secondPickUp = validRequests.at(request2Idx);
						firstDropOff = validRequests.at(request1Idx);
						secondDropOff = validRequests.at(request2Idx);
					}else if (sequence == "o2o1d2d1")
					{
						firstPickUp = validRequests.at(request2Idx);
						secondPickUp = validRequests.at(request1Idx);
						firstDropOff = validRequests.at(request2Idx);
						secondDropOff = validRequests.at(request1Idx);
					}else if (sequence == "o1o2d2d1")
					{
						firstPickUp = validRequests.at(request1Idx);
						secondPickUp = validRequests.at(request2Idx);
						firstDropOff = validRequests.at(request2Idx);
						secondDropOff = validRequests.at(request1Idx);
					}else if (sequence == "o2o1d1d2")
					{
						firstPickUp = validRequests.at(request2Idx);
						secondPickUp = validRequests.at(request1Idx);
						firstDropOff = validRequests.at(request1Idx);
						secondDropOff = validRequests.at(request2Idx);
					}else
					{
						std::stringstream msg; msg<<__FILE__<<":"<<__LINE__ <<":Sequence "<<sequence<<" is not recognized";
						throw std::runtime_error(msg.str() );
					}

					Schedule schedule;
					schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, firstPickUp) );
					schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, secondPickUp) );
					schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, firstDropOff) );
					schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, secondDropOff) );
					schedules.push_back(schedule);


#ifndef NDEBUG
					if ( satisfiedRequestIndices.find(request1Idx) !=  satisfiedRequestIndices.end() )
										{
											std::stringstream msg; msg<<"line:"<<__LINE__<<"Index request1Idx="<<request1Idx<<" has already been added";
											throw std::runtime_error(msg.str());
										}
#endif
					satisfiedRequestIndices.insert(request1Idx);
#ifndef NDEBUG
					if ( satisfiedRequestIndices.find(request2Idx) !=  satisfiedRequestIndices.end() )
										{
											std::stringstream msg; msg<<"line:"<<__LINE__<<"Index request1Idx="<<request2Idx<<" has already been added";
											throw std::runtime_error(msg.str());
										}
#endif
					satisfiedRequestIndices.insert(request2Idx);



#ifndef NDEBUG
					if (request1Idx >= requestQueue.size()  || request2Idx >= requestQueue.size())
					{
						std::stringstream msg; msg<< __FILE__<<":"<<__LINE__<< ":"<<" request1Index="<< request1Idx<<
							" and request2Index="<<request2Idx<< ", while they should both be requestQueue.size()= "<<requestQueue.size();
						throw std::runtime_error(msg.str() );
					}

					if (requestQueue.size() != howManyRequests)
					{
						std::stringstream msg; msg<<"requestQueue.size() changed. Before it was "<< howManyRequests
						<<", while now it is "<<requestQueue.size();
					}
#endif
					//aa}
				}
				//aa{
				else // request vi is not matched with any other
				{
#ifndef NDEBUG
					if (validRequests.size() != requestQueue.size() )
					{
						std::stringstream msg; msg<<"validRequests.size()="<<validRequests.size()<<", requestQueue.size()="
							<<requestQueue.size()<<std::endl; throw std::runtime_error(msg.str() );
					}
#endif
					const TripRequestMessage& request = validRequests.at(*vi);
					const unsigned request1Idx = *vi;
					if (satisfiedRequestIndices.find(request1Idx) ==  satisfiedRequestIndices.end() )
					{
						satisfiedRequestIndices.insert(request1Idx);
						Schedule schedule;
						schedule.push_back( ScheduleItem(ScheduleItemType::PICKUP, request) );
						schedule.push_back( ScheduleItem(ScheduleItemType::DROPOFF, request) );
						schedules.push_back(schedule);

					}


#ifndef NDEBUG
					if (request1Idx >= requestQueue.size()   )
					{
						std::stringstream msg; msg<< __FILE__<<":"<<__LINE__<< ":"<<" request1Index="<< request1Idx<<
							 ", while it should be less than requestQueue.size()= "<<requestQueue.size();
						throw std::runtime_error(msg.str() );
					}

					if (requestQueue.size() != howManyRequests)
					{
						std::stringstream msg; msg<<"requestQueue.size() changed. Before it was "<< howManyRequests
						<<", while now it is "<<requestQueue.size();
					}


#endif




				}
				//aa}
			}

#ifndef NDEBUG
		// 3.1 Check if the same passenger is inserted in more than one schedule
		std::set<std::string> passengerIds;
		for (const Schedule& schedule : schedules)
		for (const ScheduleItem& item : schedule)
		{
			if ( item.scheduleItemType == ScheduleItemType::PICKUP )
			{
				if (passengerIds.find( item.tripRequest.userId) != passengerIds.end() )
				{
					std::stringstream msg; msg<<"The same passenger "<<item.tripRequest.userId<<" is supposed to be picked up multiple times."
						<<" This is impossible as she is not ubiquitous!";
					throw std::runtime_error(msg.str() );
				}else passengerIds.insert(item.tripRequest.userId);
			}
		}

#endif

		profilingTime_current = clock();
		profilingTime_Matching = profilingTime_current - profilingTime_previous;
		profilingTime_previous = profilingTime_current;

		// 4. Send assignments for requests
		for (const Schedule& schedule : schedules)
		{
			const ScheduleItem& firstScheduleItem = schedule.front();
			const TripRequestMessage& firstRequest = firstScheduleItem.tripRequest;

			const unsigned startNodeId = firstRequest.startNodeId;
			const Node* startNode = nodeIdMap.find(startNodeId)->second;

			const Person* bestDriver = findClosestDriver(startNode);

			assignSchedule(bestDriver, schedule);

			ControllerLog() << "Schedule for the " << firstRequest.userId << " at time " << currTick.frame()
				<< ". Message was sent at " << firstRequest.timeOfRequest.frame() << " with startNodeId " << firstRequest.startNodeId
				<< ", destinationNodeId " << firstRequest.destinationNodeId << ", and driverId "<< bestDriver->getDatabaseId();

			const ScheduleItem& secondScheduleItem = schedule.at(1);
			if ( secondScheduleItem.scheduleItemType == ScheduleItemType::PICKUP )
			{

#ifndef NDEBUG
				//aa{ CONSISTENCY CHECKS
				if (secondScheduleItem.tripRequest.userId ==  firstRequest.userId)
				{
					std::stringstream msg; msg<<"Malformed schedule. Trying to pick up twice the same person "<<firstRequest.userId;
					throw ScheduleException(msg.str() );
				}

				if (requestQueue.size() != howManyRequests)
				{
					std::stringstream msg; msg<<"requestQueue.size() changed. Before it was "<< howManyRequests
					<<", while now it is "<<requestQueue.size();
				}
				//aa} CONSISTENCY CHECKS
#endif
				ControllerLog()<<". The trip is shared with person " << secondScheduleItem.tripRequest.userId;
			}
			ControllerLog()<<std::endl;
		}

		profilingTime_current = clock();
		profilingTime_sendAssignement = profilingTime_current - profilingTime_previous;
		profilingTime_previous = profilingTime_current ;

		// 5. Remove from the pending requests the ones that have been assigned
		std::list<TripRequestMessage>::iterator requestToEliminate = requestQueue.begin();
		int lastEliminatedIndex = -1;

#ifndef NDEBUG
		unsigned eliminationsPerformed=0;
#endif

		//// DebugPrint
		// Print()<<"\n\n\n DebugPrint, satisfiedRequestIndices=";
		// for (const unsigned satisfiedRequestIndex : satisfiedRequestIndices) Print()<< satisfiedRequestIndex<<", "; Print()<<std::endl;
		// Print()<<"DebugPrint, requestQueue = " << getRequestQueueStr() << std::endl;


		for (const unsigned satisfiedRequestIndex : satisfiedRequestIndices)
		{
			// Print()<<"DebugPrint, eliminating "<<satisfiedRequestIndex<<", while requests are "<<getRequestQueueStr()<<std::endl;
			int advancement = satisfiedRequestIndex - lastEliminatedIndex - 1;
			std::advance(requestToEliminate,  advancement);
#ifndef NDEBUG

			if (requestToEliminate == requestQueue.end() )
			{
				std::stringstream msg; msg << "Trying to eliminate a request " << satisfiedRequestIndex << " that is not in the satisfiedRequests, which are ";
				for (const unsigned s : satisfiedRequestIndices)
					msg<< s <<", ";
				msg<<". Eliminations performed before "<< eliminationsPerformed;
				throw std::runtime_error(msg.str() );
			}
			eliminationsPerformed++;
#endif
			requestToEliminate = requestQueue.erase(requestToEliminate);
			lastEliminatedIndex = satisfiedRequestIndex;

		}

#ifndef NDEBUG
		if (validRequests.size() - satisfiedRequestIndices.size() != requestQueue.size() )
		{
			std::stringstream msg; msg <<"We should have validRequests.size() - satisfiedRequestIndices.size() == requestQueue.size(), while "
				<< validRequests.size()<<" - "<<satisfiedRequestIndices.size()<<" != "<<requestQueue.size();
			throw std::runtime_error(msg.str() );
		}

		if (validRequests.size() != howManyRequests)
		{
			std::stringstream msg; msg<<"validRequests.size() changed. Before it was "<< howManyRequests
			<<", while now it is "<<validRequests.size();
		}
#endif

	}

	profilingTime_current = clock();
	profilingTime_removeServedRequests = profilingTime_current - profilingTime_previous;
	profilingTime_previous = profilingTime_current ;



	ControllerLog()<<"Schedules computed, now Requests to be scheduled "<< requestQueue.size() << ", available drivers "<<availableDrivers.size();
	ControllerLog()<<". Performance profilingTime_directTripTime="<<toMs(profilingTime_directTripTime)<<", profilingTime_graphConstruction="<<
			toMs(profilingTime_graphConstruction)<<", profilingTime_Matching="<<toMs(profilingTime_Matching)<<", profilingTime_sendAssignement"<<
			toMs(profilingTime_sendAssignement)<<", profilingTime_removeServedRequests="<<toMs(profilingTime_removeServedRequests)<<std::endl;
}

double SharedController::toMs(int c) const
{
	return c / ( CLOCKS_PER_SEC / 1000 );
}

bool SharedController::isCruising(Person* p)
{
    const MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        if (currDriver->getDriverStatus() == MobilityServiceDriverStatus::CRUISING)
        {
            return true;
        }
    }
    return false;
}

const Node* SharedController::getCurrentNode(Person* p)
{
    const MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        return currDriver->getCurrentNode();
    }
    return nullptr;
}


void SharedController::checkSequence(const std::string &sequence) const
{
	if (sequence != "o1o2d1d2" && sequence != "o2o1d2d1" && sequence != "o1o2d2d1" && sequence != "o2o1d1d2")
	{
		std::stringstream msg;
		msg << __FILE__ << ":" << __LINE__ << ": sequence " << sequence << " is not recognized";
		throw std::runtime_error(msg.str());
	}
}

void SharedController::sendCruiseCommand(const Person* driver, const Node* nodeToCruiseTo, const timeslice currTick ) const
{
	if ( driverSchedules.find(driver) == driverSchedules.end()  )
	{
		std::stringstream msg; msg<<"Trying to send a message to driver "<<driver->getDatabaseId()<<" pointer "<< driver<<
				" who has not entry in the driverSchedules";
				throw std::runtime_error(msg.str() );
	}
	OnCallController::sendCruiseCommand(driver, nodeToCruiseTo, currTick );
}

#ifndef NDEBUG
void SharedController::consistencyChecks(const std::string& label) const
{
	OnCallController::consistencyChecks(label);
}
#endif

}

