#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

using std::string;
using std::pair;

void sim_mob::conf::granularities_pimpl::pre ()
{
}

std::pair<sim_mob::Granularity, sim_mob::Granularity> sim_mob::conf::granularities_pimpl::post_granularities ()
{
	return std::make_pair(agentGran, signalGran);
}

void sim_mob::conf::granularities_pimpl::agent (const sim_mob::Granularity& value)
{
	agentGran = value;
}

void sim_mob::conf::granularities_pimpl::signal (const sim_mob::Granularity& value)
{
	signalGran = value;
}









