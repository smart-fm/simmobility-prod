/*
 * StudyArea.cpp
 *
 *  Created on: 6 Jun 2017
 *      Author: gishara
 */

#include "StudyArea.hpp"

using namespace sim_mob::long_term;

StudyArea::StudyArea(BigSerial id,  BigSerial fmTazId, std::string studyCode):
		id(id),fmTazId(fmTazId),studyCode(studyCode) {}

StudyArea::~StudyArea() {}

BigSerial StudyArea::getId() const
{
	return id;
}

void StudyArea::setId(BigSerial id)
{
	this->id = id;
}

const std::string& StudyArea::getStudyCode() const
{
	return studyCode;
}

void StudyArea::setStudyCode(const std::string& studyCode)
{
	this->studyCode = studyCode;
}

BigSerial StudyArea::getFmTazId() const
{
	return fmTazId;
}

void StudyArea::setFmTazId(BigSerial fmTazId)
{
	this->fmTazId = fmTazId;
}
