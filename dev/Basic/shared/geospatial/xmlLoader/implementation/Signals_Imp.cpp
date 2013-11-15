//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Signals_t_pimpl::pre ()
{
}

std::vector<sim_mob::Signal*>& sim_mob::xml::Signals_t_pimpl::post_Signals_t ()
{
	return sim_mob::Signal::all_signals_; //might not be very necessary
}

void sim_mob::xml::Signals_t_pimpl::Signal (sim_mob::Signal* value)
{
	sim_mob::Signal::all_signals_.push_back(value);
	StreetDirectory::instance().registerSignal(*value);
}


