#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roundabouts_pimpl::pre ()
{
	model.clear();
}

const std::vector<sim_mob::MultiNode*>& sim_mob::xml::roundabouts_pimpl::post_roundabouts ()
{
	return model;
}

void sim_mob::xml::roundabouts_pimpl::roundabout (sim_mob::MultiNode* value)
{
	model.push_back(value);
}

