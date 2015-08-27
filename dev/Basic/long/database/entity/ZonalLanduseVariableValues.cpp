//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * ZonalLanduseVariableValues.cpp
 *
 *  Created on: 6 Aug, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/ZonalLanduseVariableValues.hpp>

using namespace sim_mob::long_term;

ZonalLanduseVariableValues::ZonalLanduseVariableValues(	int alt_id,	int dgpid, int dwl, double f_loc_com, double f_loc_res, double f_loc_open, double odi10_loc, double dis2mrt, double dis2exp ):
														alt_id(alt_id),	dgpid(dgpid), dwl(dwl), f_loc_com(f_loc_com), f_loc_res(f_loc_res), f_loc_open(f_loc_open), odi10_loc(odi10_loc), dis2mrt(dis2mrt), dis2exp(dis2exp) {}

ZonalLanduseVariableValues::ZonalLanduseVariableValues( const ZonalLanduseVariableValues & source)
{
	this->alt_id = source.alt_id;
	this->dgpid  = source.dgpid;
	this->dwl  	 = source.dwl;
	this->f_loc_com = source.f_loc_com;
	this->f_loc_res = source.f_loc_res;
	this->f_loc_open = source.f_loc_open;
	this->odi10_loc = source.odi10_loc;
	this->dis2mrt = source.dis2mrt;
	this->dis2exp = source.dis2exp;
}

ZonalLanduseVariableValues& ZonalLanduseVariableValues::operator=( const ZonalLanduseVariableValues &source )
{
	this->alt_id = source.alt_id;
	this->dgpid  = source.dgpid;
	this->dwl  	 = source.dwl;
	this->f_loc_com = source.f_loc_com;
	this->f_loc_res = source.f_loc_res;
	this->f_loc_open = source.f_loc_open;
	this->odi10_loc = source.odi10_loc;
	this->dis2mrt = source.dis2mrt;
	this->dis2exp = source.dis2exp;

	return *this;
}

ZonalLanduseVariableValues::~ZonalLanduseVariableValues(){}

int ZonalLanduseVariableValues::getAltId() const
{
	return alt_id;
}

int ZonalLanduseVariableValues::getDgpid() const
{
	return dgpid;
}

int ZonalLanduseVariableValues::getDwl() const
{
	return dwl;
}

double ZonalLanduseVariableValues::getFLocCom() const
{
	return f_loc_com;
}

double ZonalLanduseVariableValues::getFLocRes() const
{
	return f_loc_res;
}


double ZonalLanduseVariableValues::getFLocOpen() const
{
	return f_loc_open;
}


double ZonalLanduseVariableValues::getOdi10Loc() const
{
	return odi10_loc;
}

double ZonalLanduseVariableValues::getDis2mrt() const
{
	return dis2mrt;
}

double ZonalLanduseVariableValues::getDis2exp() const
{
	return dis2exp;
}


std::ostream& operator<<(std::ostream& strm, const ZonalLanduseVariableValues& data)
{
    return strm << "{"
				<< "\"dgpid \":\"" << data.getDgpid() << "\","
				<< "\"dwl \":\"" << data.getDwl() << "\","
				<< "\"f_loc_com \":\"" << data.getFLocCom() << "\","
				<< "\"f_loc_res \":\"" << data.getFLocRes() << "\","
				<< "\"f_loc_open \":\"" << data.getFLocOpen() << "\","
				<< "\"odi10_loc \":\"" << data.getOdi10Loc() << "\","
				<< "\"dis2mrt \":\"" << data.getDis2mrt() << "\","
				<< "\"distoexp \":\"" << data.getDis2exp() << "\","
				<< "}";
}
