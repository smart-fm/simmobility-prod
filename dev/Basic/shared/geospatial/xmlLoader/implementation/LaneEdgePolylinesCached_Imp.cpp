#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::laneEdgePolylines_cached_t_pimpl::pre ()
{
	model.clear();
}

std::vector<std::vector<sim_mob::Point2D> > sim_mob::xml::laneEdgePolylines_cached_t_pimpl::post_laneEdgePolylines_cached_t ()
{
	return model;
}

void sim_mob::xml::laneEdgePolylines_cached_t_pimpl::laneEdgePolyline_cached (std::pair<short,std::vector<sim_mob::Point2D> > value)
{
	model.insert(model.begin()+value.first, value.second);
}

