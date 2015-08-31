//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * HitsIndividualLogsum.cpp
 *
 *  Created on: 17 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/HitsIndividualLogsum.hpp>

using namespace std;
using namespace sim_mob::long_term;


HitsIndividualLogsum::HitsIndividualLogsum(int id, string hitsId, int paxId, int homePostcode, int homeTaz, int workPostcode, int workTaz, int cars )
										  :id(id), hitsId(hitsId), paxId(paxId), homePostcode(homePostcode), homeTaz(homeTaz), workPostcode(workPostcode), workTaz(workTaz), cars(cars){}

HitsIndividualLogsum::HitsIndividualLogsum( const HitsIndividualLogsum& source)
{
	this->id = source.id;
	this->hitsId = source.hitsId;
	this->paxId = source.paxId;
	this->homePostcode = source.homePostcode;
	this->homeTaz = source.homeTaz;
	this->workPostcode = source.workPostcode;
	this->workTaz = source.workTaz;
	this->cars = source.cars;
}

HitsIndividualLogsum& HitsIndividualLogsum::operator=( const HitsIndividualLogsum& source)
{
	this->id = source.id;
	this->hitsId = source.hitsId;
	this->paxId = source.paxId;
	this->homePostcode = source.homePostcode;
	this->homeTaz = source.homeTaz;
	this->workPostcode = source.workPostcode;
	this->workTaz = source.workTaz;
	this->cars = source.cars;

	return *this;
}

int HitsIndividualLogsum::getId() const
{
	return id;
}

string HitsIndividualLogsum::getHitsId() const
{
	return hitsId;
}

int HitsIndividualLogsum::getPaxId() const
{
	return paxId;
}

int HitsIndividualLogsum::getHomePostcode() const
{
	return homePostcode;
}

int HitsIndividualLogsum::getHomeTaz() const
{
	return homeTaz;
}

int HitsIndividualLogsum::getWorkPostcode() const
{
	return workPostcode;
}

int HitsIndividualLogsum::getWorkTaz() const
{
	return workTaz;
}

int HitsIndividualLogsum::getCars() const
{
	return cars;
}

HitsIndividualLogsum::~HitsIndividualLogsum(){}
