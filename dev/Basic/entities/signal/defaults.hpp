
//#ifndef TC_DEFAULTS_
//#define TC_DEFAULTS_
#include<string>
#include<sstream>
#include<iostream>
#include<stdio.h>
//#include<stdio.h>
#include "geospatial/Link.hpp"
#include "geospatial/Crossing.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

#pragma once


namespace sim_mob {
//using namespace std;

enum TrafficColor
{
    Red =1,    			///< Stop, do not go beyond the stop line.
    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
    Green = 3,   		///< Proceed either in the forward, left, or right direction.
    FlashingRed = 4,	///future use
    FlashingAmber = 5,	///future use
    FlashingGreen = 6	///future use
};
namespace {
std::string getColor(size_t id)
{
	std::ostringstream o;
	switch(id)
	{
	case sim_mob::Red:
		return "Red";
	case sim_mob::Amber:
		return "Amber";
	case sim_mob::Green:
		return "Green";
	case sim_mob::FlashingRed:
		return "FlashingRed";
	case sim_mob::FlashingAmber:
		return "FlashingAmber";
	case sim_mob::FlashingGreen:
		return "FlashingGreen";
	default:
		o << id << " Unknown";
		return o.str();
	}
	o << id << " Unknown__";
	return o.str();
}
}

//Private namespace
//TODO: Might want to move this private namespace out of the header file. ~Seth
namespace {
//parameters for calculating next cycle length
const double DSmax = 0.9, DSmed = 0.5, DSmin = 0.3;
const double CLmax = 140, CLmed = 100, CLmin = 60;

//parameters for calculating next Offset
const double CL_low = 70, CL_up = 120;
const double Off_low = 5, Off_up = 26;

const double fixedCL = 60;
}

//Link and crossing of an intersection/traffic signal
struct LinkAndCrossing
{
	LinkAndCrossing(int id_,sim_mob::Link const * link_,sim_mob::Crossing const * crossing_,double angle_):
		id(id_),
		link(link_),
		crossing(crossing_),
		angle(angle_)
	{}
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

