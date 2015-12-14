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
#include "EncodedParamsBySimulation.hpp"

using namespace sim_mob::long_term;

EncodedParamsBySimulation::EncodedParamsBySimulation( BigSerial simVersionId,BigSerial postcode, BigSerial buildingId, BigSerial unitId, BigSerial projectId):
				simVersionId(simVersionId),postcode(postcode),buildingId(buildingId),unitId(unitId),projectId(projectId){}


EncodedParamsBySimulation::~EncodedParamsBySimulation() {}

BigSerial EncodedParamsBySimulation::getPostcode() const
{
	return this->postcode;
}

BigSerial EncodedParamsBySimulation::getBuildingId() const
{
	return this->buildingId;
}

BigSerial EncodedParamsBySimulation::getUnitId() const
{
	return this->unitId;
}

BigSerial EncodedParamsBySimulation::getProjectId() const
{
	return this->projectId;
}

BigSerial EncodedParamsBySimulation::getSimVersionId() const
{
		return simVersionId;
}

void EncodedParamsBySimulation::setPostcode(BigSerial postcode)
{
	this->postcode = postcode;
}

void EncodedParamsBySimulation::setBuildingId(BigSerial buildingId)
{
	this->buildingId = buildingId;
}

void EncodedParamsBySimulation::setUnitId(BigSerial unitId)
{
	this->unitId = unitId;
}

void EncodedParamsBySimulation::setProjectId(BigSerial projectId)
{
	this->projectId = projectId;
}

void EncodedParamsBySimulation::setSimVersionId(BigSerial simVersionId)
{
		this->simVersionId = simVersionId;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const EncodedParamsBySimulation& data) {
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







