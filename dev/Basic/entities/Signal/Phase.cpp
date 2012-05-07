#include "Phase.hpp"
#include <vector>

namespace sim_mob
{
	void Phase::updatePhaseParams()
	{
		phaseOffset = startPecentage * cycleLength / 100;
		phaseLength = percentage * cycleLength / 100;
		total_g = computeTotalG();
	}
	/*
	 * Functionalities of this function will be listed here as they emerge:
	 * 1- update the color of the link_maps
	 */
	void Phase::update(double currentCycleTimer)
	{
		//todo: avoid color updation if already end of cycle
		double lapse = currentCycleTimer - phaseOffset;
		//update each link-link signal's color
		std::vector<sim_mob::links_map>::iterator it = links_map_.begin();

		for(; it != links_map_.end(); it++)
		{
			ColorSequence cs = (*it).colorSequence;
			(*it).currColor = cs.computeColor(lapse);
		}
	}
//	assumption : total green time = anytime in the color sequence except red!
//	formula : for a given phase, total_g is maximum (G+FG+...+A - All_red, red...) of the linkFrom(s)
	/*todo a container should be created(probabely at splitPlan level and mapped to "percentage" container)
	to store total_g for all phases once the phase is met, so that this function is not called for the second time
	if any plan is selected for the second time.*/
	double Phase::computeTotalG()
	{
		double green, max_green;
		std::vector<sim_mob::links_map>::iterator links_map_it = links_map_.begin();
		for(max_green = 0; links_map_it != links_map_.end(); links_map_it++)
		{
			std::vector< std::pair<TrafficColor,std::size_t> >::const_iterator  color_it = (*links_map_it).colorSequence.ColorDuration.begin();
			for(green = 0; color_it != (*links_map_it).colorSequence.ColorDuration.end(); color_it++)
			{
				if((*color_it).first != Red)
				green += (*color_it).second;
			}
			if(max_green < green) max_green = green;//formula :)
		}
		return max_green;
	}

}//namespace
