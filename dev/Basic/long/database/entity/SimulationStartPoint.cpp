/*
 * SimulationVersion.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: gishara
 */

#include "database/entity/SimulationStartPoint.hpp"

using namespace sim_mob::long_term;

SimulationStartPoint::SimulationStartPoint(BigSerial id, std::string scenario, std::tm simulationStartDate,std::string mainSchemaVersion,std::string configSchemaVersion,std::string calibrationSchemaVersion,std::string geometrySchemaVersion, int simStoppedTick) : id(id), scenario(scenario),simulationStartDate(simulationStartDate),
		mainSchemaVersion(mainSchemaVersion), configSchemaVersion(configSchemaVersion),calibrationSchemaVersion(calibrationSchemaVersion),geometrySchemaVersion(geometrySchemaVersion),simStoppedTick(simStoppedTick){}

SimulationStartPoint::~SimulationStartPoint(){}

BigSerial SimulationStartPoint::getId() const
{
	return id;
}

std::string SimulationStartPoint::getScenario() const
{
	return scenario;
}

std::tm SimulationStartPoint::getSimulationStartDate() const
{
	return simulationStartDate;
}

int SimulationStartPoint::getSimStoppedTick() const
{
	return simStoppedTick;
}

void SimulationStartPoint::setId(BigSerial simVersionid)
{
	this->id = simVersionid;
}

void SimulationStartPoint::setScenario(std::string simScenario)
{
	this->scenario = simScenario;
}

void SimulationStartPoint::setSimulationStartDate(std::tm simStratDate)
{
	this->simulationStartDate = simStratDate;
}

void SimulationStartPoint::setSimStoppedTick(int simStoppedTick)
{
	this->simStoppedTick = simStoppedTick;
}

const std::string& SimulationStartPoint::getCalibrationSchemaVersion() const
{
	return calibrationSchemaVersion;
}

void SimulationStartPoint::setCalibrationSchemaVersion(const std::string& calibSchemaVersion)
{
	this->calibrationSchemaVersion = calibrationSchemaVersion;
}

const std::string& SimulationStartPoint::getConfigSchemaVersion() const
{
	return configSchemaVersion;
}

void SimulationStartPoint::setConfigSchemaVersion(const std::string& cfgSchemaVersion)
{
	this->configSchemaVersion = cfgSchemaVersion;
}

const std::string& SimulationStartPoint::getGeometrySchemaVersion() const
{
	return geometrySchemaVersion;
}

void SimulationStartPoint::setGeometrySchemaVersion(const std::string& geomtrySchemaVersion)
{
	this->geometrySchemaVersion = geomtrySchemaVersion;
}

const std::string& SimulationStartPoint::getMainSchemaVersion() const
{
	return mainSchemaVersion;
}

void SimulationStartPoint::setMainSchemaVersion(const std::string& mainSchemaVersion)
{
	this->mainSchemaVersion = mainSchemaVersion;
}

namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const SimulationStartPoint& data)
		{
			return strm << "{"
						<< "\"id \":\"" << data.id  << "\","
						<< "\"scenario \":\"" << data.scenario  << "\""
						<< "\"simulationStartDate \":\"" << data.simulationStartDate.tm_year << data.simulationStartDate.tm_mon<<data.simulationStartDate.tm_mday<<"\""
						<< "\"mainSchemaVersion \":\"" << data.mainSchemaVersion  << "\""
						<< "\"geometrySchemaVersion \":\"" << data.geometrySchemaVersion  << "\""
						<< "\"configSchemaVersion \":\"" << data.configSchemaVersion  << "\""
						<< "\"calibrationSchemaVersion \":\"" << data.calibrationSchemaVersion  << "\""
						<< "\"simStoppedTick \":\"" << data.simStoppedTick  << "\""
						<< "}";
		}
	}
}


