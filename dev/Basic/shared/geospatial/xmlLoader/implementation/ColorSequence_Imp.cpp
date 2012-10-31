#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::ColorSequence_t_pimpl::pre ()
{
}

std::pair<std::string,std::vector<std::pair<sim_mob::TrafficColor,std::size_t> > > sim_mob::xml::ColorSequence_t_pimpl::post_ColorSequence_t ()
{
	return std::pair<std::string,std::vector<std::pair<TrafficColor,std::size_t> > >();
}

void sim_mob::xml::ColorSequence_t_pimpl::TrafficLightType (const ::std::string& value)
{
	std::cout << "TrafficLightType: " <<value << std::endl;
}

void sim_mob::xml::ColorSequence_t_pimpl::ColorDuration (std::pair<sim_mob::TrafficColor,std::size_t> value)
{
}


