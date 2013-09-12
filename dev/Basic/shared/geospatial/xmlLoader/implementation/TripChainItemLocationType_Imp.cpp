//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::TripchainItemLocationType_pimpl::pre ()
{
}

std::string sim_mob::xml::TripchainItemLocationType_pimpl::post_TripchainItemLocationType ()
{
	return post_string ();
}
