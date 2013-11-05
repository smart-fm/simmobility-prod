//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::linkAndCrossings_t_pimpl::pre ()
{
	model.clear();
}

sim_mob::LinkAndCrossingC sim_mob::xml::linkAndCrossings_t_pimpl::post_linkAndCrossings_t ()
{
	return model;
}

void sim_mob::xml::linkAndCrossings_t_pimpl::linkAndCrossing (sim_mob::LinkAndCrossing value)
{
	model.insert(value);
}

