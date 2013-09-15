//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;



void sim_mob::xml::Links_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::Links_pimpl::Link (sim_mob::Link* value)
{
	model.push_back(value);
}

std::vector<sim_mob::Link*> Links_pimpl::post_Links ()
{
	for (std::vector<sim_mob::Link*>::iterator linkIt=model.begin(); linkIt!=model.end(); linkIt++) {
		book.addLink(*linkIt);
	}


	return model;
}

