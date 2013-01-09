#include "conf1-pimpl.hpp"

using namespace sim_mob::conf;

#include <stdexcept>
#include <iostream>

#include "util/ReactionTimeDistributions.hpp"

using sim_mob::ReactionTimeDist;
using std::string;
using std::pair;

void sim_mob::conf::distribution_pimpl::pre ()
{
}

pair<string, ReactionTimeDist*> sim_mob::conf::distribution_pimpl::post_distribution ()
{
	if (distType=="normal") {
		return std::make_pair(distID, new NormalReactionTimeDist(distMean, distStdev));
	} else if (distType=="lognormal") {
		return std::make_pair(distID, new LognormalReactionTimeDist(distMean, distStdev));
	} else {
		throw std::runtime_error("Distributions other than normal/lognormal not currently supported.");
	}
}

void sim_mob::conf::distribution_pimpl::id (const ::std::string& id)
{
	distID = id;
}

void sim_mob::conf::distribution_pimpl::type (const ::std::string& type)
{
	distType = type;
}

void sim_mob::conf::distribution_pimpl::mean (int mean)
{
	distMean = mean;
}

void sim_mob::conf::distribution_pimpl::stdev (int stdev)
{
	distStdev = stdev;
}

