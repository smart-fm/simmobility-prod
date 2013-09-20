//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadBump_t_pimpl::pre ()
{
}

void sim_mob::xml::RoadBump_t_pimpl::roadBumpID (const ::std::string& value)
{
	//std::cout << "roadBumpID: " <<value << std::endl;
}

void sim_mob::xml::RoadBump_t_pimpl::segmentID (unsigned long long value)
{
}

void sim_mob::xml::RoadBump_t_pimpl::post_RoadBump_t ()
{
	//std::pair<unsigned long, sim_mob::RoadItem*> temp = RoadItem_t_pimpl::post_RoadItem_t();
}
