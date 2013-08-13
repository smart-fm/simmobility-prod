#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::coordinate_map_t_pimpl::pre ()
{
	model.clear();
}

std::vector<sim_mob::CoordinateTransform*> sim_mob::xml::coordinate_map_t_pimpl::post_coordinate_map_t ()
{
	return model;
}

void sim_mob::xml::coordinate_map_t_pimpl::utm_projection (sim_mob::UTM_Projection* value)
{
	model.push_back(value);
}

void sim_mob::xml::coordinate_map_t_pimpl::linear_scale (sim_mob::LinearScale* value)
{
	model.push_back(value);
}


