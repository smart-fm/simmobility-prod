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


int VehicleController::assignVehicleToRequest(VehicleRequest request) {
	TaxiDriver* best_driver;
	double best_distance = -1;
	double best_x, best_y;

	std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

	std::map<unsigned int, Node*>::iterator it = nodeIdMap.find(request.startNodeId); 
	if (it == nodeIdMap.end()) {
		Print() << "Request contains bad start node" << std::endl;
		return PARSING_FAILED;
	}
	Node* startNode = it->second;

	it = nodeIdMap.find(request.destinationNodeId); 
	if (it == nodeIdMap.end()) {
		Print() << "Request contains bad destination node" << std::endl;
		return PARSING_FAILED;
	}
	Node* destinationNode = it->second;

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
		Print() << "No available taxis" << std::endl;
		return PARSING_RETRY;
	}

	Print() << "Closest taxi is at (" << best_x << ", " << best_y << ")" << std::endl;;

	messaging::MessageBus::PostMessage(best_driver->getParent(), CALL_TAXI, messaging::MessageBus::MessagePtr(
		new TaxiCallMessage(currTick, request.personId, request.startNodeId, request.destinationNodeId)));

	Print() << "Assignment sent for " << request.personId << " at time " << currTick.frame()
		<< ". Message was sent at " << request.currTick.frame() << " with startNodeId " << request.startNodeId
		<< ", destinationNodeId " << request.destinationNodeId << ", and taxiDriverId null" << std::endl;

	return PARSING_SUCCESS;
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
			Print() << "Request contains bad start node" << std::endl;
			return;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad destination node" << std::endl;
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
				Print() << "Request contains bad start node" << std::endl;
				return;
			}
			Node* startNode1 = it->second;

			it = nodeIdMap.find((*request1).destinationNodeId); 
			if (it == nodeIdMap.end()) {
				Print() << "Request contains bad destination node" << std::endl;
				return;
			}
			Node* destinationNode1 = it->second;

			it = nodeIdMap.find((*request2).startNodeId); 
			if (it == nodeIdMap.end()) {
				Print() << "Request contains bad start node" << std::endl;
				return;
			}
			Node* startNode2 = it->second;

			it = nodeIdMap.find((*request2).destinationNodeId); 
			if (it == nodeIdMap.end()) {
				Print() << "Request contains bad destination node" << std::endl;
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
		timedelta = 20;
	}
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus VehicleController::frame_tick(timeslice now)
{
	// mtx.lock();
	// if (currIntTick == tickThreshold)
	// {
	// 	currIntTick = 0;
	// 	assignSharedVehicles(vehicleDrivers, requestQueue, now);
	// 	requestQueue.clear();
	// } else
	// {
	// 	currIntTick += 1;
	// }
	// mtx.unlock();

	// return Entity::UpdateStatus::Continue;

	if (currIntTick == tickThreshold)
	{
		currIntTick = 0;

		std::vector<VehicleRequest> retryRequestQueue;

		std::vector<VehicleRequest>::iterator request = requestQueue.begin();
		while (request != requestQueue.end())
		{
			int r = assignVehicleToRequest(*request);

		    if (r == PARSING_RETRY) retryRequestQueue.push_back(*request);
		    request++;
		}

		requestQueue.clear();

		request = retryRequestQueue.begin();
		while (request != retryRequestQueue.end())
		{
		    requestQueue.push_back(*request);
		    request++;
		}
	}
	else
	{
		currIntTick += 1;
	}

	return Entity::UpdateStatus::Continue;
}

void VehicleController::frame_output(timeslice now)
{

}

void VehicleController::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type) {
	        case MSG_VEHICLE_REQUEST:
	        {
				const VehicleRequestMessage& requestArgs = MSG_CAST(VehicleRequestMessage, message);

				Print() << "Request received from " << requestArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
					<< requestArgs.currTick.frame() << " with startNodeId " << requestArgs.startNodeId << ", destinationNodeId "
					<< requestArgs.destinationNodeId << ", and taxiDriverId null" << std::endl;

				requestQueue.push_back({requestArgs.currTick, requestArgs.personId, requestArgs.startNodeId, requestArgs.destinationNodeId});
	            break;
	        }

	        case MSG_VEHICLE_ASSIGNMENT:
	        {
				const VehicleAssignmentMessage& replyArgs = MSG_CAST(VehicleAssignmentMessage, message);
				if (!replyArgs.success) {
					Print() << "Request received from " << replyArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
						<< replyArgs.currTick.frame() << " with startNodeId " << replyArgs.startNodeId << ", destinationNodeId "
						<< replyArgs.destinationNodeId << ", and taxiDriverId null" << std::endl;

					requestQueue.push_back({replyArgs.currTick, replyArgs.personId, replyArgs.startNodeId, replyArgs.destinationNodeId});
				} else {
					Print() << "Assignment response received from " << replyArgs.personId << " at time "
						<< currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame() << " with startNodeId "
						<< replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId << ", and taxiDriverId "
						<< replyArgs.taxiDriverId << std::endl;
				}
	        }

	        default:break;
	    };
}

bool VehicleController::isNonspatial()
{
	return true;
}
}
}







