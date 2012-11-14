#include "defaults.hpp"

#include<string>
#include<sstream>
#include<iostream>

#include "geospatial/Link.hpp"
#include "geospatial/Crossing.hpp"
#include "Phase.hpp"

namespace sim_mob {

sim_mob::LinkAndCrossing::LinkAndCrossing(int id_,sim_mob::Link const * link_,sim_mob::Crossing const * crossing_,double angle_):
			id(id_),
			link(link_),
			crossing(crossing_),
			angle(angle_)
{}
sim_mob::LinkAndCrossing::LinkAndCrossing(){
	id = -1;
	link = 0;
	crossing = 0;
	angle = -1;
}

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

