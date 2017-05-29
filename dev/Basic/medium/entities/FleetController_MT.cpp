//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <conf/ConfigManager.hpp>
#include "FleetController_MT.hpp"
#include "entities/FleetController.hpp"
#include "entities/misc/TaxiTrip.hpp"
#include "entities/Person_MT.hpp"
#include "logging/ControllerLog.hpp"

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
	std::map<std::string, FleetController::FleetTimePriorityQueue> fleetMap;

	for (auto i = taxiFleet.begin(); i != taxiFleet.end(); i++)
	{
		const FleetController::FleetItem &taxi = (*i);
		fleetMap[taxi.vehicleNo].push(taxi);
	}

	int currTaxi = 0;
	// 47, 94, 141, 188, 235
	const int numberOfTaxis = 47;

	ControllerLog() << "Maximum number of taxis " << fleetMap.size() << std::endl;
	ControllerLog() << "Number of taxis " << numberOfTaxis << std::endl;

	for (auto i=fleetMap.begin(); i!= fleetMap.end(); i++)
	{
		if (currTaxi < numberOfTaxis) {
			const FleetController::FleetTimePriorityQueue& fleetItems = i->second;
			const FleetController::FleetItem& taxi = fleetItems.top();

			Person_MT* person = new Person_MT("TaxiController", ConfigManager::GetInstance().FullConfig().mutexStategy(), -1);
			person->setTaxiFleet(fleetItems);
			person->setDatabaseId(taxi.driverId);
			person->setPersonCharacteristics();

			vector<TripChainItem*> tripChain;

			if (taxi.startNode)
			{
				TaxiTrip *taxiTrip = new TaxiTrip("0", "TaxiTrip", 0, -1, DailyTime(taxi.startTime * 1000.0), DailyTime(),
												  0, const_cast<Node*>(taxi.startNode), "node", nullptr, "node");
				tripChain.push_back((TripChainItem *)taxiTrip);
				person->setTripChain(tripChain);

				addOrStashTaxis(person, agentList);
			}

			currTaxi++;
		}
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
