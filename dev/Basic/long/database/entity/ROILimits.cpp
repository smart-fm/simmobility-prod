/*
 * ROILimits.cpp
 *
 *  Created on: 17 May 2016
 *      Author: gishara
 */

#include "ROILimits.hpp"

using namespace sim_mob::long_term;

ROILimits::ROILimits(BigSerial buildingTypeId, double roiLimit):buildingTypeId(buildingTypeId),roiLimit(roiLimit){}

ROILimits::~ROILimits(){}

ROILimits::ROILimits(const ROILimits& source)
{
	this->buildingTypeId = source.buildingTypeId;
	this->roiLimit = source.roiLimit;

}

ROILimits& ROILimits::operator=(const ROILimits& source)
{
	this->buildingTypeId = source.buildingTypeId;
	this->roiLimit = source.roiLimit;

	return *this;
}

BigSerial ROILimits::getBuildingTypeId() const
{
		return buildingTypeId;
}

double ROILimits::getRoiLimit() const
{
		return roiLimit;
}

void ROILimits::setBuildingTypeId(BigSerial developmentTypeId)
{
		this->buildingTypeId = developmentTypeId;
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
					<< "\"developmentTypeId\":\"" << data.buildingTypeId << "\","
					<< "\"roiLimit\":\"" << data.roiLimit << "\""
					<< "}";
		}
	}
}







