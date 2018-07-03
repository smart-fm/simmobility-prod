/*
 * PreSchool.cpp
 *
 *  Created on: 15 Mar 2016
 *      Author: gishara
 */

#include "PreSchool.hpp"

using namespace sim_mob::long_term;

PreSchool::PreSchool(BigSerial preSchoolId, std::string preSchoolName  , BigSerial postcode ,std::string preSchoolDistrict,double centroidX, double centroidY)
							:preSchoolId(preSchoolId),preSchoolName(preSchoolName), postcode(postcode), preSchoolDistrict(preSchoolDistrict),centroidX(centroidX),centroidY(centroidY) {}

PreSchool::~PreSchool(){}

double PreSchool::getCentroidX() const
{
	return centroidX;
}

void PreSchool::setCentroidX(double centroidX)
{
	this->centroidX = centroidX;
}

double PreSchool::getCentroidY() const
{
	return centroidY;
}

void PreSchool::setCentroidY(double centroidY)
{
	this->centroidY = centroidY;
}

const std::string& PreSchool::getPreSchoolDistrict() const
{
	return this->preSchoolDistrict;
}

void PreSchool::setPreSchoolDistrict(const std::string& dgp)
{
	this->preSchoolDistrict = dgp;
}

BigSerial PreSchool::getPostcode() const
{
	return postcode;
}

void PreSchool::setPostcode(BigSerial postcode)
{
	this->postcode = postcode;
}

BigSerial PreSchool::getPreSchoolId() const
{
	return preSchoolId;
}

void PreSchool::setPreSchoolId(BigSerial schoolId)
{
	this->preSchoolId = schoolId;
}

const std::string& PreSchool::getPreSchoolName() const
{
	return preSchoolName;
}

void PreSchool::setPreSchoolName(const std::string& schoolName)
{
	this->preSchoolName = schoolName;
}
