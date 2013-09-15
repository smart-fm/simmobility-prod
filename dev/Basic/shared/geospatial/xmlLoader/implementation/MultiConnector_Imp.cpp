//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Multi_Connector_t_pimpl:: pre ()
{
}

std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > sim_mob::xml::Multi_Connector_t_pimpl::post_Multi_Connector_t ()
{
	  return model;
}

void sim_mob::xml::Multi_Connector_t_pimpl::RoadSegment (unsigned long long RoadSegment)
{
	  //tmp_rs++;
	  model.first = RoadSegment;
}

void sim_mob::xml::Multi_Connector_t_pimpl::Connectors (std::set<std::pair<unsigned long,unsigned long> > Connectors)
{
	  model.second = Connectors;
}

