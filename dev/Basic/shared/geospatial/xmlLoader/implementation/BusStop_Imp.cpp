#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::BusStop_t_pimpl::pre ()
{
	model = sim_mob::BusStop();
	//bs_info = BusStopInfo();
	//bs_info.busStop = bs;
}

std::pair<unsigned long,sim_mob::BusStop*> sim_mob::xml::BusStop_t_pimpl::post_BusStop_t ()
{
	sim_mob::BusStop* res = new sim_mob::BusStop(model);

	//Retrieve a temporary.
	std::pair<unsigned long, sim_mob::RoadItem*> temp = RoadItem_t_pimpl::post_RoadItem_t();
	res->id = temp.second->getRoadItemID();
	res->start = temp.second->getStart();
	res->end = temp.second->getEnd();
	delete temp.second;

	return std::make_pair(temp.first, res);
}

void sim_mob::xml::BusStop_t_pimpl::xPos (double value)
{
	model.xPos = value;
}

void sim_mob::xml::BusStop_t_pimpl::yPos (double value)
{
	model.yPos = value;
}

void sim_mob::xml::BusStop_t_pimpl::lane_location (unsigned long long value)
{
	//bs_info.lane_location = value;
}

void sim_mob::xml::BusStop_t_pimpl::is_terminal (bool value)
{
	model.is_terminal = value;
}

void sim_mob::xml::BusStop_t_pimpl::is_bay (bool value)
{
	model.is_bay = value;
}

void sim_mob::xml::BusStop_t_pimpl::has_shelter (bool value)
{
	model.has_shelter = value;
}

void sim_mob::xml::BusStop_t_pimpl::busCapacityAsLength (unsigned int value)
{
	model.busCapacityAsLength = value;
}

void sim_mob::xml::BusStop_t_pimpl::busstopno (const ::std::string& value)
{
	model.busstopno_ = value;
}


