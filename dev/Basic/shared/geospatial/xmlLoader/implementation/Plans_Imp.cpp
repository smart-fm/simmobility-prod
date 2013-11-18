//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Plans_t_pimpl::pre ()
{
	model = (std::vector<std::vector<double> >());
}

std::vector<std::vector<double> >& sim_mob::xml::Plans_t_pimpl::post_Plans_t ()
{
	return model;
}

void sim_mob::xml::Plans_t_pimpl::plan (std::pair<short,std::vector<double> > &value)
{
	model.push_back(value.second);
}
