//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

/////////////////////////////////////////////

// new_connectors_t_pimpl
//

void sim_mob::xml::new_connectors_t_pimpl::
pre ()
{
}

void sim_mob::xml::new_connectors_t_pimpl::
new_connector (std::pair<unsigned long,boost::tuple<unsigned long,unsigned long,unsigned long> > & new_connector)
{
  // TODO
  //
}

std::map<unsigned long,boost::tuple<unsigned long,unsigned long,unsigned long> > & sim_mob::xml::new_connectors_t_pimpl::
post_new_connectors_t ()
{
  // TODO
  //
  // return ... ;
}

