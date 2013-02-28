#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Phases_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::Phases_t_pimpl::Phase (sim_mob::Phase phase)
{
	model.push_back(phase);
}

sim_mob::Signal::phases sim_mob::xml::Phases_t_pimpl::post_Phases_t ()
{
	return model;
}

