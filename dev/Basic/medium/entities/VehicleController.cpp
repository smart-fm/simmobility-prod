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

bool VehicleController::RegisterVehicleController(int id, const MutexStrategy& mtxStrat)
{
	VehicleController *vehicleController = new VehicleController(id, mtxStrat);

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

// TODO: These functions should be abstract
void VehicleController::addTaxiDriver(Person_MT* person)
{
	VehicleController::taxiDrivers.push_back(person);
}

void VehicleController::removeTaxiDriver(Person_MT* person)
{
	taxiDrivers.erase(std::remove(taxiDrivers.begin(), taxiDrivers.end(), person), taxiDrivers.end());
}


Response VehicleController::assignVehicleToRequest(Request request) {
	Response response = {false};

	TaxiDriver* best_driver;
	double best_distance = -1;

	auto person = taxiDrivers.begin();

	while (person != taxiDrivers.end())
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

					TaxiDriverMovement* movement = best_driver->getMovementFacet();
					const Node* node = movement->getCurrentNode();

					best_distance = std::abs(request.startNode->getPosX() - node->getPosX());
					best_distance += std::abs(request.startNode->getPosY() - node->getPosY());
				}

				else
				{
					double curr_distance = 0.0;

					// TODO: Find shortest path instead
					TaxiDriverMovement* movement = curr_driver->getMovementFacet();
					const Node* node = movement->getCurrentNode();

					curr_distance = std::abs(request.startNode->getPosX() - node->getPosX());
					curr_distance += std::abs(request.startNode->getPosY() - node->getPosY());

					if (curr_distance < best_distance)
					{
						best_driver = curr_driver;
						best_distance = curr_distance;
					}
				}
			}
		}

		person++;
	}

	printf("Best distance: %f\n", best_distance);

	if (best_distance == -1) return response;

	// TODO: Call taxi function to take request
	response.success = true;

	return response;
}

Entity::UpdateStatus VehicleController::frame_tick(timeslice now)
{
	// TODO: See if keeping track of all taxi locations
	//       speeds up a lot of time
	printf("TESTING\n");
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus VehicleController::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void VehicleController::frame_output(timeslice now)
{

}

bool VehicleController::isNonspatial()
{
	return true;
}
}
}

