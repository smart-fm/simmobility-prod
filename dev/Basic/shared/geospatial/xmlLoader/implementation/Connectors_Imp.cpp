#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::connectors_t_pimpl::pre ()
{
	  model.clear();
}

std::set<std::pair<unsigned long,unsigned long> > sim_mob::xml::connectors_t_pimpl::post_connectors_t ()
{
	  return model;
}

void sim_mob::xml::connectors_t_pimpl::Connector (std::pair<unsigned long,unsigned long> value)
{
	  model.insert(value);
}

