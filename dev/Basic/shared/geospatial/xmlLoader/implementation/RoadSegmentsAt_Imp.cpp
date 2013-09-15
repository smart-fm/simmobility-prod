//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadSegmentsAt_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::RoadSegmentsAt_t_pimpl::segmentID (unsigned long long value)
{
	model.insert(value);
}

std::set<unsigned long> sim_mob::xml::RoadSegmentsAt_t_pimpl::post_RoadSegmentsAt_t ()
{
	return model;
}
