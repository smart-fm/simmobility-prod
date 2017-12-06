//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "entities/roles/driver/OnCallDriver.hpp"
#include "ParkingAgent.hpp"

using namespace sim_mob;
using namespace medium;

std::unordered_map<const SMSVehicleParking *, ParkingAgent *> ParkingAgent::mapOfParkingAgents;

ParkingAgent::ParkingAgent(const MutexStrategy &mtxStrat, int id, const SMSVehicleParking *parking) :
		Agent(mtxStrat, id), smsVehicleParking(parking)
{
	setParentConflux();
}

ParkingAgent::~ParkingAgent()
{
}

void ParkingAgent::setParentConflux()
{
	parentConflux = Conflux::getConfluxFromNode(smsVehicleParking->getAccessNode());
	parentConflux->addParkingAgent(this);
}

void ParkingAgent::registerParkingAgent(ParkingAgent *pkAgent)
{
	const SMSVehicleParking *smsParking = pkAgent->getSMSParking();

	if(pkAgent && smsParking)
	{
		mapOfParkingAgents[smsParking] = pkAgent;
	}
}

ParkingAgent* ParkingAgent::getParkingAgent(const SMSVehicleParking *parking)
{
	auto it = mapOfParkingAgents.find(parking);

	if(it != mapOfParkingAgents.end())
	{
		return it->second;
	}

	return nullptr;
}

void ParkingAgent::addParkedPerson(Person_MT *person)
{
	parkedPersons.push_back(person);
}

Entity::UpdateStatus ParkingAgent::frame_init(timeslice now)
{
	if(!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus ParkingAgent::frame_tick(timeslice now)
{
	auto it = parkedPersons.begin();
	while(it != parkedPersons.end())
	{
		parentConflux->updateParkedServiceDriver(*it);

		OnCallDriver *onCallDriver = dynamic_cast<OnCallDriver *>((*it)->getRole());
		if(!onCallDriver || onCallDriver->isToBeRemovedFromParking())
		{
			it = parkedPersons.erase(it);
			continue;
		}

		++it;
	}

	return UpdateStatus::Continue;
}