#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::fwdBckSegments_t_pimpl::pre ()
{
	model.clear();
}

std::vector<sim_mob::RoadSegment*> sim_mob::xml::fwdBckSegments_t_pimpl::post_fwdBckSegments_t ()
{
	return model;
}

void sim_mob::xml::fwdBckSegments_t_pimpl::Segment (sim_mob::RoadSegment* value)
{
	model.push_back(value);
}

