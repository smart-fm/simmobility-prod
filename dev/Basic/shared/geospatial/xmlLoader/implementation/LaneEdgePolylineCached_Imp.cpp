//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::laneEdgePolyline_cached_t_pimpl::pre ()
{
	model = std::make_pair(-1, std::vector<sim_mob::Point2D>());
}

void sim_mob::xml::laneEdgePolyline_cached_t_pimpl::laneNumber (short value)
{
	model.first = value;
}

void sim_mob::xml::laneEdgePolyline_cached_t_pimpl::polyline (std::vector<sim_mob::Point2D> value)
{
	model.second = value;
}

std::pair<short,std::vector<sim_mob::Point2D> > sim_mob::xml::laneEdgePolyline_cached_t_pimpl::post_laneEdgePolyline_cached_t ()
{
	return model;
}
