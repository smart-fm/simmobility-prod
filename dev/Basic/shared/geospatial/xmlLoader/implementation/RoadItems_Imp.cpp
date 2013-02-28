#include "geo10-pimpl.hpp"

#include <map>

#include "geospatial/RoadItem.hpp"
#include "metrics/Length.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::RoadItems_t_pimpl::pre ()
{
	model.clear();
}

std::map<sim_mob::centimeter_t,const sim_mob::RoadItem*> sim_mob::xml::RoadItems_t_pimpl::post_RoadItems_t ()
{
	return model;
}

void sim_mob::xml::RoadItems_t_pimpl::BusStop (std::pair<unsigned long,sim_mob::BusStop*> value)
{
	model[value.first] = value.second;
}

void sim_mob::xml::RoadItems_t_pimpl::ERP_Gantry ()
{
}

void sim_mob::xml::RoadItems_t_pimpl::Crossing (std::pair<unsigned long,sim_mob::Crossing*> value)
{
	model[value.first] = value.second;
	book.addCrossing(value.second);
}

void sim_mob::xml::RoadItems_t_pimpl::RoadBump ()
{
}


