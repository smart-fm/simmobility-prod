//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;




void sim_mob::xml::Lanes_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::Lanes_pimpl::Lane (sim_mob::Lane* value)
{
	model.push_back(value);
}

std::vector<sim_mob::Lane*> sim_mob::xml::Lanes_pimpl::post_Lanes ()
{
	//Register these for lookup later
	for (std::vector<sim_mob::Lane*>::iterator it=model.begin(); it!=model.end(); it++) {
		book.addLane(*it);
	}

	return model;
}
