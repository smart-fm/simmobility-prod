//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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


