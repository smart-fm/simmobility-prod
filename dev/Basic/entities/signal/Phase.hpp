#pragma once

#include "Color.hpp"
#include<vector>
#include<string>
#include <map>
#include<util/LangHelpers.hpp>
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
class SplitPlan;


//////////////some bundling ///////////////////

//////////////////// Links ///////////////////////////////////////////////////////////////////////////////////////

struct ll
{
	ll(sim_mob::Link *linkto = nullptr):LinkTo(linkto) {
			colorSequence.addColorDuration(Green,0);
			colorSequence.addColorDuration(Amber,3);//a portion of the total time of the phase length is taken by amber
			colorSequence.addColorDuration(Red,1);//All red moment ususally takes 1 second

		currColor = sim_mob::Red;
	}

	sim_mob::Link *LinkTo;
	mutable ColorSequence colorSequence;
	mutable TrafficColor currColor;
};
typedef ll linkToLink;

typedef std::multimap</*linkFrom*/sim_mob::Link *, sim_mob::linkToLink> links_map; //mapping of LinkFrom to linkToLink{which is LinkTo,colorSequence,currColor}

////////////////////crossings////////////////////////////////////////////////////////////////////////////////////
struct crossings
{
	crossings(sim_mob::Link *link_,sim_mob::Crossing *crossig_):link(link_),crossig(crossig_){
		colorSequence.addColorDuration(Green,0);
		colorSequence.addColorDuration(FlashingGreen,0);
		colorSequence.addColorDuration(Red,1);//All red moment ususally takes 1 second
		currColor = sim_mob::Red;
	}
	sim_mob::Link * link;//this is extra but keep it until you are sure!
	sim_mob::Crossing *crossig;//same as the key in the corresponding multimap(yes yes: it is redundant)
	ColorSequence colorSequence;//color and duration
	TrafficColor currColor;
} ;

typedef struct crossings Crossings;


////////////////////////////////phase communication to plan///////////////////////////////////////////////
typedef struct
{
	bool endOfPhase;
	links_map links;//the required field in this container are:link to, link from, current color(all but ColorSequence)
	Crossings crossings;//the required field in this container are:link , crossing, current color(all but ColorSequence)
}PhaseUpdateResult;
/////////////////////////////////////////////////////////////////////////////
typedef std::multimap<sim_mob::Crossing *, sim_mob::Crossings> crossings_map;

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
	typedef links_map::iterator links_map_iterator;
	typedef links_map::const_iterator links_map_const_iterator;
	typedef std::pair<links_map_const_iterator, links_map_const_iterator> links_map_equal_range;

	typedef crossings_map::iterator crossings_map_iterator;

	Phase(){}
	Phase(std::string name_, sim_mob::SplitPlan *parent = nullptr):name(name_),parentPlan(parent){}

	void setPercentage(double p)
	{
		percentage = p;
	}
	void setPhaseOffset(double p)
	{
		phaseOffset = p;
	}
	links_map_equal_range LinkFrom_Range(sim_mob::Link *LinkFrom)
	{
		return links_map_.equal_range(LinkFrom);
	}
	links_map_iterator LinkFrom_begin() const
	{
		return links_map_.begin();
	}
	links_map_iterator LinkFrom_end() const
	{
		return links_map_.end();
	}
	links_map_equal_range getLinkTos(sim_mob::Link  *const LinkFrom)const//dont worry about constantization!! :) links_map_equal_range is using a constant iterator
	{
		return links_map_.equal_range(LinkFrom);
	}
	void addLinkMaping(sim_mob::Link * lf, sim_mob::linkToLink ll)const {

		links_map_.insert(std::pair<sim_mob::Link *, sim_mob::linkToLink>(lf,ll));
	}
	void addCrossingMapping(sim_mob::Link *,sim_mob::Crossing *, ColorSequence);
	void addCrossingMapping(sim_mob::Link *,sim_mob::Crossing *);
	//add crossing to any link of this node which is not involved in this phase
	void addDefaultCrossings();
	const links_map & getLinkMaps();
//	links_map_equal_range  getLinkTos(sim_mob::Link *LinkFrom) ;
	void updatePhaseParams(double phaseOffset_, double percentage_);
	/* Used in computing DS for split plan selection
	 * the argument is the output
	 * */
	void update(double lapse) const;
	double computeTotalG() const;//total green time
	const std::string & getPhaseName() { return name;}
	std::string createStringRepresentation() const;
	void initialize();
	void calculateGreen();
	void calculatePhaseLength();
	void printColorDuration() ;
	void printPhaseColors(double currCycleTimer) const;
	const std::string & getName() const;

	const std::string name; //we can assign a name to a phase for ease of identification
private:
	unsigned int TMP_PhaseID;
	std::size_t startPecentage;
	mutable std::size_t percentage;
	double phaseOffset; //the amount of time from cycle start until this phase start
	double phaseLength;
	double total_g;

	//The links that will get a green light at this phase
	mutable sim_mob::links_map links_map_;
	//The crossings that will get a green light at this phase
	mutable sim_mob::crossings_map crossings_map_;

	sim_mob::SplitPlan *parentPlan;

	friend class SplitPlan;
	friend class Signal;
};
}//namespace
