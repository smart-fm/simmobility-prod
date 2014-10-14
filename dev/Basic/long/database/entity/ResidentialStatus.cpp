//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


/*
 * ResidentialStatus.cpp
 *
 *  Created on: 4 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/ResidentialStatus.hpp>

using namespace sim_mob::long_term;

ResidentialStatus::ResidentialStatus(BigSerial id, std::string name) : id(id), name(name){}

ResidentialStatus::~ResidentialStatus(){}

BigSerial ResidentialStatus::getId() const
{
	return id;
}

std::string ResidentialStatus::getName() const
{
	return name;
}

namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const ResidentialStatus& data)
		{
			return strm << "{"
						<< "\"id \":\"" << data.id  << "\","
						<< "\"name \":\"" << data.name.c_str()  << "\""
						<< "}";
		}
	}
}

