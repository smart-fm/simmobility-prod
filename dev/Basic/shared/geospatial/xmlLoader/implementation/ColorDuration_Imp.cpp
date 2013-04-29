#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::ColorDuration_t_pimpl::pre ()
{
	model.first = sim_mob::InvalidTrafficColor;
	model.second = -1;
}

void sim_mob::xml::ColorDuration_t_pimpl::TrafficColor (sim_mob::TrafficColor value)
{
 model.first = value;
}

void sim_mob::xml::ColorDuration_t_pimpl::Duration (short value)
{
	model.second = /*static_cast<unsigned short>*/ (value) ;
	//std::cout << "Duration: " << static_cast<unsigned short> (value) << std::endl;
}

std::pair<sim_mob::TrafficColor,short> sim_mob::xml::ColorDuration_t_pimpl::post_ColorDuration_t ()
{
	return model;
}

