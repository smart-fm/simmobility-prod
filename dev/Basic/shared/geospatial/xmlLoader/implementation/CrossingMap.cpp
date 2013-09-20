//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


// crossings_maps_t_pimpl
//

void sim_mob::xml::crossings_maps_t_pimpl::
pre ()
{
	  model.clear();
}

void sim_mob::xml::crossings_maps_t_pimpl::
crossings_map (std::pair<sim_mob::Crossing *, sim_mob::Crossings> crossings_map)
{
  model[crossings_map.first] = crossings_map.second;
//  std::cout << "Crossing " << crossings_map.first << " Added to the map, new size of map = " << model.size() << std::endl;
//
}

std::map<sim_mob::Crossing *, sim_mob::Crossings> sim_mob::xml::crossings_maps_t_pimpl::
post_crossings_maps_t ()
{
//	  std::cout << "Sending Crossing map of size " << model.size() << std::endl;
  return model;
}

// crossings_map_t_pimpl
//

void sim_mob::xml::crossings_map_t_pimpl::
pre ()
{
	  model = sim_mob::Crossings();
}

void sim_mob::xml::crossings_map_t_pimpl::
linkID (unsigned int linkID)
{
//  std::cout << "linkID: " << linkID << std::endl;
  model.link = book.getLink(linkID);
}

void sim_mob::xml::crossings_map_t_pimpl::
crossingID (unsigned int crossingID)
{
//  std::cout << "crossingID: " << crossingID << std::endl;
  model.crossig = book.getCrossing(crossingID);
}

void sim_mob::xml::crossings_map_t_pimpl::
ColorSequence (std::pair<sim_mob::TrafficLightType, std::vector<std::pair<TrafficColor,short> > > ColorSequence)//todo:make reference
{
  model.colorSequence.setColorDuration(ColorSequence.second);
  model.colorSequence.setTrafficLightType(ColorSequence.first);
}

std::pair<sim_mob::Crossing *, sim_mob::Crossings> sim_mob::xml::crossings_map_t_pimpl:://todo:make reference
post_crossings_map_t ()
{
//	std::cout << "Sending a crossing pair with key " << model.crossig << "\n";
  return std::make_pair(model.crossig,model);
}
