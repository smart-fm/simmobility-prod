/*
 * ROILimits.cpp
 *
 *  Created on: 17 May 2016
 *      Author: gishara
 */

#include "ROILimits.hpp"

using namespace sim_mob::long_term;

ROILimits::ROILimits(BigSerial developmentTypeId, double roiLimit):developmentTypeId(developmentTypeId),roiLimit(roiLimit){}

ROILimits::~ROILimits(){}

ROILimits::ROILimits(const ROILimits& source)
{
	this->developmentTypeId = source.developmentTypeId;
	this->roiLimit = source.roiLimit;

}

ROILimits& ROILimits::operator=(const ROILimits& source)
{
	this->developmentTypeId = source.developmentTypeId;
	this->roiLimit = source.roiLimit;

	return *this;
}

BigSerial ROILimits::getDevelopmentTypeId() const
{
		return developmentTypeId;
}

double ROILimits::getRoiLimit() const
{
		return roiLimit;
}

void ROILimits::setDevelopmentTypeId(BigSerial developmentTypeId)
{
		this->developmentTypeId = developmentTypeId;
}

void ROILimits::setRoiLimit(double roiLimit)
{
		this->roiLimit = roiLimit;
}

namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const ROILimits& data)
		{
			return strm << "{"
					<< "\"developmentTypeId\":\"" << data.developmentTypeId << "\","
					<< "\"roiLimit\":\"" << data.roiLimit << "\""
					<< "}";
		}
	}
}







