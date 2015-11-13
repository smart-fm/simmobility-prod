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
#include "StatusOfWorld.hpp"

using namespace sim_mob::long_term;

StatusOfWorld::StatusOfWorld( BigSerial simVersionId,BigSerial postcode, BigSerial buildingId, BigSerial unitId, BigSerial projectId):
				simVersionId(simVersionId),postcode(postcode),buildingId(buildingId),unitId(unitId),projectId(projectId){}


StatusOfWorld::~StatusOfWorld() {}

BigSerial StatusOfWorld::getPostcode() const
{
	return this->postcode;
}

BigSerial StatusOfWorld::getBuildingId() const
{
	return this->buildingId;
}

BigSerial StatusOfWorld::getUnitId() const
{
	return this->unitId;
}

BigSerial StatusOfWorld::getProjectId() const
{
	return this->projectId;
}

BigSerial StatusOfWorld::getSimVersionId() const
{
		return simVersionId;
}

void StatusOfWorld::setPostcode(BigSerial postcode)
{
	this->postcode = postcode;
}

void StatusOfWorld::setBuildingId(BigSerial buildingId)
{
	this->buildingId = buildingId;
}

void StatusOfWorld::setUnitId(BigSerial unitId)
{
	this->unitId = unitId;
}

void StatusOfWorld::setProjectId(BigSerial projectId)
{
	this->projectId = projectId;
}

void StatusOfWorld::setSimVersionId(BigSerial simVersionId)
{
		this->simVersionId = simVersionId;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const StatusOfWorld& data) {
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







