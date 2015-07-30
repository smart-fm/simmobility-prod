//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * planningsubzone.cpp
 *
 *  Created on: 30 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/PlanningSubzone.hpp>

namespace sim_mob
{
	namespace long_term
	{

		PlanningSubzone::PlanningSubzone(BigSerial id, BigSerial planningAreaId, std::string name):id(id), planningAreaId(planningAreaId), name(name){}

		PlanningSubzone::~PlanningSubzone(){}

		PlanningSubzone::PlanningSubzone(PlanningSubzone &source)
		{
			this->id = source.id;
			this->planningAreaId = source.planningAreaId;
			this->name = source.name;
		}

		PlanningSubzone & PlanningSubzone::operator=( PlanningSubzone & source)
		{
			this->id = source.id;
			this->planningAreaId = source.planningAreaId;
			this->name = source.name;

			return *this;
		}

		BigSerial PlanningSubzone::getId() const
		{
			return id;
		}

		BigSerial PlanningSubzone::getPlanningAreaId() const
		{
			return planningAreaId;
		}

		std::string PlanningSubzone::getName() const
		{
			return name;
		}

		std::ostream& operator<<(std::ostream& strm, const PlanningSubzone& data)
		{
			return strm << "{"
						<< "\"id\":\"" << data.id << "\","
						<< "\"planningAreaId\":\"" << data.planningAreaId << "\","
						<< "\"name\":\"" << data.name << "\""
						<< "}";
		}

	}
} /* namespace sim_mob */
