//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PolyLine_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::PolyLine_t_pimpl::PolyPoint (sim_mob::Point2D value)
{
	model.push_back(value);
}

std::vector<sim_mob::Point2D> sim_mob::xml::PolyLine_t_pimpl::post_PolyLine_t ()
{
	return model;
}
