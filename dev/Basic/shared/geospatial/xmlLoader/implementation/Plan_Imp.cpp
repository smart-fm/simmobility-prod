//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::plan_t_pimpl::pre ()
{
	model = std::pair<short, std::vector<double> >();
}

std::pair<short,std::vector<double> >& sim_mob::xml::plan_t_pimpl::post_plan_t ()
{
	return model;
}

void sim_mob::xml::plan_t_pimpl::planID (unsigned char value)
{
	model.first = static_cast<short> (value);
}

void sim_mob::xml::plan_t_pimpl::PhasePercentage (double value)
{
	model.second.push_back(value);
}
