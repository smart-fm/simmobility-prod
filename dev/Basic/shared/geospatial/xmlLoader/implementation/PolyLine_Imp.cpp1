#include "geo9-pimpl.hpp"

using namespace sim_mob::xml;


void geo::PolyLine_t_pimpl::pre ()
{
}

void geo::PolyLine_t_pimpl::PolyPoint (sim_mob::Point2D PolyPoint)
{
	polyLine.push_back(PolyPoint);
}

std::vector<sim_mob::Point2D> geo::PolyLine_t_pimpl::post_PolyLine_t ()
{
	return polyLine;
}
