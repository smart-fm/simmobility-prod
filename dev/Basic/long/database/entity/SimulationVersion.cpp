/*
 * SimulationVersion.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#include "database/entity/SimulationVersion.hpp"

using namespace sim_mob::long_term;

SimulationVersion::SimulationVersion(BigSerial id, std::string scenario, std::tm simulationStartDate,int simStoppedTick) : id(id), scenario(scenario),simulationStartDate(simulationStartDate), simStoppedTick(simStoppedTick){}

SimulationVersion::~SimulationVersion(){}

BigSerial SimulationVersion::getId() const
{
	return id;
}

std::string SimulationVersion::getScenario() const
{
	return scenario;
}

std::tm SimulationVersion::getSimulationStartDate() const
{
	return simulationStartDate;
}

int SimulationVersion::getSimStoppedTick() const
{
	return simStoppedTick;
}

void SimulationVersion::setId(BigSerial simVersionid)
{
	this->id = simVersionid;
}

void SimulationVersion::setScenario(std::string simScenario)
{
	this->scenario = simScenario;
}

void SimulationVersion::setSimulationStartDate(std::tm simStratDate)
{
	this->simulationStartDate = simStratDate;
}

void SimulationVersion::setSimStoppedTick(int simStoppedTick)
{
	this->simStoppedTick = simStoppedTick;
}

namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const SimulationVersion& data)
		{
			return strm << "{"
						<< "\"id \":\"" << data.id  << "\","
						<< "\"scenario \":\"" << data.scenario  << "\""
						<< "\"simulationStartDate \":\"" << data.simulationStartDate.tm_year << data.simulationStartDate.tm_mon<<data.simulationStartDate.tm_mday<<"\""
						<< "\"simStoppedTick \":\"" << data.simStoppedTick  << "\""
						<< "}";
		}
	}
}


