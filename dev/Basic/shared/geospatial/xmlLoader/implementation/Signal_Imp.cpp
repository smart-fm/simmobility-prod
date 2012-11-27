#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Signal_t_pimpl::pre ()
{
}

sim_mob::Signal* sim_mob::xml::Signal_t_pimpl::post_Signal_t ()
{
	return nullptr;
}

void sim_mob::xml::Signal_t_pimpl::signalID (unsigned char value)
{
	//std::cout << "signalID: " << static_cast<unsigned short> (value) << std::endl;
}

void sim_mob::xml::Signal_t_pimpl::nodeID (unsigned int value)
{
	//std::cout << "nodeID: " <<value << std::endl;
}

void sim_mob::xml::Signal_t_pimpl::signalTimingMode ()
{
}

void sim_mob::xml::Signal_t_pimpl::linkAndCrossings (sim_mob::LinkAndCrossingC value)
{
}

void sim_mob::xml::Signal_t_pimpl::SplitPlan (sim_mob::SplitPlan value)
{
}


