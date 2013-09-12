//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PointPair_t_pimpl::pre ()
{
	model = std::make_pair(sim_mob::Point2D(), sim_mob::Point2D());
}

void sim_mob::xml::PointPair_t_pimpl::first (sim_mob::Point2D value)
{
	model.first = value;
}

void sim_mob::xml::PointPair_t_pimpl::second (sim_mob::Point2D value)
{
	model.second = value;
}

std::pair<sim_mob::Point2D,sim_mob::Point2D> sim_mob::xml::PointPair_t_pimpl::post_PointPair_t ()
{
	return model;
}
