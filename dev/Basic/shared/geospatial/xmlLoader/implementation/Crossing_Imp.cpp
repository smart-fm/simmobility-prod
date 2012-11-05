#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::crossing_t_pimpl::pre ()
{
	model = sim_mob::Crossing();
}


std::pair<unsigned long,sim_mob::Crossing*> sim_mob::xml::crossing_t_pimpl::post_crossing_t ()
{
	sim_mob::Crossing* res = new sim_mob::Crossing(model);

	//Retrieve a temporary.
	std::pair<unsigned long, sim_mob::RoadItem*> temp = RoadItem_t_pimpl::post_RoadItem_t();
	res->id = temp.second->getRoadItemID();
	res->start = temp.second->getStart();
	res->end   = temp.second->getEnd();
	delete temp.second;

	return std::make_pair(temp.first, res);
}

void sim_mob::xml::crossing_t_pimpl::nearLine (std::pair<sim_mob::Point2D,sim_mob::Point2D> value)
{
	model.nearLine = value;
}

void sim_mob::xml::crossing_t_pimpl::farLine (std::pair<sim_mob::Point2D,sim_mob::Point2D> value)
{
	model.farLine = value;
}

