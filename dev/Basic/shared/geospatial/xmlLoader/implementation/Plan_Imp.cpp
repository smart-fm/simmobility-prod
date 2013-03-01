#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Plan_t_pimpl::pre ()
{
}

std::pair<short int, std::vector<double> > sim_mob::xml::Plan_t_pimpl::post_Plan_t ()
{
}

void sim_mob::xml::Plan_t_pimpl::planID (unsigned char value)
{
	//std::cout << "planID: " << static_cast<unsigned short> (value) << std::endl;
}

void sim_mob::xml::Plan_t_pimpl::PhasePercentage (double value)
{
	//std::cout << "PhasePercentage: " <<value << std::endl;
}
