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
	void Phase::update(double currentCycleTimer) const
	{
		//todo: avoid color updation if already end of cycle
		double lapse = currentCycleTimer - phaseOffset;
		//update each link-link signal's color
		links_map_iterator link_it = links_map_.begin();
		for(;link_it != links_map_.end() ; link_it++)
		{
			(*link_it).second.currColor = (*link_it).second.colorSequence.computeColor(lapse);
		}
		//update each crossing signal's color
		//common sense says there is only one crossing per link, but I kept a container for it just in case
		crossings_map_iterator crossing_it = crossings_map_.begin();
		for(;crossing_it != crossings_map_.end() ; crossing_it++)
		{
			(*crossing_it).second.currColor = (*crossing_it).second.colorSequence.computeColor(lapse);
		}

	}
//	assumption : total green time = the whole duration in the color sequence except red!
//	formula : for a given phase, total_g is maximum (G+FG+...+A - All_red, red...) of the linkFrom(s)
	/*todo a container should be created(probabely at splitPlan level and mapped to "choiceSet" container)
	to store total_g for all phases once the phase is met, so that this function is not called for the second time
	if any plan is selected for the second time.*/
	double Phase::computeTotalG ()const
	{
		double green, max_green;
		links_map_const_iterator link_it = links_map_.begin();
		for(max_green = 0; link_it != links_map_.end() ; link_it++)
		{
			std::vector< std::pair<TrafficColor,std::size_t> >::const_iterator  color_it = (*link_it).second.colorSequence.ColorDuration.begin();
			for(green = 0; color_it != (*link_it).second.colorSequence.ColorDuration.end(); color_it++)
			{
				if((*color_it).first != Red)
				green += (*color_it).second;
			}
			if(max_green < green) max_green = green;//formula :)
		}
		return max_green;
	}
//	 links_map_equal_range  Phase::getLinkTos(sim_mob::Link *LinkFrom)
//		{
//			return links_map_.equal_range(LinkFrom);
//		}
	links_map_equal_range Phase::getLinkTos(sim_mob::Link  *const LinkFrom)const//dont worry about constantization!! :) links_map_equal_range is using a constant iterator
	{
		links_map_equal_range ppp = links_map_.equal_range(LinkFrom);
		return ppp;
	}
}//namespace
