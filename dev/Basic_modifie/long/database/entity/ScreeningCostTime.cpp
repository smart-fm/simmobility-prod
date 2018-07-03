//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * screeningCostTime.cpp
 *
 *  Created on: 20 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/ScreeningCostTime.hpp>
#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{

		ScreeningCostTime::ScreeningCostTime( int _id, int _planningAreaOrigin, std::string _areaOrigin, int _planningAreaDestination, std::string _areaDestination, double _time,
											  double _cost): id(_id), planningAreaOrigin(_planningAreaOrigin), areaOrigin(_areaOrigin),
											  planningAreaDestination(_planningAreaDestination), areaDestination(_areaDestination), time(_time), cost(_cost) {}

		ScreeningCostTime::~ScreeningCostTime(){}

		int ScreeningCostTime::getId() const
		{
			return id;
		}

		int ScreeningCostTime::getPlanningAreaOrigin() const
		{
			return planningAreaOrigin;
		}

		std::string ScreeningCostTime::getAreaOrigin() const
		{
			return areaOrigin;
		}

		int ScreeningCostTime::getPlanningAreaDestination() const
		{
			return planningAreaDestination;
		}

		std::string ScreeningCostTime::getAreaDestination() const
		{
			return areaDestination;
		}

		double ScreeningCostTime::getTime() const
		{
			return time;
		}

		double ScreeningCostTime::getCost() const
		{
			return cost;
		}

		std::ostream& operator<<(std::ostream& strm, const ScreeningCostTime& data)
		{
			return strm << "{"
						<< "\"id\":\"" << data.id << "\","
						<< "\"planningAreaOrigin\":\"" << data.getPlanningAreaOrigin() << "\","
						<< "\"getAreaOrigin\":\"" << data.getAreaOrigin() << "\","
						<< "\"planningAreaDestination\":\"" << data.getPlanningAreaDestination() << "\","
						<< "\"AreaDestination\":\"" << data.getPlanningAreaOrigin() << "\","
						<< "\"time\":\"" << data.getTime() << "\","
						<< "\"cost\":\"" << data.getCost() << "\""
						<< "}";
		}
	}
}
