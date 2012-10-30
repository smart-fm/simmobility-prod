#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PolyLine_t_pimpl::pre ()
{
	model.clear();
}

void sim_mob::xml::PolyLine_t_pimpl::PolyPoint (sim_mob::Point2D value)
{
	model.push_back(value);
}

std::vector<sim_mob::Point2D> sim_mob::xml::PolyLine_t_pimpl::post_PolyLine_t ()
{
	return model;
}
