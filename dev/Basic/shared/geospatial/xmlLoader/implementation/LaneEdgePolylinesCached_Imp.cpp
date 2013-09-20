//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::laneEdgePolylines_cached_t_pimpl::pre ()
{
	model.clear();
}

std::vector<std::vector<sim_mob::Point2D> > sim_mob::xml::laneEdgePolylines_cached_t_pimpl::post_laneEdgePolylines_cached_t ()
{
	return model;
}

void sim_mob::xml::laneEdgePolylines_cached_t_pimpl::laneEdgePolyline_cached (std::pair<short,std::vector<sim_mob::Point2D> > value)
{
	model.insert(model.begin()+value.first, value.second);
}

