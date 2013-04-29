#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Signals_t_pimpl::pre ()
{
}

std::vector<sim_mob::Signal*> sim_mob::xml::Signals_t_pimpl::post_Signals_t ()
{
	  return sim_mob::Signal::all_signals_; //might not be very necessary
}

void sim_mob::xml::Signals_t_pimpl::signal (sim_mob::Signal* value)
{
	sim_mob::Signal::all_signals_.push_back(value);
}


