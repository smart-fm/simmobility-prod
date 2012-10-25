#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PolyLine_t_pimpl::pre ()
{
	polyLine.clear();
}

void sim_mob::xml::PolyLine_t_pimpl::PolyPoint (sim_mob::Point2D PolyPoint)
{
	polyLine.push_back(PolyPoint);
}

std::vector<sim_mob::Point2D> sim_mob::xml::PolyLine_t_pimpl::post_PolyLine_t ()
{
	return polyLine;
}
