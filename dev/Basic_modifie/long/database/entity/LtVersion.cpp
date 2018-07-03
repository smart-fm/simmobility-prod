/*
 * LtVersion.cpp
 *
 *  Created on: 14 Sep 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/LtVersion.hpp>

using namespace sim_mob::long_term;

LtVersion::LtVersion(long long _id, string _base_version,string _comments,string _user_id, tm _change_date)
				: id(_id),base_version(_base_version), comments(_comments), user_id(_user_id), change_date(_change_date){}

LtVersion::~LtVersion(){}

long long LtVersion::getId() const
{
	return id;
}

string LtVersion::getBase_version() const
{
	return base_version;
}

string LtVersion::getComments() const
{
	return comments;
}

string LtVersion::getUser_id() const
{
	return user_id;
}

tm LtVersion::getChange_date() const
{
	return change_date;
}

void LtVersion::setId(long long _id)
{
	id = _id;
}

void LtVersion::setBase_version(string _base_version)
{
	base_version = _base_version;
}

void LtVersion::setComments(string _comments)
{
	comments = _comments;
}

void LtVersion::setUser_id(string _user_id)
{
	user_id = _user_id;
}

void LtVersion::setChange_date(tm _change_date)
{
	change_date = _change_date;
}



