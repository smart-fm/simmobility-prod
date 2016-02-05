/*
 * StatusOfWorld.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: gishara
 */
/*
 * SlaParcel.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gishara
 */
#include "SimulationStoppedPoint.hpp"

using namespace sim_mob::long_term;

SimulationStoppedPoint::SimulationStoppedPoint( BigSerial simVersionId,BigSerial postcode, BigSerial buildingId, BigSerial unitId, BigSerial projectId):
				simVersionId(simVersionId),postcode(postcode),buildingId(buildingId),unitId(unitId),projectId(projectId){}


SimulationStoppedPoint::~SimulationStoppedPoint() {}

BigSerial SimulationStoppedPoint::getPostcode() const
{
	return this->postcode;
}

BigSerial SimulationStoppedPoint::getBuildingId() const
{
	return this->buildingId;
}

BigSerial SimulationStoppedPoint::getUnitId() const
{
	return this->unitId;
}

BigSerial SimulationStoppedPoint::getProjectId() const
{
	return this->projectId;
}

BigSerial SimulationStoppedPoint::getSimVersionId() const
{
		return simVersionId;
}

void SimulationStoppedPoint::setPostcode(BigSerial postcode)
{
	this->postcode = postcode;
}

void SimulationStoppedPoint::setBuildingId(BigSerial buildingId)
{
	this->buildingId = buildingId;
}

void SimulationStoppedPoint::setUnitId(BigSerial unitId)
{
	this->unitId = unitId;
}

void SimulationStoppedPoint::setProjectId(BigSerial projectId)
{
	this->projectId = projectId;
}

void SimulationStoppedPoint::setSimVersionId(BigSerial simVersionId)
{
		this->simVersionId = simVersionId;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const SimulationStoppedPoint& data) {
            return strm << "{"
            			<< "\"simVersionId\":\"" << data.simVersionId << "\","
						<< "\"postcode\":\"" << data.postcode << "\","
						<< "\"buildingId\":\"" << data.buildingId << "\","
						<< "\"unitId\":\"" << data.unitId << "\","
						<< "\"projectId\":\"" << data.projectId << "\","
						<< "}";
        }
    }
}







