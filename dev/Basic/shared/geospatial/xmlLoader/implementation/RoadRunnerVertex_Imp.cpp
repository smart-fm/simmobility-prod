//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roadrunner_vertex_t_pimpl::pre ()
{
	model = sim_mob::LatLngLocation();
}

sim_mob::LatLngLocation sim_mob::xml::roadrunner_vertex_t_pimpl::post_roadrunner_vertex_t ()
{
	return model;
}

void sim_mob::xml::roadrunner_vertex_t_pimpl::latitude (double value)
{
	model.latitude = value;
}

void sim_mob::xml::roadrunner_vertex_t_pimpl::longitude (double value)
{
	model.longitude = value;
}

