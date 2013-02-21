#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::simulation_pimpl::pre ()
{
}

void sim_mob::conf::simulation_pimpl::post_simulation ()
{
}

void sim_mob::conf::simulation_pimpl::base_granularity (const sim_mob::Granularity& value)
{
	config->simulation().baseGranularity = value;
}

void sim_mob::conf::simulation_pimpl::total_runtime (const sim_mob::Granularity& value)
{
	config->simulation().totalRuntime = value;
}

void sim_mob::conf::simulation_pimpl::total_warmup (const sim_mob::Granularity& value)
{
	config->simulation().totalWarmup = value;
}

void sim_mob::conf::simulation_pimpl::start_time (const sim_mob::DailyTime& value)
{
	config->simulation().startTime = value;
}

void sim_mob::conf::simulation_pimpl::granularities (const std::pair<sim_mob::Granularity, sim_mob::Granularity>& value)
{
	config->simulation().agentGranularity = value.first;
	config->simulation().signalGranularity = value.second;
}

void sim_mob::conf::simulation_pimpl::react_times ()
{
	//This is done within the react_times class (since there's three properties to return).
}

void sim_mob::conf::simulation_pimpl::geospatial ()
{
}

void sim_mob::conf::simulation_pimpl::agents ()
{
}




