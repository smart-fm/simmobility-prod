/*
 * StudentStop.cpp
 *
 *  Created on: 13 Mar 2018
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "StudentStop.hpp"

using namespace sim_mob::long_term;

StudentStop::StudentStop(BigSerial homeStopEzLinkId, BigSerial schoolStopEzLinkId): homeStopEzLinkId(homeStopEzLinkId),schoolStopEzLinkId(schoolStopEzLinkId){}

StudentStop::~StudentStop(){}

StudentStop::StudentStop(const StudentStop& source)
{
    this->homeStopEzLinkId = source.homeStopEzLinkId;
    this->schoolStopEzLinkId = source.schoolStopEzLinkId;
}


StudentStop& StudentStop::operator=(const StudentStop& source)
{
    this->homeStopEzLinkId = source.homeStopEzLinkId;
    this->schoolStopEzLinkId = source.schoolStopEzLinkId;

    return *this;
}

BigSerial StudentStop::getHomeStopEzLinkId() const
{
    return homeStopEzLinkId;
}

void StudentStop::setHomeStopEzLinkId(BigSerial homeStopEzLinkId)
{
    this->homeStopEzLinkId = homeStopEzLinkId;
}

BigSerial StudentStop::getSchoolStopEzLinkId() const
{
    return schoolStopEzLinkId;
}

void StudentStop::setSchoolStopEzLinkId(BigSerial schoolStopEzLinkId)
{
    this->schoolStopEzLinkId = schoolStopEzLinkId;
}
