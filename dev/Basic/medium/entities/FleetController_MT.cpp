//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "FleetController_MT.hpp"

#include "conf/ConfigManager.hpp"
#include "entities/misc/TaxiTrip.hpp"
#include "entities/Person_MT.hpp"

using namespace std;
using namespace sim_mob;
using namespace medium;

FleetController_MT* FleetController_MT::fleetMgr = nullptr;

FleetController_MT::FleetController_MT() : FleetController()
{
}

FleetController_MT::~FleetController_MT()
{
}

FleetController_MT * FleetController_MT::getInstance()
{
	if(!fleetMgr)
	{
		fleetMgr = new FleetController_MT();
	}

	return fleetMgr;
}

void FleetController_MT::initialise(std::set<sim_mob::Entity *> &agentList)
{
	const unsigned int maxFleetSize = ConfigManager::GetInstance().FullConfig().mobilityServiceController.maxFleetSize;
	unsigned int currTaxi = 0, vehToBeLoaded = min(maxFleetSize, (unsigned int)taxiFleet.size());
	auto serviceVehicle = taxiFleet.begin();

	ControllerLog() << "Total number of service vehicles loaded from database: " << taxiFleet.size() << std::endl;
	ControllerLog() << "Max. fleet size configured: " << maxFleetSize << std::endl;

	while(currTaxi < vehToBeLoaded && serviceVehicle != taxiFleet.end())
	{
		if ((*serviceVehicle).startNode)
		{
			Person_MT* person = new Person_MT("FleetController", ConfigManager::GetInstance().FullConfig().mutexStategy(), -1);
			person->setServiceVehicle((*serviceVehicle));
			person->setDatabaseId((*serviceVehicle).driverId);
			person->setPersonCharacteristics();

			string tripType;

			switch((*serviceVehicle).controllerSubscription)
			{
			case SERVICE_CONTROLLER_ON_HAIL:
				tripType = "OnHailTrip";
				break;
			default:
				tripType = "TaxiTrip";
			}

			vector<TripChainItem*> tripChain;
			TaxiTrip *taxiTrip = new TaxiTrip("0", tripType, 0, -1, DailyTime((*serviceVehicle).startTime * 1000.0),
			                                  DailyTime((*serviceVehicle).endTime * 1000), 0, (*serviceVehicle).startNode,
			                                  "node", nullptr, "node");
			tripChain.push_back((TripChainItem *)taxiTrip);
			person->setTripChain(tripChain);

			addOrStashTaxis(person, agentList);

			//Valid vehicle loaded
			currTaxi++;
		}
		else
		{
			Warn() << "Vehicle " << (*serviceVehicle).vehicleNo << ", with driver " << (*serviceVehicle).driverId
			       << " has invalid start node.";
		}

		serviceVehicle++;
	}
}

void FleetController_MT::addOrStashTaxis(Person *person, std::set<Entity *> &activeAgents)
{
	if (person->getStartTime() == 0)
	{
		//Only agents with a start time of zero should start immediately in the all_agents list.
		activeAgents.insert((Entity *)person);
	}
	else
	{
		//Start later.
		pendingChildren.push((Entity *)person);
	}
}
