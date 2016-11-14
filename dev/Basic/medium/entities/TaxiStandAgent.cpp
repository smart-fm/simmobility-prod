/*
 * TaxiStandAgent.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: fm-simmobility
 */

#include "entities/TaxiStandAgent.hpp"
#include "message/MessageBus.hpp"

namespace sim_mob
{
std::map<const TaxiStand*, TaxiStandAgent*> TaxiStandAgent::allTaxiStandAgents;

TaxiStandAgent::TaxiStandAgent(const MutexStrategy& mtxStrat, int id, const TaxiStand* stand):taxiStand(stand),Agent(mtxStrat, id)
{

}

TaxiStandAgent::~TaxiStandAgent()
{

}

Entity::UpdateStatus TaxiStandAgent::frame_init(timeslice now)
{
	if(!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus TaxiStandAgent::frame_tick(timeslice now)
{
	return Entity::UpdateStatus::Continue;
}

void TaxiStandAgent::frame_output(timeslice now)
{

}

bool TaxiStandAgent::isNonspatial()
{
	return false;
}

void TaxiStandAgent::setTaxiStand(const TaxiStand* stand)
{
	taxiStand = stand;
}

const TaxiStand* TaxiStandAgent::getTaxiStand() const
{
	return taxiStand;
}

void TaxiStandAgent::addWaitingPerson(medium::Person_MT* person)
{
	waitingPeople.push(person);
}

void TaxiStandAgent::registerTaxiStandAgent(TaxiStandAgent* agent)
{
	if(agent && agent->getTaxiStand() )
	{
		allTaxiStandAgents[agent->getTaxiStand()] = agent;
	}
}

}
