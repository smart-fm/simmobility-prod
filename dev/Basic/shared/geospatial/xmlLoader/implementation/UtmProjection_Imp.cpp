#include "geo10-pimpl.hpp"

#include <stdexcept>
#include <algorithm>
#include <cctype>

using namespace sim_mob::xml;


void sim_mob::xml::utm_projection_t_pimpl::pre ()
{
	model = sim_mob::UTM_Projection();
}

sim_mob::UTM_Projection* sim_mob::xml::utm_projection_t_pimpl::post_utm_projection_t ()
{
	return new sim_mob::UTM_Projection(model);
}

void sim_mob::xml::utm_projection_t_pimpl::coordinate_system (const ::std::string& value)
{
	//Spaces are ignored.
	std::string trimmed = value;
	trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), isspace), trimmed.end());

	if (trimmed=="WGS84") {
		model.coordSys = sim_mob::COORD_WGS84;
	} else {
		throw std::runtime_error("Unknown coordinate system in UTM projection.");
	}
}

void sim_mob::xml::utm_projection_t_pimpl::utm_zone (const ::std::string& value)
{
	//Spaces are ignored.
	std::string trimmed = value;
	trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(), isspace), trimmed.end());

	if (trimmed=="48N") {
		model.utmZone = sim_mob::UTM_48N;
	} else {
		model.utmZone = sim_mob::UTM_INVALID;
	}
}


