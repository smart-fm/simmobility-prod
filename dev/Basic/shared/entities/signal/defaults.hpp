#pragma once
#include<cstdio>
#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>


namespace sim_mob {

//Forward declarations
class Crossing;
class Link;


enum TrafficColor
{
    Red =1,    			///< Stop, do not go beyond the stop line.
    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
    Green = 3,   		///< Proceed either in the forward, left, or right direction.
    FlashingRed = 4,	///future use
    FlashingAmber = 5,	///future use
    FlashingGreen = 6	///future use
};

//Link and crossing of an intersection/traffic signal
struct LinkAndCrossing
{
	LinkAndCrossing(int id_,sim_mob::Link const * link_,sim_mob::Crossing const * crossing_,double angle_);
	size_t id;         //index for backward compatibility (setupindexMaps()
	double angle;         //index for backward compatibility (setupindexMaps()
	sim_mob::Link const * link;
	sim_mob::Crossing const * crossing;
};

/*
 * obtaining link and -its corresponding- crossong information from node variable(as amember of signal)
 * is time consuming, especially when it needs to be repeated for every signal every - number of- ticks
 * therefore this container is filled up initially to save a huge amount of processing time in return for
 * a small extra storage
 */
typedef boost::multi_index_container<
		LinkAndCrossing, boost::multi_index::indexed_by<
		boost::multi_index::random_access<>															//0
    ,boost::multi_index::ordered_unique<boost::multi_index::member<LinkAndCrossing, size_t , &LinkAndCrossing::id> >//1
	,boost::multi_index::ordered_unique<boost::multi_index::member<LinkAndCrossing, sim_mob::Link const * , &LinkAndCrossing::link> >//2
	,boost::multi_index::ordered_non_unique<boost::multi_index::member<LinkAndCrossing, double , &LinkAndCrossing::angle> >//3
	,boost::multi_index::ordered_non_unique<boost::multi_index::member<LinkAndCrossing, sim_mob::Crossing const * , &LinkAndCrossing::crossing> >//4
   >
> LinkAndCrossingC;//Link and Crossing Container(multi index)
typedef boost::multi_index::nth_index<LinkAndCrossingC, 2>::type LinkAndCrossingByLink;
typedef boost::multi_index::nth_index<LinkAndCrossingC, 3>::type LinkAndCrossingByAngle;
typedef boost::multi_index::nth_index<LinkAndCrossingC, 4>::type LinkAndCrossingByCrossing;

typedef LinkAndCrossingByAngle::reverse_iterator LinkAndCrossingIterator;
typedef LinkAndCrossingByCrossing::iterator SignalCrossingIterator;


}

