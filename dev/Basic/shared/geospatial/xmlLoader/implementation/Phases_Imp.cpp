//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Phases_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::Phases_t_pimpl::phase (sim_mob::Phase& phase)
{
	model.push_back(phase);
}

sim_mob::Signal::phases& sim_mob::xml::Phases_t_pimpl::post_phases_t ()
{
	return model;
}

