/*
 * JobsBySectorByTaz.cpp
 *
 *  Created on: 26 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include <database/entity/JobsByIndustryTypeByTaz.hpp>

using namespace sim_mob::long_term;

JobsByIndustryTypeByTaz::JobsByIndustryTypeByTaz(BigSerial tazId, int industryType1, int industryType2, int industryType3, int industryType4, int industryType5, int industryType6, int industryType7,int industryType8,int industryType9, int industryType10, int industryType11, int industryType98):
                                     tazId(tazId), industryType1(industryType1),industryType2(industryType2),industryType3(industryType3),industryType4(industryType4),industryType5(industryType5),industryType6(industryType6),industryType7(industryType7),industryType8(industryType8),industryType9(industryType9),industryType10(industryType10), industryType11(industryType11),industryType98(industryType98){}

JobsByIndustryTypeByTaz::~JobsByIndustryTypeByTaz() {}

JobsByIndustryTypeByTaz& JobsByIndustryTypeByTaz::operator=(const JobsByIndustryTypeByTaz& source)
{
    this->tazId         = source.tazId;
    this->industryType1 = source.industryType1;
    this->industryType2     = source.industryType2;
    this->industryType3     = source.industryType3;
    this->industryType4     = source.industryType4;
    this->industryType5 = source.industryType5;
    this->industryType6     = source.industryType6;
    this->industryType7     = source.industryType7;
    this->industryType8     = source.industryType8;
    this->industryType9     = source.industryType9;
    this->industryType10    = source.industryType10;
    this->industryType11    = source.industryType11;
    this->industryType98    = source.industryType98;

    return *this;
}

int JobsByIndustryTypeByTaz::getIndustryType1() const
{
    return industryType1;
}

void JobsByIndustryTypeByTaz::setIndustryType1(int indType1)
{
    this->industryType1 = indType1;
}

int JobsByIndustryTypeByTaz::getIndustryType10() const
{
    return industryType10;
}

void JobsByIndustryTypeByTaz::setIndustryType10(int indType10)
{
    this->industryType10 = indType10;
}

int JobsByIndustryTypeByTaz::getIndustryType11() const
{
    return industryType11;
}

void JobsByIndustryTypeByTaz::setIndustryType11(int indType11)
{
    this->industryType11 = indType11;
}

int JobsByIndustryTypeByTaz::getIndustryType2() const
{
    return industryType2;
}

void JobsByIndustryTypeByTaz::setIndustryType2(int indType2)
{
    this->industryType2 = indType2;
}

int JobsByIndustryTypeByTaz::getIndustryType3() const
{
    return industryType3;
}

void JobsByIndustryTypeByTaz::setIndustryType3(int indType3)
{
    this->industryType3 = indType3;
}

int JobsByIndustryTypeByTaz::getIndustryType4() const
{
    return industryType4;
}

void JobsByIndustryTypeByTaz::setIndustryType4(int indType4)
{
    this->industryType4 = indType4;
}

int JobsByIndustryTypeByTaz::getIndustryType5() const
{
    return industryType5;
}

void JobsByIndustryTypeByTaz::setIndustryType5(int indType5)
{
    this->industryType5 = indType5;
}

int JobsByIndustryTypeByTaz::getIndustryType6() const
{
    return industryType6;
}

void JobsByIndustryTypeByTaz::setIndustryType6(int indType6)
{
    this->industryType6 = indType6;
}

int JobsByIndustryTypeByTaz::getIndustryType7() const
{
    return industryType7;
}

void JobsByIndustryTypeByTaz::setIndustryType7(int indType7)
{
    this->industryType7 = indType7;
}

int JobsByIndustryTypeByTaz::getIndustryType8() const
{
    return industryType8;
}

void JobsByIndustryTypeByTaz::setIndustryType8(int indType8)
{
    this->industryType8 = indType8;
}

int JobsByIndustryTypeByTaz::getIndustryType9() const
{
    return industryType9;
}

void JobsByIndustryTypeByTaz::setIndustryType9(int indType9)
{
    this->industryType9 = indType9;
}

int JobsByIndustryTypeByTaz::getIndustryType98() const
{
    return industryType98;
}

void JobsByIndustryTypeByTaz::setIndustryType98(int indType98)
{
    this->industryType98 = indType98;
}

BigSerial JobsByIndustryTypeByTaz::getTazId() const
{
    return tazId;
}

void JobsByIndustryTypeByTaz::setTazId(BigSerial tazId)
{
    this->tazId = tazId;
}


