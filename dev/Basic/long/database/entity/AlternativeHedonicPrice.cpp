//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)


/*
 * AlternativeHedonicPrice.cpp
 *
 *  Created on: 24 Feb 2016
 *  Author: Chetan Rogbeer <Chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/AlternativeHedonicPrice.hpp>

namespace sim_mob
{
	namespace long_term
	{
		AlternativeHedonicPrice::AlternativeHedonicPrice( int _planning_area_id, std::string _planning_area, int _dwelling_type,double _total_price):
														 planning_area_id(_planning_area_id), planning_area(_planning_area), dwelling_type(_dwelling_type),
														 total_price(_total_price){}

		AlternativeHedonicPrice::~AlternativeHedonicPrice() {}

		int AlternativeHedonicPrice::getPlanningAreaId()
		{
			return planning_area_id;
		}

		std::string AlternativeHedonicPrice::getPlanningArea()
		{
			return planning_area;
		}

		int AlternativeHedonicPrice::getDwellingType()
		{
			return dwelling_type;
		}

		double AlternativeHedonicPrice::getTotalPrice()
		{
			return total_price;
		}
	}
}
