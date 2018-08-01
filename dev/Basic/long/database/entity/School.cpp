/*
 * School.cpp
 *
 *  Created on: 7 Nov 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "School.hpp"

using namespace sim_mob::long_term;

School::School(BigSerial id, BigSerial fmBuildingId ,double floorArea, double schoolSlot, double centroidX, double centroidY, bool giftedProgram, bool sapProgram,
			   std::string planningArea, BigSerial tazName, std::string schoolType, bool artProgram, bool musicProgram, bool langProgram, bool expressTest, double studentDensity, int numStudents) : id(id),fmBuildingId(fmBuildingId), floorArea(floorArea), schoolSlot(schoolSlot),centroidX(centroidX),centroidY(centroidY),
			   giftedProgram(giftedProgram), sapProgram(sapProgram), planningArea(planningArea),tazName(tazName),schoolType(schoolType),artProgram(artProgram),musicProgram(musicProgram),langProgram(langProgram),expressTest(expressTest), studentDensity(studentDensity), numStudents(numStudents){}

School::~School(){}

double School::getCentroidX() const
{
	return centroidX;
}

void School::setCentroidX(double centroidX)
{
	this->centroidX = centroidX;
}

double School::getCentroidY() const
{
	return centroidY;
}

void School::setCentroidY(double centroidY)
{
	this->centroidY = centroidY;
}

double School::getFloorArea() const
{
	return floorArea;
}

void School::setFloorArea(double floorArea)
{
	this->floorArea = floorArea;
}

BigSerial School::getFmBuildingId() const
{
	return fmBuildingId;
}

void School::setFmBuildingId(BigSerial fmBuildingId)
{
	this->fmBuildingId = fmBuildingId;
}

bool School::isGiftedProgram() const
{
	return giftedProgram;
}

void School::setGiftedProgram(bool giftedProgram)
{
	this->giftedProgram = giftedProgram;
}

BigSerial School::getId() const
{
	return id;
}

void School::setId(BigSerial id)
{
	this->id = id;
}

std::string School::getPlanningArea() const
{
	return planningArea;
}

void School::setPlanningArea(std::string planningArea)
{
	this->planningArea = planningArea;
}

bool School::isSapProgram() const
{
	return sapProgram;
}

void School::setSapProgram(bool sapProgram)
{
	this->sapProgram = sapProgram;
}

double School::getSchoolSlot() const
{
	return schoolSlot;
}

void School::setSchoolSlot(double schoolSlot)
{
	this->schoolSlot = schoolSlot;
}

BigSerial School::getTazName() const
{
	return tazName;
}

void School::setTazName(BigSerial tazName)
{
	this->tazName = tazName;
}

std::string School::getSchoolType() const
{
	return schoolType;
}

void School::setSchoolType(std::string schoolType)
{
	this->schoolType = schoolType;
}

int School::getNumStudents() const
{
	return this->numStudents;
}

void School::addStudent(BigSerial studentId)
{
	this->students.push_back(studentId);
	numStudents++;
}

void School::addIndividualDistance(DistanceIndividual &distanceIndividual)
{
	distanceIndList.push_back(distanceIndividual);
}

std::vector<School::DistanceIndividual>  School::getSortedDistanceIndList()
{
	std::sort(distanceIndList.begin(), distanceIndList.end(), School::OrderByDistance());
	return distanceIndList;
}

std::vector<School*> getSortedProbSchoolList( std::vector<School*> studentsWithProb)
{
	std::sort(studentsWithProb.begin(), studentsWithProb.end(), School::OrderByProbability());
	return studentsWithProb;
}

std::vector<BigSerial> School::getStudents()
{
	return this->students;
}

std::vector<BigSerial> School::getSelectedStudents()
{
	return this->selectedStudents;
}

int School::getNumSelectedStudents()
{
	return this->selectedStudents.size();
}

void School::setSelectedStudentList(std::vector<BigSerial>selectedStudents)
{
	this->selectedStudents = selectedStudents;
}

void School::setNumStudentsCanBeAssigned(int numStudents)
{
	this->numStudentsCanBeAssigned = numStudents;
}

int School::getNumStudentsCanBeAssigned()
{
	return this->numStudentsCanBeAssigned;
}

void School::setReAllocationProb(double probability)
{
	this->reAllocationProb = probability;
}

double School::getReAllocationProb()
{
	return this->reAllocationProb;
}

void School::addSelectedStudent(BigSerial individualId)
{
	this->selectedStudents.push_back(individualId);
}

int  School::getNumOfSelectedStudents()
{
	return this->selectedStudents.size();
}

bool School::isArtProgram() const
{
	return artProgram;
}

void School::setArtProgram(bool artProgram)
{
	this->artProgram = artProgram;
}

bool School::isLangProgram() const
{
	return langProgram;
}

void School::setLangProgram(bool langProgram)
{
	this->langProgram = langProgram;
}

bool School::isMusicProgram() const
{
	return musicProgram;
}

void School::setMusicProgram(bool musicProgram)
{
	this->musicProgram = musicProgram;
}

bool School::isExpressTest() const
{
	return expressTest;
}

void School::setExpressTest(bool expressTest)
{
	this->expressTest = expressTest;
}

double School::getStudentDensity() const
{
	return studentDensity;
}

void School::setStudentDensity(double studentDensity)
{
	this->studentDensity = studentDensity;
}
