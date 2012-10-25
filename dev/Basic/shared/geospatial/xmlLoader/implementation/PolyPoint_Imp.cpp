#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::PolyPoint_t_pimpl::pre ()
{
}

void sim_mob::xml::PolyPoint_t_pimpl::pointID (const ::std::string& pointID)
{
	savedID = pointID;
}

void sim_mob::xml::PolyPoint_t_pimpl::location (sim_mob::Point2D location)
{
	savedPos = location;
}

sim_mob::Point2D sim_mob::xml::PolyPoint_t_pimpl::post_PolyPoint_t ()
{
	//TODO: For now, we simply return the position. May save the ID later.
	return savedPos;
}
