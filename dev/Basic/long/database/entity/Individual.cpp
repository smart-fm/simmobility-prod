//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Individual.cpp
 *
 *  Created on: 3 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Individual.hpp>

using namespace sim_mob::long_term;

Individual::Individual( BigSerial id, BigSerial individualTypeId, BigSerial householdId, BigSerial jobId, BigSerial ethnicityId, BigSerial employmentStatusId,
						BigSerial genderId, BigSerial educationId, BigSerial occupationId, BigSerial vehiculeCategoryId, BigSerial transitCategoryId,
						BigSerial ageCategoryId, BigSerial residentialStatusId, bool householdHead, float income, int memberId, bool workerAtHome, bool driversLicence,	std::tm dateOfBirth) :
						id(id), individualTypeId(individualTypeId), householdId(householdId), jobId(jobId), ethnicityId(ethnicityId), employmentStatusId(employmentStatusId),
						genderId(genderId), educationId(educationId), occupationId(occupationId), vehiculeCategoryId(vehiculeCategoryId), transitCategoryId(transitCategoryId),
						ageCategoryId(ageCategoryId), residentialStatusId(residentialStatusId), householdHead(householdHead), income(income), memberId(memberId), workAtHome(workAtHome),
						driversLicence(driversLicence), dateOfBirth(dateOfBirth) {}

Individual::~Individual() {}

Individual& Individual::operator=(const Individual& source)
{
	this->id = source.id;
	this->individualTypeId = source.individualTypeId;
	this->householdId = source.householdId;
	this->jobId = source.jobId;
	this->ethnicityId = source.ethnicityId;
	this->employmentStatusId = source.employmentStatusId;
	this->genderId = source.genderId;
	this->educationId = source.educationId;
	this->occupationId = source.occupationId;
	this->vehiculeCategoryId = source.vehiculeCategoryId;
	this->transitCategoryId = source.transitCategoryId;
	this->ageCategoryId = source.ageCategoryId;
	this->residentialStatusId = source.residentialStatusId;
	this->householdHead = source.householdHead;
	this->income = source.income;
	this->memberId = source.memberId;
	this->workAtHome = source.workAtHome;
	this->driversLicence = source.driversLicence;
	this->dateOfBirth = source.dateOfBirth;

	return *this;
}

BigSerial Individual::getId() const
{
	return id;
}

BigSerial Individual::getIndividualTypeId() const
{
	return individualTypeId;
}

BigSerial Individual::getHouseholdId() const
{
	return householdId;
}

BigSerial Individual::getJobId() const
{
	return jobId;
}

BigSerial Individual::getEthnicityId() const
{
	return ethnicityId;
}

BigSerial Individual::getEmploymentStatusId() const
{
	return employmentStatusId;
}

BigSerial Individual::getGenderId() const
{
	return genderId;
}

BigSerial Individual::getEducationId() const
{
	return educationId;
}

BigSerial Individual::getOccupationId() const
{
	return occupationId;
}

BigSerial Individual::getVehiculeCategoryId() const
{
	return vehiculeCategoryId;
}

BigSerial Individual::getTransitCategoryId() const
{
	return transitCategoryId;
}

BigSerial Individual::getAgeCategoryId() const
{
	return ageCategoryId;
}

BigSerial Individual::getResidentialStatusId() const
{
	return residentialStatusId;
}

bool Individual::getHouseholdHead() const
{
	return householdHead;
}

float Individual::getIncome() const
{
	return income;
}

int Individual::getMemberId() const
{
	return memberId;
}

bool Individual::getWorkAtHome() const
{
	return workAtHome;
}

bool Individual::getDriversLicence() const
{
	return driversLicence;
}

std::tm Individual::getDateOfBirth() const
{
	return dateOfBirth;
}

namespace sim_mob {
namespace long_term {
std::ostream& operator<<(std::ostream& strm, const Individual& data) {
	return strm << "{" << "\"id \":\"" << data.id << "\","
			<< "\"individualTypeId \":\"" << data.individualTypeId << "\","
			<< "\"householdId \":\"" << data.householdId << "\","
			<< "\"jobId \":\"" << data.jobId << "\"," << "\"ethnicityId \":\""
			<< data.ethnicityId << "\"," << "\"employmentStatusId \":\""
			<< data.employmentStatusId << "\"," << "\"genderId \":\""
			<< data.genderId << "\"," << "\"educationId \":\""
			<< data.educationId << "\"," << "\"occupationId \":\""
			<< data.occupationId << "\"," << "\"vehiculeCategoryId \":\""
			<< data.vehiculeCategoryId << "\"," << "\"transitCategoryId \":\""
			<< data.transitCategoryId << "\"," << "\"ageCategoryId \":\""
			<< data.ageCategoryId << "\"," << "\"residentialStatusId \":\""
			<< data.residentialStatusId << "\"," << "\"householdHead \":\""
			<< data.householdHead << "\"," << "\"income \":\"" << data.income
			<< "\"," << "\"memberId \":\"" << data.memberId << "\","
			<< "\"workerAtHome \":\"" << data.workAtHome << "\","
			<< "\"driversLicence \":\"" << data.driversLicence << "\","
			<< "\"dateOfBirth \":\"" << data.dateOfBirth.tm_year << " " << data.dateOfBirth.tm_mon << " " << data.dateOfBirth.tm_mday << "\"" << "}";

}
}
}

