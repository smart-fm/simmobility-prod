/*
 * ReactionTimeDistributions.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: lzm
 */

#include "ReactionTimeDistributions.hpp"

using namespace sim_mob;

GenType ReactionTimeDistributions::gt;
NormalDis* ReactionTimeDistributions::dis;
RNG* ReactionTimeDistributions::rng_noraml;
ReactionTimeDistributions ReactionTimeDistributions::instance_;

size_t sim_mob::ReactionTimeDistributions::normal()
{
	return rng_noraml->operator ()();
}

sim_mob::ReactionTimeDistributions::~ReactionTimeDistributions()
{
	// TODO Auto-generated destructor stub
}

