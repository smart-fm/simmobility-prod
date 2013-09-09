#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void roadrunner_vertex_t_pimpl::pre ()
{
	model = sim_mob::LatLngLocation();
}

sim_mob::LatLngLocation roadrunner_vertex_t_pimpl::post_roadrunner_vertex_t ()
{
	return model;
}

void roadrunner_vertex_t_pimpl::latitude (double value)
{
	model.latitude = value;
}

void roadrunner_vertex_t_pimpl::longitude (double value)
{
	model.longitude = value;
}

