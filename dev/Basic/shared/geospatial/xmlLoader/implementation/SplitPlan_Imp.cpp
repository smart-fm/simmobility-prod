//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;


void sim_mob::xml::SplitPlan_t_pimpl::pre ()
{
	model = sim_mob::SplitPlan();
}

sim_mob::SplitPlan& sim_mob::xml::SplitPlan_t_pimpl::post_SplitPlan_t ()
{
	return model;
}

void sim_mob::xml::SplitPlan_t_pimpl::splitplanID (unsigned int value)
{
	 model.TMP_PlanID = value;
}

void sim_mob::xml::SplitPlan_t_pimpl::cycleLength (unsigned char value)
{
	model.cycleLength =  static_cast<unsigned short> (value) ;
}

void sim_mob::xml::SplitPlan_t_pimpl::offset (unsigned char value)
{
	model.offset =  static_cast<unsigned short> (value);
}

void sim_mob::xml::SplitPlan_t_pimpl::ChoiceSet (std::vector<std::vector<double> >& value)
{
	model.setChoiceSet(value);
}


