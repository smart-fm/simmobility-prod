//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cstdio>
#include <iostream>

#include <set>



namespace sim_mob {

//Forward declarations
class Crossing;
class Link;


/**
 * Link and crossing of an intersection/traffic signal
 */
struct LinkAndCrossing
{
	LinkAndCrossing(int id_,sim_mob::Link const * link_,sim_mob::Crossing const * crossing_,double angle_);
	LinkAndCrossing();
	///index for backward compatibility (setupindexMaps()
	size_t id;
	double angle;
	sim_mob::Link const * link;
	sim_mob::Crossing const * crossing;
	size_t getId() const;
};

struct LinkAndCrossingComparison {
	bool operator() (const LinkAndCrossing&a, const LinkAndCrossing&b);
};

/**
 * obtaining link and -its corresponding- crossong information from node variable(as amember of signal)
 * is time consuming, especially when it needs to be repeated for every signal every - number of- ticks
 * therefore this container is filled up initially to save a huge amount of processing time in return for
 * a small extra storage
 */

typedef std::set<LinkAndCrossing,LinkAndCrossingComparison >LinkAndCrossingC;
}

