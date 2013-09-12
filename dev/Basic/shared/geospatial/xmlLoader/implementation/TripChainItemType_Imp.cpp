//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripchainItemType_pimpl::pre ()
{
}

std::string sim_mob::xml::TripchainItemType_pimpl::post_TripchainItemType ()
{
	return post_string ();
}
