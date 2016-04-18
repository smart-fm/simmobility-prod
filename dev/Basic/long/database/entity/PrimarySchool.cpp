/*
 * PrimarySchool.cpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#include "PrimarySchool.hpp"

using namespace sim_mob::long_term;

PrimarySchool::PrimarySchool(BigSerial schoolId, BigSerial postcode , double centroidX, double centroidY, std::string schoolName, int giftedProgram, int sapProgram, std::string dgp, BigSerial tazId,int numStudents)
							:schoolId(schoolId),postcode(postcode), centroidX(centroidX),centroidY(centroidY), schoolName(schoolName), giftedProgram(giftedProgram), sapProgram(sapProgram),dgp(dgp), tazId(tazId),numStudents(numStudents){}

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
	this->numStudents = source.numStudents;

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
	this->numStudents = source.numStudents;

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

int PrimarySchool::getNumStudents() const
{
	return this->numStudents;
}

void PrimarySchool::addStudent(Individual *student)
{
	this->students.push_back(student);
	numStudents++;
}

void PrimarySchool::addIndividualDistance(DistanceIndividual &distanceIndividual)
{
	distanceIndList.push_back(distanceIndividual);
}

std::vector<PrimarySchool::DistanceIndividual>  PrimarySchool::getSortedDistanceIndList()
{
	std::sort(distanceIndList.begin(), distanceIndList.end(), PrimarySchool::OrderByDistance());
	return distanceIndList;
}

std::vector<BigSerial> PrimarySchool::getStudents()
{
	std::vector<BigSerial> studentIdList;
	for( Individual *student: students)
	{
		studentIdList.push_back(student->getId());
	}
	return studentIdList;
}

std::vector<BigSerial> PrimarySchool::getSelectedStudents()
{
	return this->selectedStudents;
}

void PrimarySchool::setSelectedStudentList(std::vector<BigSerial>selectedStudentsList)
{
	this->selectedStudents = selectedStudentsList;
}

void PrimarySchool::setNumStudentsCanBeAssigned(int numStudents)
{
	this->numStudentsCanBeAssigned = numStudents;
}

int PrimarySchool::getNumStudentsCanBeAssigned()
{
	return this->numStudentsCanBeAssigned;
}

void PrimarySchool::setReAllocationProb(double probability)
{
	this->reAllocationProb = probability;
}

double PrimarySchool::getReAllocationProb()
{
	return this->reAllocationProb;
}

void PrimarySchool::addSelectedStudent(BigSerial individualId)
{
	this->selectedStudents.push_back(individualId);
}
