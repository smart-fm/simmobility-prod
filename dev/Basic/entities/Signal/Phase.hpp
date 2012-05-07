#pragma once

#include "Color.hpp"
#include<vector>
#include<string>
namespace sim_mob
{
//Forward declarations
class Crossing;
class Link;

//////////////some bundling ///////////////////
typedef struct
{
	sim_mob::Link *LinkTo;
	sim_mob::Link *LinkFrom;
	ColorSequence colorSequence;
	TrafficColor currColor;
} links_map;

typedef struct
{
	sim_mob::Crossing crossig();
//	struct VehicleTrafficColors colorSequence;
} crossings_map;

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
	void setPercentage(std::size_t p)
	{
		percentage = p;
	}
	void setCycleLength(std::size_t c)
	{
		cycleLength = c;
	}
	std::vector<sim_mob::links_map> & LinksMap()
	{
		return links_map_;
	}
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
	std::vector<sim_mob::links_map> links_map_;
	//The crossings that will get a green light at this phase
	std::vector<sim_mob::crossings_map> crossings_map_;

	friend class SplitPlan;
	friend class Signal;
};
}//namespace
