//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadItem_t_pimpl::pre ()
{
	model = sim_mob::RoadItem();
	offset = 0;
}

std::pair<unsigned long,sim_mob::RoadItem*> sim_mob::xml::RoadItem_t_pimpl::post_RoadItem_t ()
{
	sim_mob::RoadItem *res = new sim_mob::RoadItem(model);

	return std::make_pair(offset, res);
}


void sim_mob::xml::RoadItem_t_pimpl::id (unsigned long long value)
{
	model.id = value;
}

void sim_mob::xml::RoadItem_t_pimpl::Offset (unsigned short value)
{
	offset = value;
}

void sim_mob::xml::RoadItem_t_pimpl::start (sim_mob::Point2D value)
{
	model.start = value;
}

void sim_mob::xml::RoadItem_t_pimpl::end (sim_mob::Point2D value)
{
	model.end = value;
}

