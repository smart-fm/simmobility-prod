//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::linkAndCrossing_t_pimpl::pre ()
{
	model = sim_mob::LinkAndCrossing();
}

sim_mob::LinkAndCrossing& sim_mob::xml::linkAndCrossing_t_pimpl::post_linkAndCrossing_t ()
{
	return model;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::ID (unsigned char value)
{

	model.id = value;
}

void sim_mob::xml::linkAndCrossing_t_pimpl::linkID (unsigned int value)
{
	model.link = book.getLink(value);
}

void sim_mob::xml::linkAndCrossing_t_pimpl::crossingID (unsigned int value)
{
	model.crossing = book.getCrossing(value);
}

void sim_mob::xml::linkAndCrossing_t_pimpl::angle (unsigned char value)
{
	model.angle = value;
}


