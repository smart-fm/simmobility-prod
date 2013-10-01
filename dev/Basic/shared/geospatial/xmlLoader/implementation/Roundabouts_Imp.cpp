//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roundabouts_pimpl::pre ()
{
	model.clear();
}

const std::vector<sim_mob::MultiNode*>& sim_mob::xml::roundabouts_pimpl::post_roundabouts ()
{
	return model;
}

void sim_mob::xml::roundabouts_pimpl::roundabout (sim_mob::MultiNode* value)
{
	model.push_back(value);
}

