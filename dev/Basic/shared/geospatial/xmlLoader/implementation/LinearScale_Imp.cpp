#include "geo10-pimpl.hpp"

#include <stdexcept>

using namespace sim_mob::xml;


void sim_mob::xml::linear_scale_t_pimpl::pre ()
{
	model = sim_mob::LinearScale();
}

sim_mob::LinearScale* sim_mob::xml::linear_scale_t_pimpl::post_linear_scale_t ()
{
	return new sim_mob::LinearScale(model);
}

void sim_mob::xml::linear_scale_t_pimpl::source (const std::pair<sim_mob::LinearScale::Range, sim_mob::LinearScale::Range>& value)
{
	model.sourceX = value.first;
	model.sourceY = value.second;
}

void sim_mob::xml::linear_scale_t_pimpl::destination (const std::pair<sim_mob::LinearScale::Range, sim_mob::LinearScale::Range>& value)
{
	model.destLatitude = value.first;
	model.destLongitude = value.second;
}


