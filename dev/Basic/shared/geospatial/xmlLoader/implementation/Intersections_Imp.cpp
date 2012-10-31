#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Intersections_pimpl::pre ()
{
	model.clear();
}

std::vector<sim_mob::MultiNode*>& sim_mob::xml::Intersections_pimpl::post_Intersections ()
{
	return model;
}

void sim_mob::xml::Intersections_pimpl::Intersection (sim_mob::MultiNode* value)
{
	model.push_back(value);
}


