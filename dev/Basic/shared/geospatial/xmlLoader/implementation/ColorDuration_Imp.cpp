#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::ColorDuration_t_pimpl::pre ()
{
}

void sim_mob::xml::ColorDuration_t_pimpl::TrafficColor ()
{
}

void sim_mob::xml::ColorDuration_t_pimpl::Duration (unsigned char value)
{
	//std::cout << "Duration: " << static_cast<unsigned short> (value) << std::endl;
}

std::pair<sim_mob::TrafficColor,std::size_t> sim_mob::xml::ColorDuration_t_pimpl::post_ColorDuration_t ()
{
}

