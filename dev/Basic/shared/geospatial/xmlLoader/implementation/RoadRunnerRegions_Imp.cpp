#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roadrunner_regions_t_pimpl::pre ()
{
	model = std::map<int, sim_mob::RoadRunnerRegion>();
}

std::map<int, sim_mob::RoadRunnerRegion> sim_mob::xml::roadrunner_regions_t_pimpl::post_roadrunner_regions_t ()
{
	return model;
}

void sim_mob::xml::roadrunner_regions_t_pimpl::region (const sim_mob::RoadRunnerRegion& value)
{
	model[value.id] = value;
}


