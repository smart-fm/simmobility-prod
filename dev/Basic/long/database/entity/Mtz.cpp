//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * mtz.cpp
 *
 *  Created on: 30 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Mtz.hpp>

namespace sim_mob
{
	namespace long_term
	{
		Mtz::Mtz(BigSerial id, BigSerial planningSubzoneId, std::string name):id(id), planningSubzoneId(planningSubzoneId), name(name){}

		Mtz::~Mtz(){}

		Mtz::Mtz(const Mtz& source)
		{
			this->id = source.id;
			this->planningSubzoneId = source.planningSubzoneId;
			this->name = source.name;
		}

		Mtz& Mtz::operator =(const Mtz &source)
		{
			this->id = source.id;
			this->planningSubzoneId  = source.planningSubzoneId;
			this->name = source.name;

			return *this;
		}

		BigSerial Mtz::getId() const
		{
			return id;
		}

		BigSerial Mtz::getPlanningSubzoneId() const
		{
			return planningSubzoneId;
		}

		std::string Mtz::getName() const
		{
			return name;
		}

		std::ostream& operator<<(std::ostream& strm, const Mtz& data)
		{
			return strm << "{"
						<< "\"id\":\"" << data.id << "\","
						<< "\"planningSubzoneId\":\"" << data.planningSubzoneId << "\","
						<< "\"name\":\"" << data.name << "\""
						<< "}";
		}


	}
} /* namespace sim_mob */





