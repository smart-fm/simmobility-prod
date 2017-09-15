/*
 * JobsBySectorByTaz.cpp
 *
 *  Created on: 26 Jul 2017
 *      Author: Gishara Premarathne <gishara@smart.mit.edu>
 */

#include "JobsBySectorByTaz.hpp"

using namespace sim_mob::long_term;

JobsBySectorByTaz::JobsBySectorByTaz(BigSerial tazId, int sector1, int sector2, int sector3, int sector4, int sector5, int sector6, int sector7,int sector8,int sector9, int sector10, int sector11, int sector98):
									 tazId(tazId), sector1(sector1),sector2(sector2),sector3(sector3),sector4(sector4),sector5(sector5),sector6(sector6),sector7(sector7),sector8(sector8),sector9(sector9),sector10(sector10), sector11(sector11),sector98(sector98){}

JobsBySectorByTaz::~JobsBySectorByTaz() {}

JobsBySectorByTaz& JobsBySectorByTaz::operator=(const JobsBySectorByTaz& source)
{
	this->tazId 		= source.tazId;
	this->sector1	= source.sector1;
	this->sector2 	= source.sector2;
	this->sector3 	= source.sector3;
	this->sector4 	= source.sector4;
	this->sector5	= source.sector5;
	this->sector6 	= source.sector6;
	this->sector7 	= source.sector7;
	this->sector8 	= source.sector8;
	this->sector9 	= source.sector9;
	this->sector10 	= source.sector10;
	this->sector11 	= source.sector11;
	this->sector98 	= source.sector98;

    return *this;
}

int JobsBySectorByTaz::getSector1() const
{
	return sector1;
}

void JobsBySectorByTaz::setSector1(int sector1)
{
	this->sector1 = sector1;
}

int JobsBySectorByTaz::getSector10() const
{
	return sector10;
}

void JobsBySectorByTaz::setSector10(int sector10)
{
	this->sector10 = sector10;
}

int JobsBySectorByTaz::getSector11() const
{
	return sector11;
}

void JobsBySectorByTaz::setSector11(int sector11)
{
	this->sector11 = sector11;
}

int JobsBySectorByTaz::getSector2() const
{
	return sector2;
}

void JobsBySectorByTaz::setSector2(int sector2)
{
	this->sector2 = sector2;
}

int JobsBySectorByTaz::getSector3() const
{
	return sector3;
}

void JobsBySectorByTaz::setSector3(int sector3)
{
	this->sector3 = sector3;
}

int JobsBySectorByTaz::getSector4() const
{
	return sector4;
}

void JobsBySectorByTaz::setSector4(int sector4)
{
	this->sector4 = sector4;
}

int JobsBySectorByTaz::getSector5() const
{
	return sector5;
}

void JobsBySectorByTaz::setSector5(int sector5)
{
	this->sector5 = sector5;
}

int JobsBySectorByTaz::getSector6() const
{
	return sector6;
}

void JobsBySectorByTaz::setSector6(int sector6)
{
	this->sector6 = sector6;
}

int JobsBySectorByTaz::getSector7() const
{
	return sector7;
}

void JobsBySectorByTaz::setSector7(int sector7)
{
	this->sector7 = sector7;
}

int JobsBySectorByTaz::getSector8() const
{
	return sector8;
}

void JobsBySectorByTaz::setSector8(int sector8)
{
	this->sector8 = sector8;
}

int JobsBySectorByTaz::getSector9() const
{
	return sector9;
}

void JobsBySectorByTaz::setSector9(int sector9)
{
	this->sector9 = sector9;
}

int JobsBySectorByTaz::getSector98() const
{
	return sector98;
}

void JobsBySectorByTaz::setSector98(int sector98)
{
	this->sector98 = sector98;
}

BigSerial JobsBySectorByTaz::getTazId() const
{
	return tazId;
}

void JobsBySectorByTaz::setTazId(BigSerial tazId)
{
	this->tazId = tazId;
}


