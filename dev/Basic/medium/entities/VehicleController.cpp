/*
 * VehicleController.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "VehicleController.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include <soci/postgresql/soci-postgresql.h>
#include <soci/soci.h>
#include "util/Utils.hpp"
#include <iostream>
#include <boost/date_time.hpp>
#include "../shared/message/MessageBus.hpp"
#include "path/PathSetManager.hpp"
#include "geospatial/network/RoadNetwork.hpp"

namespace bt = boost::posix_time;
namespace sim_mob
{
namespace medium
{
VehicleController* VehicleController::instance = nullptr;

VehicleController* VehicleController::GetInstance()
{
	return instance;
}

VehicleController::~VehicleController()
{
}

bool VehicleController::RegisterVehicleController(int id,
	const MutexStrategy& mtxStrat,
	int tickRefresh,
	double shareThreshold)
{
	VehicleController *vehicleController = new VehicleController(id, mtxStrat, tickRefresh, shareThreshold);

	if (!instance)
	{
		instance = vehicleController;
		return true;
	}
	return false;
}

bool VehicleController::HasVehicleController()
{
	return (instance != nullptr);
}

void VehicleController::addVehicleDriver(Person_MT* person)
{
	VehicleController::vehicleDrivers.push_back(person);
}

void VehicleController::removeVehicleDriver(Person_MT* person)
{
	vehicleDrivers.erase(std::remove(vehicleDrivers.begin(),
		vehicleDrivers.end(), person), vehicleDrivers.end());
}


void VehicleController::assignVehicleToRequest(VehicleRequest request) {
	TaxiDriver* best_driver;
	double best_distance = -1;
	double best_x, best_y;

	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

	std::map<unsigned int, Node*>::iterator it = nodeIdMap.find(request.startNodeId); 
	if (it == nodeIdMap.end()) {
		printf("Request contains bad start node\n");
		return;
	}
	Node* startNode = it->second;

	it = nodeIdMap.find(request.destinationNodeId); 
	if (it == nodeIdMap.end()) {
		printf("Request contains bad destination node\n");
		return;
	}
	Node* destinationNode = it->second;

	printf("Request made from (%f, %f)\n", startNode->getPosX(), startNode->getPosY());
	auto person = vehicleDrivers.begin();

	while (person != vehicleDrivers.end())
	{
		if ((*person)->getRole())
		{
			TaxiDriver* curr_driver = dynamic_cast<TaxiDriver*>((*person)->getRole());
			if (curr_driver)
			{
				if (curr_driver->getDriverMode() != CRUISE)
				{
					person++;
					continue;
				}

				if (best_distance < 0)
				{
					best_driver = curr_driver;

					TaxiDriverMovement* movement = best_driver
						->getMovementFacet();
					const Node* node = movement->getCurrentNode();

					best_distance = std::abs(startNode->getPosX()
						- node->getPosX());
					best_distance += std::abs(startNode->getPosY()
						- node->getPosY());
				}

				else
				{
					double curr_distance = 0.0;

					// TODO: Find shortest path instead
					TaxiDriverMovement* movement = curr_driver
						->getMovementFacet();
					const Node* node = movement->getCurrentNode();

					curr_distance = std::abs(startNode->getPosX()
						- node->getPosX());
					curr_distance += std::abs(startNode->getPosY()
						- node->getPosY());

					if (curr_distance < best_distance)
					{
						best_driver = curr_driver;
						best_distance = curr_distance;
						best_x = node->getPosX();
						best_y = node->getPosY();
					}
				}
			}
		}

		person++;
	}

	if (best_distance == -1)
	{
		printf("No available taxis\n");
	}

	messaging::MessageBus::PostMessage(best_driver->getParent(), CALL_TAXI, messaging::MessageBus::MessagePtr(
		new TaxiCallMessage(request.personId, destinationNode)));

	printf("Closest taxi is at (%f, %f)\n", best_x, best_y);
}

void VehicleController::assignSharedVehicles(std::vector<Person_MT*> drivers,
	std::vector<VehicleRequest> requests,
	timeslice now)
{
	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

	std::vector<double> desiredTravelTimes;

	auto request = requests.begin();
	while (request != requests.end())
	{
		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end()) {
			printf("Request contains bad start node\n");
			return;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			printf("Request contains bad destination node\n");
			return;
		}
		Node* destinationNode = it->second;

		double tripTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
			startNode->getNodeId(), destinationNode->getNodeId(), DailyTime(now.ms()));
		desiredTravelTimes.push_back(tripTime);

		request++;
	}

	auto request1 = requests.begin();
	int reqest1Index = 0;
	while (request1 != requests.end())
	{
		auto request2 = request1 + 1;
		int reqest2Index = reqest1Index + 1;
		while (request2 != requests.end())
		{
			std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request1).startNodeId); 
			if (it == nodeIdMap.end()) {
				printf("Request contains bad start node\n");
				return;
			}
			Node* startNode1 = it->second;

			it = nodeIdMap.find((*request1).destinationNodeId); 
			if (it == nodeIdMap.end()) {
				printf("Request contains bad destination node\n");
				return;
			}
			Node* destinationNode1 = it->second;

			it = nodeIdMap.find((*request2).startNodeId); 
			if (it == nodeIdMap.end()) {
				printf("Request contains bad start node\n");
				return;
			}
			Node* startNode2 = it->second;

			it = nodeIdMap.find((*request2).destinationNodeId); 
			if (it == nodeIdMap.end()) {
				printf("Request contains bad destination node\n");
				return;
			}
			Node* destinationNode2 = it->second;

			// o1 o2 d1 d2
			double tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(now.ms()));

			double tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(now.ms()));

			if ((tripTime1 <= desiredTravelTimes.at(reqest1Index) + timedelta) && (tripTime2 <= desiredTravelTimes.at(reqest2Index) + timedelta))
			{
				// TODO: construct graph
			}

			// o2 o1 d2 d1
			tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(now.ms()));

			tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(now.ms()));

			if ((tripTime1 <= desiredTravelTimes.at(reqest1Index) + timedelta) && (tripTime2 <= desiredTravelTimes.at(reqest2Index) + timedelta))
			{
				// TODO: construct graph
			}

			// o1 o2 d2 d1
			tripTime1 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode1->getNodeId(), startNode2->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode2->getNodeId(), destinationNode2->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode2->getNodeId(), destinationNode1->getNodeId(), DailyTime(now.ms()));

			if (tripTime1 <= desiredTravelTimes.at(reqest1Index) + timedelta)
			{
				// TODO: construct graph
			}

			// o2 o1 d1 d2
			tripTime2 = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
				startNode2->getNodeId(), startNode1->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					startNode1->getNodeId(), destinationNode1->getNodeId(), DailyTime(now.ms()))
				+ PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
					destinationNode1->getNodeId(), destinationNode2->getNodeId(), DailyTime(now.ms()));

			if (tripTime2 <= desiredTravelTimes.at(reqest2Index) + timedelta)
			{
				// TODO: construct graph
			}

			// TODO: Mark all compatible trips here

			request2++;
			reqest2Index++;
		}
		request1++;
		reqest1Index++;
	}
}

Entity::UpdateStatus VehicleController::frame_init(timeslice now)
{
	if (!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
		test.push_back(now);
		timedelta = 20;
	}
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus VehicleController::frame_tick(timeslice now)
{
	mtx.lock();
	if (currTick == tickThreshold)
	{
		currTick = 0;
		assignSharedVehicles(vehicleDrivers, requestQueue, now);
		requestQueue.clear();
	} else
	{
		currTick += 1;
	}
	mtx.unlock();

	return Entity::UpdateStatus::Continue;

	mtx.lock();
	if (currTick == tickThreshold)
	{
		currTick = 0;

		auto request = requestQueue.begin();
		while (request != requestQueue.end())
		{
			assignVehicleToRequest(*request);
			request++;
		}

		requestQueue.clear();
	}
	else
	{
		currTick += 1;
	}
	mtx.unlock();

	return Entity::UpdateStatus::Continue;
}

void VehicleController::frame_output(timeslice now)
{

}

void VehicleController::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	mtx.lock();
	switch (type) {
	        case MSG_VEHICLE_REQUEST:
	        {
				const VehicleRequestMessage& requestArgs = MSG_CAST(VehicleRequestMessage, message);
				requestQueue.push_back({requestArgs.personId, requestArgs.startNodeId, requestArgs.destinationNodeId});
	            break;
	        }

	        default:break;
	    };
	mtx.unlock();
}

bool VehicleController::isNonspatial()
{
	return true;
}
}
}



