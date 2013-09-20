//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PolyPoint_t_pimpl::pre ()
{
	model = Point2D();
}

void sim_mob::xml::PolyPoint_t_pimpl::pointID (const ::std::string& value)
{
	//TODO: What to do with the saved ID?
	//savedID = pointID;
}

void sim_mob::xml::PolyPoint_t_pimpl::location (sim_mob::Point2D value)
{
	model = value;
}

sim_mob::Point2D sim_mob::xml::PolyPoint_t_pimpl::post_PolyPoint_t ()
{
	//TODO: For now, we simply return the position. May save the ID later.
	return model;
}
