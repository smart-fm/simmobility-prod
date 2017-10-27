/*
 * JobsWithIndustryTypeAndTazId.cpp
 *
 *  Created on: 8 Sep 2017
 *      Author: gishara
 */

#include <database/entity/JobsWithIndustryTypeAndTazId.hpp>

using namespace sim_mob::long_term;

JobsWithIndustryTypeAndTazId::JobsWithIndustryTypeAndTazId(BigSerial jobId, int industryTypeId, BigSerial tazId):jobId(jobId), industryTypeId(industryTypeId),tazId(tazId){}

JobsWithIndustryTypeAndTazId::~JobsWithIndustryTypeAndTazId() {}

JobsWithIndustryTypeAndTazId::JobsWithIndustryTypeAndTazId( const JobsWithIndustryTypeAndTazId& source)
{
	this->jobId = source.jobId;
	this->industryTypeId = source.industryTypeId;
	this->tazId = source.tazId;

}

JobsWithIndustryTypeAndTazId& JobsWithIndustryTypeAndTazId::operator=( const JobsWithIndustryTypeAndTazId& source)
{
	this->jobId = source.jobId;
	this->industryTypeId = source.industryTypeId;
	this->tazId = source.tazId;

	return *this;
}

int JobsWithIndustryTypeAndTazId::getIndustryTypeId() const
{
	return industryTypeId;
}

void JobsWithIndustryTypeAndTazId::setIndustryTypeId(int industryTypeId)
{
	this->industryTypeId = industryTypeId;
}

BigSerial JobsWithIndustryTypeAndTazId::getJobId() const
{
	return jobId;
}

void JobsWithIndustryTypeAndTazId::setJobId(BigSerial jobId)
{
	this->jobId = jobId;
}

BigSerial JobsWithIndustryTypeAndTazId::getTazId() const
{
	return tazId;
}

void JobsWithIndustryTypeAndTazId::setTazId(BigSerial tazId)
{
	this->tazId = tazId;
}
