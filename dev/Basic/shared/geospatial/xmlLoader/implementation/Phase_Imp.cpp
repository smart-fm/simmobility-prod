#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Phase_t_pimpl::pre ()
{
	model = sim_mob::Phase();
}

sim_mob::Phase sim_mob::xml::Phase_t_pimpl::post_Phase_t ()
{
	return model;
}

void sim_mob::xml::Phase_t_pimpl::phaseID (unsigned char value)
{
	model.name = value; //just for now. we may have to delete id
	//std::cout << "phaseID: " << static_cast<unsigned short> (value) << std::endl;
}

void sim_mob::xml::Phase_t_pimpl::name (const ::std::string& value)
{
	model.name = value;
	//std::cout << "name: " <<value << std::endl;
}

void sim_mob::xml::Phase_t_pimpl::links_map (std::multimap<sim_mob::Link*,sim_mob::linkToLink> value)
{
	model.links_map_ = value;
}

void sim_mob::xml::Phase_t_pimpl::crossings_maps (std::map<sim_mob::Crossing *, sim_mob::Crossings> value)
{
	model.crossings_map_ =  value;
}
