#pragma once

#include "Color.hpp"
#include<vector>
#include<string>
#include <map>
//#include <boost/multi_index_container.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
namespace sim_mob
{
//Forward declarations
class Crossing;
class Link;

//////////////some bundling ///////////////////
using namespace ::boost;
using namespace ::boost::multi_index;
//////////////////// Links ///////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	sim_mob::Link *LinkTo;
	ColorSequence colorSequence;
	TrafficColor currColor;
} linkToLink;

typedef std::multimap</*linkFrom*/sim_mob::Link *, sim_mob::linkToLink> links_map; //mapping of LinkFrom to linkToLink{which is LinkTo,colorSequence,currColor}
typedef links_map::iterator links_map_iterator;
typedef links_map::const_iterator links_map_const_iterator;
typedef std::pair<links_map_const_iterator, links_map_const_iterator> links_map_equal_range;
////////////////////crossings////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	sim_mob::Link * link;
	sim_mob::Crossing *crossig;//same as the key in the corresponding multimap(yes yes: it is redundant)
	ColorSequence colorSequence;//color and duration
	TrafficColor currColor;

} Crossings;

///////////////////////////////////////////////////////////////////////////////
typedef std::multimap<sim_mob::Crossing *, sim_mob::Crossings> crossings_map;
typedef crossings_map::iterator crossings_map_iterator;
typedef crossings_map::const_iterator crossings_map_const_iterator;
typedef std::pair<crossings_map_const_iterator, crossings_map_const_iterator> crossings_map_equal_range;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * in adaptive signal control, the cycle length, phase percentage and offset are likely to change
 * but the color sequence of a traffic signal, amber time etc are assumed to be intact for many years.
 */
class Phase
{
public:

	Phase(double CycleLenght,std::size_t start, std::size_t percent): cycleLength(CycleLenght),startPecentage(start),percentage(percent){
		updatePhaseParams();
	};
	Phase(){}

	void setPercentage(std::size_t p)
	{
		percentage = p;
	}
	void setCycleLength(std::size_t c)
	{
		cycleLength = c;
	}
	links_map_equal_range LinkFrom_Range(sim_mob::Link *LinkFrom)
	{
		return links_map_.equal_range(LinkFrom);
	}
	links_map_iterator LinkFrom_begin()
	{
		return links_map_.begin();
	}
	links_map_iterator LinkFrom_end()
	{
		return links_map_.end();
	}
	const links_map & getLinkMaps();
//	links_map_equal_range  getLinkTos(sim_mob::Link *LinkFrom) ;
	links_map_equal_range getLinkTos(sim_mob::Link *const LinkFrom)const ;
	void updatePhaseParams();
	/* Used in computing DS for split plan selection
	 * the argument is the output
	 * */
	void update(double lapse);
	double computeTotalG();//total green time
	std::string name; //we can assign a name to a phase for ease of identification
private:
	std::size_t startPecentage;
	std::size_t percentage;
	double cycleLength;
	double phaseOffset; //the amount of time from cycle start until this phase start
	double phaseLength;
	double total_g;

	//The links that will get a green light at this phase
	sim_mob::links_map links_map_;
	//The crossings that will get a green light at this phase
	sim_mob::crossings_map crossings_map_;

	friend class SplitPlan;
	friend class Signal;
};
}//namespace
