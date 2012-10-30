#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::roundabout_t_pimpl::pre ()
{
}

sim_mob::MultiNode* sim_mob::xml::roundabout_t_pimpl::post_roundabout_t ()
{
	//sim_mob::Node* v (post_Node_t ());
	return nullptr;
}

void sim_mob::xml::roundabout_t_pimpl::roadSegmentsAt (std::set<unsigned long> value)
{
}

void sim_mob::xml::roundabout_t_pimpl::Connectors (const std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >& value)
{
}

void sim_mob::xml::roundabout_t_pimpl::ChunkLengths ()
{
}

void sim_mob::xml::roundabout_t_pimpl::Offsets ()
{
}

void sim_mob::xml::roundabout_t_pimpl::Separators ()
{
}

void sim_mob::xml::roundabout_t_pimpl::addDominantLane ()
{
}

void sim_mob::xml::roundabout_t_pimpl::roundaboutDominantIslands (float value)
{
	//std::cout << "roundaboutDominantIslands: " <<value << std::endl;
}

void sim_mob::xml::roundabout_t_pimpl::roundaboutNumberOfLanes (int value)
{
	//std::cout << "roundaboutNumberOfLanes: " <<value << std::endl;
}

void sim_mob::xml::roundabout_t_pimpl::entranceAngles ()
{
}


