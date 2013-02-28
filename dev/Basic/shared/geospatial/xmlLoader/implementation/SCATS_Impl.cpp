#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;

void sim_mob::xml::SCATS_t_pimpl::pre (){
	model.SplitPlan = sim_mob::SplitPlan();
	model.signalTimingMode = -1;
}
void sim_mob::xml::SCATS_t_pimpl::signalTimingMode (int value)
{
	model.signalTimingMode = value;
}

void sim_mob::xml::SCATS_t_pimpl::SplitPlan (sim_mob::SplitPlan value)
{
	model.SplitPlan = value;
}

sim_mob::xml::helper::SignalHelper::SCATS_Info sim_mob::xml::SCATS_t_pimpl::post_SCATS_t ()
{
	return model;
}
