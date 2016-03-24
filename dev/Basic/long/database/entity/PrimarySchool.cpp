/*
 * PrimarySchool.cpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#include "PrimarySchool.hpp"

using namespace sim_mob::long_term;

PrimarySchool::PrimarySchool(BigSerial schoolId, BigSerial postcode , double centroidX, double centroidY, std::string schoolName, int giftedProgram, int sapProgram, std::string dgp, BigSerial tazId)
							:schoolId(schoolId),postcode(postcode), centroidX(centroidX),centroidY(centroidY), schoolName(schoolName), giftedProgram(giftedProgram), sapProgram(sapProgram),dgp(dgp), tazId(tazId) {}

PrimarySchool::~PrimarySchool(){}

PrimarySchool::PrimarySchool(const PrimarySchool& source)
{
	this->schoolId = source.schoolId;
	this->postcode = source.postcode;
	this->centroidX = source.centroidX;
	this->centroidY = source.centroidY;
	this->schoolName = source.schoolName;
	this->giftedProgram = source.giftedProgram;
	this->sapProgram = source.sapProgram;
	this->dgp = source.dgp;
	this->tazId = source.tazId;

}

PrimarySchool& PrimarySchool::operator=(const PrimarySchool& source)
{
	this->schoolId = source.schoolId;
	this->postcode = source.postcode;
	this->centroidX = source.centroidX;
	this->centroidY = source.centroidY;
	this->schoolName = source.schoolName;
	this->giftedProgram = source.giftedProgram;
	this->sapProgram = source.sapProgram;
	this->dgp = source.dgp;
	this->tazId = source.tazId;

	return *this;
}

double PrimarySchool::getCentroidX() const
{
	return centroidX;
}

void PrimarySchool::setCentroidX(double centroidX)
{
	this->centroidX = centroidX;
}

double PrimarySchool::getCentroidY() const
{
	return centroidY;
}

void PrimarySchool::setCentroidY(double centroidY)
{
	this->centroidY = centroidY;
}

const std::string& PrimarySchool::getDgp() const
{
	return dgp;
}

void PrimarySchool::setDgp(const std::string& dgp)
{
	this->dgp = dgp;
}

int PrimarySchool::isGiftedProgram() const
{
	return giftedProgram;
}

void PrimarySchool::setGiftedProgram(int giftedProgram)
{
	this->giftedProgram = giftedProgram;
}

BigSerial PrimarySchool::getPostcode() const
{
	return postcode;
}

void PrimarySchool::setPostcode(BigSerial postcode)
{
	this->postcode = postcode;
}

int PrimarySchool::isSapProgram() const
{
	return sapProgram;
}

void PrimarySchool::setSapProgram(int sapProgram)
{
	this->sapProgram = sapProgram;
}

BigSerial PrimarySchool::getSchoolId() const
{
	return schoolId;
}

void PrimarySchool::setSchoolId(BigSerial schoolId)
{
	this->schoolId = schoolId;
}

const std::string& PrimarySchool::getSchoolName() const
{
	return schoolName;
}

void PrimarySchool::setSchoolName(const std::string& schoolName)
{
	this->schoolName = schoolName;
}

BigSerial PrimarySchool::getTazId() const
{
	return tazId;
}

void PrimarySchool::setTazId(BigSerial tazId)
{
	this->tazId = tazId;
}

