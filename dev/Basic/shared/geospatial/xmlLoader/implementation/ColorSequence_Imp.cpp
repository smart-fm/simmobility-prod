#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::ColorSequence_t_pimpl::pre ()
{
}

std::pair<sim_mob::TrafficLightType, std::vector<std::pair<sim_mob::TrafficColor,std::size_t> > > sim_mob::xml::ColorSequence_t_pimpl::post_ColorSequence_t ()
{

	return model;
}

void sim_mob::xml::ColorSequence_t_pimpl::TrafficLightType (sim_mob::TrafficLightType value)
{
	model.first = value;
//	std::cout << "TrafficLightType: " <<value << std::endl;
}

void sim_mob::xml::ColorSequence_t_pimpl::ColorDuration (std::pair<sim_mob::TrafficColor,std::size_t> value)
{
	model.second.push_back(value);
}


