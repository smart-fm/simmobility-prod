#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void roadrunner_shape_t_pimpl::pre ()
{
	model = std::vector<sim_mob::LatLngLocation>();
}

std::vector<sim_mob::LatLngLocation> roadrunner_shape_t_pimpl::post_roadrunner_shape_t ()
{
	return model;
}

void roadrunner_shape_t_pimpl::vertex (const sim_mob::LatLngLocation& value)
{
	model.push_back(value);
}


