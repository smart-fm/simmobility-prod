#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roadrunner_region_t_pimpl::pre ()
{
	model = sim_mob::RoadRunnerRegion();
}

sim_mob::RoadRunnerRegion sim_mob::xml::roadrunner_region_t_pimpl::post_roadrunner_region_t ()
{
	return model;
}

void sim_mob::xml::roadrunner_region_t_pimpl::id (int value)
{
	model.id = value;
}

void sim_mob::xml::roadrunner_region_t_pimpl::shape (const std::vector<sim_mob::LatLngLocation>& value)
{
	model.points = value;
}




