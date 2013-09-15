//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roadrunner_shape_t_pimpl::pre ()
{
	model = std::vector<sim_mob::LatLngLocation>();
}

std::vector<sim_mob::LatLngLocation> sim_mob::xml::roadrunner_shape_t_pimpl::post_roadrunner_shape_t ()
{
	return model;
}

void sim_mob::xml::roadrunner_shape_t_pimpl::vertex (const sim_mob::LatLngLocation& value)
{
	model.push_back(value);
}


