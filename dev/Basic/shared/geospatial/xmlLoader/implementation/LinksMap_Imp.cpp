//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::links_map_t_pimpl::pre ()
{
	  model.first = 0;
	  ll.LinkTo = 0;
	  ll.RS_From = 0;
	  ll.RS_To = 0;
	  LinkFrom_ = 0;
}

std::pair<sim_mob::Link*,sim_mob::linkToLink>& sim_mob::xml::links_map_t_pimpl::post_links_map_t ()
{
//	return std::pair<sim_mob::Link*,sim_mob::linkToLink>(nullptr, sim_mob::linkToLink(nullptr))	;
    model.second = ll;
    return model;
}

void sim_mob::xml::links_map_t_pimpl::LinkFrom (unsigned int value)
{
	model.first = book.getLink(value);
}

void sim_mob::xml::links_map_t_pimpl::LinkTo (unsigned int value)
{
	ll.LinkTo = book.getLink(value);
}

void sim_mob::xml::links_map_t_pimpl::SegmentFrom (unsigned int value)
{
	ll.RS_From = book.getSegment(value);
}

void sim_mob::xml::links_map_t_pimpl::SegmentTo (unsigned int value)
{
	ll.RS_To = book.getSegment(value);
}

void sim_mob::xml::links_map_t_pimpl::ColorSequence (std::pair<sim_mob::TrafficLightType,std::vector<std::pair<TrafficColor,int> > >& value)
{
    ll.colorSequence.setTrafficLightType(value.first);
    ll.colorSequence.setColorDuration(value.second);
}


