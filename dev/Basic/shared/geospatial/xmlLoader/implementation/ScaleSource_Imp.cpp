#include "geo10-pimpl.hpp"

#include <stdexcept>
#include <utility>
#include "util/Utils.hpp"

using namespace sim_mob::xml;

void sim_mob::xml::scale_source_t_pimpl::pre ()
{
	model = std::make_pair(sim_mob::LinearScale::Range(), sim_mob::LinearScale::Range());
}

std::pair<sim_mob::LinearScale::Range, sim_mob::LinearScale::Range> sim_mob::xml::scale_source_t_pimpl::post_scale_source_t ()
{
	return model;
}

void sim_mob::xml::scale_source_t_pimpl::x_range (const ::std::string& value)
{
	std::pair<double, double> parsed = sim_mob::Utils::parse_scale_minmax(value);

	model.first.min = parsed.first;
	model.first.max = parsed.second;
}

void sim_mob::xml::scale_source_t_pimpl::y_range (const ::std::string& value)
{
	std::pair<double, double> parsed = sim_mob::Utils::parse_scale_minmax(value);

	model.second.min = parsed.first;
	model.second.max = parsed.second;
}



