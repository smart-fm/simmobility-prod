//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::connector_t_pimpl::pre ()
{
	model = std::make_pair(0,0);
}

std::pair<unsigned long,unsigned long> sim_mob::xml::connector_t_pimpl::post_connector_t ()
{
	  return model;
}

void sim_mob::xml::connector_t_pimpl::laneFrom (unsigned long long value)
{
	  model.first = value;
}

void sim_mob::xml::connector_t_pimpl::laneTo (unsigned long long value)
{
	model.second = value;
}


