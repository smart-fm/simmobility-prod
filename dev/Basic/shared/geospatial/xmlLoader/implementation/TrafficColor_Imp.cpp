#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;

sim_mob::TrafficColor getTrafficColorType(std::string value)
{
	if(value == "InvalidTrafficColor") return sim_mob::InvalidTrafficColor;
	else if(value == "Red") return sim_mob::Red;
	else if(value == "Amber") return sim_mob::Amber;
	else if(value == "Green") return sim_mob::Green;
	else if(value == "FlashingRed") return sim_mob::FlashingRed;
	else if(value == "FlashingAmber") return sim_mob::FlashingAmber;
	else if(value == "FlashingGreen") return sim_mob::FlashingGreen;
	throw std::runtime_error("Unknown traffic color");
}
void sim_mob::xml::TrafficColor_t_pimpl::pre ()
{
}

sim_mob::TrafficColor sim_mob::xml::TrafficColor_t_pimpl::post_TrafficColor_t ()
{
	return  getTrafficColorType(post_string ());
}
