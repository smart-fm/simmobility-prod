#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::links_map_t_pimpl::pre ()
{
}

std::pair<sim_mob::Link*,sim_mob::linkToLink> sim_mob::xml::links_map_t_pimpl::post_links_map_t ()
{
	return std::pair<sim_mob::Link*,sim_mob::linkToLink>(nullptr, sim_mob::linkToLink(nullptr))	;
}

void sim_mob::xml::links_map_t_pimpl::linkFrom (unsigned int value)
{
	//std::cout << "linkFrom: " <<value << std::endl;
}

void sim_mob::xml::links_map_t_pimpl::linkTo (unsigned int value)
{
	//std::cout << "linkTo: " <<value << std::endl;
}

void sim_mob::xml::links_map_t_pimpl::SegmentFrom (unsigned int value)
{
	//std::cout << "SegmentFrom: " <<value << std::endl;
}

void sim_mob::xml::links_map_t_pimpl::SegmentTo (unsigned int value)
{
	//std::cout << "SegmentTo: " <<value << std::endl;
}

void sim_mob::xml::links_map_t_pimpl::ColorSequence (std::pair<std::string,std::vector<std::pair<TrafficColor,std::size_t> > > value)
{
}


