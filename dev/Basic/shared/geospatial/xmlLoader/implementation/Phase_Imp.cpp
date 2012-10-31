#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Phase_t_pimpl::pre ()
{
}

void sim_mob::xml::Phase_t_pimpl::post_Phase_t ()
{
}

void sim_mob::xml::Phase_t_pimpl::phaseID (unsigned char value)
{
	//std::cout << "phaseID: " << static_cast<unsigned short> (value) << std::endl;
}

void sim_mob::xml::Phase_t_pimpl::name (const ::std::string& value)
{
	//std::cout << "name: " <<value << std::endl;
}

void sim_mob::xml::Phase_t_pimpl::links_map (std::multimap<sim_mob::Link*,sim_mob::linkToLink> value)
{
}

