/*
 * BuildingMatch.cpp
 *
 *  Created on: 18 Jan 2017
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/BuildingMatch.hpp>

using namespace sim_mob::long_term;

BuildingMatch::BuildingMatch(BigSerial _fm_building, BigSerial _fm_building_id_2008,string _sla_building_id, string _sla_inc_cnc,int _match_code, tm _match_date):
							 fm_building(_fm_building),  fm_building_id_2008(_fm_building_id_2008),	 sla_building_id(_sla_building_id),  sla_inc_cnc(_sla_inc_cnc),
							 match_code(_match_code), match_date(_match_date){}

BuildingMatch::~BuildingMatch() {}

BuildingMatch::BuildingMatch(const BuildingMatch & source)
{
	this->fm_building = source.fm_building;
	this->fm_building = source.fm_building;
	this->fm_building_id_2008 = source.fm_building_id_2008;
	this->sla_building_id = source.sla_building_id;
	this->sla_inc_cnc = source.sla_inc_cnc;
	this->match_code = source.match_code;
	this->match_date = source.match_date;
}

BuildingMatch & BuildingMatch::operator=(const BuildingMatch &source)
{
	this->fm_building = source.fm_building;
	this->fm_building = source.fm_building;
	this->fm_building_id_2008 = source.fm_building_id_2008;
	this->sla_building_id = source.sla_building_id;
	this->sla_inc_cnc = source.sla_inc_cnc;
	this->match_code = source.match_code;
	this->match_date = source.match_date;

	return *this;
}

void BuildingMatch::setFm_building(BigSerial val)
{
	fm_building = val;
}

void BuildingMatch::setFm_building_id_2008(BigSerial val)
{
	fm_building_id_2008 = val;
}

void BuildingMatch::setSla_building_id(string val)
{
	sla_building_id = val;
}

void BuildingMatch::setSla_inc_cnc(string val)
{
	sla_inc_cnc = val;
}

void BuildingMatch::setMatch_code(int val)
{
	match_code = val;
}

void BuildingMatch::setMatch_date(std::tm val)
{
	match_date = val;
}


BigSerial BuildingMatch::getFm_building()
{
	return fm_building;
}

BigSerial BuildingMatch::getFm_building_id_2008()
{
	return fm_building_id_2008;
}

string BuildingMatch::getSla_building_id()
{
	return sla_building_id;
}

string BuildingMatch::getSla_inc_cnc()
{
	return sla_inc_cnc;
}

int BuildingMatch::getMatch_code()
{
	return match_code;
}

tm  BuildingMatch::getMatch_date()
{
	return match_date;
}
