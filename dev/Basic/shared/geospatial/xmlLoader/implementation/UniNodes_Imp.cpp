//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::UniNodes_pimpl::pre ()
{
	model.clear();
}

const std::set<sim_mob::UniNode*>& sim_mob::xml::UniNodes_pimpl::post_UniNodes ()
{
	return model;
}

void sim_mob::xml::UniNodes_pimpl::UniNode (sim_mob::UniNode* value)
{
	  model.insert(value);
}

