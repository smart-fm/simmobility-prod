/*
 * planningArea.cpp
 *
 *  Created on: 30 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/PlanningArea.hpp>

using namespace sim_mob::long_term;

PlanningArea::PlanningArea(BigSerial id, std::string name):id(id),name(name){}

PlanningArea::~PlanningArea(){}

PlanningArea::PlanningArea(const PlanningArea& source)
{
	this->id = source.id;
	this->name = source.name;

}

PlanningArea& PlanningArea::operator=(const PlanningArea& source)
{
	this->id = source.id;
	this->name = source.name;

	return *this;
}

BigSerial PlanningArea::getId() const
{
	return id;
}
std::string PlanningArea::getName() const
{
	return name;
}

namespace sim_mob
{
	namespace long_term
	{
		std::ostream& operator<<(std::ostream& strm, const PlanningArea& data)
		{
			return strm << "{"
					<< "\"id\":\"" << data.id << "\","
					<< "\"name\":\"" << data.name << "\""
					<< "}";
		}
	}
}




