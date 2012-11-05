#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::SubTrips_t_pimpl::pre ()
{
	model.clear();
}

std::vector<sim_mob::SubTrip> sim_mob::xml::SubTrips_t_pimpl::post_SubTrips_t ()
{
	return model;
}

void sim_mob::xml::SubTrips_t_pimpl::subTrip (sim_mob::SubTrip value)
{
	model.push_back(value);
}



