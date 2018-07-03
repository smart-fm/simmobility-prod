/*
 * TaxiStand.cpp
 *
 *  Created on: Oct 24, 2016
 *      Author: zhang huai peng
 */

#include <geospatial/network/TaxiStand.hpp>

namespace sim_mob
{
GeneralR_TreeManager<TaxiStand> TaxiStand::allTaxiStandMap;
TaxiStand::TaxiStand():roadSegment(nullptr),length(0),offset(0),id(0){
	// TODO Auto-generated constructor stub

}

TaxiStand::~TaxiStand() {
	// TODO Auto-generated destructor stub
}

void TaxiStand::setLocation(const Point& pos)
{
	location = pos;
}

const Point& TaxiStand::getLocation() const
{
	return location;
}

void TaxiStand::setRoadSegment(RoadSegment* segment)
{
	roadSegment = segment;
}

const RoadSegment* TaxiStand::getRoadSegment() const
{
	return roadSegment;
}

void TaxiStand::setLength(const double len)
{
	length = len;
}

double TaxiStand::getLength() const
{
	return length;
}

void TaxiStand::setOffset(const double offset)
{
	this->offset = offset;
}

double TaxiStand::getOffset() const
{
	return offset;
}

void TaxiStand::setStandId(int id)
{
	this->id = id;
}

int TaxiStand::getStandId() const
{
	return id;
}

double TaxiStand::getPosX() const
{
	return getLocation().getX();
}

double TaxiStand::getPosY() const
{
	return getLocation().getY();
}
}

