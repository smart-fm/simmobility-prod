/*
 * SlaBuilding.cpp
 *
 *  Created on: 18 Jan 2017
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/SlaBuilding.hpp>

using namespace sim_mob::long_term;

SlaBuilding::SlaBuilding(string sla_building_id, string sla_inc_crc, BigSerial sla_address_id): sla_building_id(sla_building_id),  sla_inc_crc(sla_inc_crc),  sla_address_id(sla_address_id){}

SlaBuilding::~SlaBuilding() {}

SlaBuilding::SlaBuilding(const SlaBuilding & source)
{
	this->sla_address_id = source.sla_address_id;
	this->sla_building_id = source.sla_building_id;
	this->sla_inc_crc = source.sla_inc_crc;
}

SlaBuilding& SlaBuilding::operator=(const SlaBuilding & source)
{
	this->sla_address_id = source.sla_address_id;
	this->sla_building_id = source.sla_building_id;
	this->sla_inc_crc = source.sla_inc_crc;

	return *this;
}

string SlaBuilding::getSla_building_id()
{
	return sla_building_id;
}

string SlaBuilding::getSla_inc_crc()
{
	return sla_inc_crc;
}

BigSerial SlaBuilding::getSla_address_id()
{
	return sla_address_id;
}

void SlaBuilding::setSla_building_id(string val)
{
	sla_building_id = val;
}

void SlaBuilding::setSla_inc_crc(string val)
{
	sla_inc_crc = val;
}

void SlaBuilding::setSla_address_id(BigSerial val)
{
	sla_address_id = val;
}


