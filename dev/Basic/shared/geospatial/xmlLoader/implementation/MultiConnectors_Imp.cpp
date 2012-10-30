#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::Multi_Connectors_t_pimpl::pre ()
{
	model.clear();
}

std::map<unsigned long,std::set<std::pair<unsigned long,unsigned long> > > sim_mob::xml::Multi_Connectors_t_pimpl::post_Multi_Connectors_t ()
{
	return model;
}

void sim_mob::xml::Multi_Connectors_t_pimpl::MultiConnectors (const std::pair<unsigned long,std::set<std::pair<unsigned long,unsigned long> > >& MultiConnectors)
{
	//tmp_cnn_cnt += MultiConnectors.second.size();
	model[MultiConnectors.first] = MultiConnectors.second;
}

