#include "Phase.hpp"
#include "SplitPlan.hpp"
#include <vector>
#include <sstream>
namespace sim_mob
{
	void Phase::updatePhaseParams(double phaseOffset_, double percentage_)
	{
		phaseOffset = phaseOffset_;
		phaseLength = percentage_;
	}
	/*
	 * Functionalities of this function will be listed here as they emerge:
	 * 1- update the color of the link_maps
	 */
	void Phase::update(double currentCycleTimer) const
	{
		std::ostringstream o;

		double lapse = currentCycleTimer - phaseOffset;
		//todo: avoid color updation if already end of cycle--update the following line just did that:
		//when time lapse i= zero it means either
		//1-signal update was called at time zero(which is not likely)
		//or
		//2-you are at the beginning of this phase so the rest(computeColor) will be ok
		if(lapse < 0)
			{
			std::cout << "Phase " << name << " update: " << "RETURNING lapse( " << lapse << ")=currentCycleTimer(" << currentCycleTimer << ") - phaseOffset(" << phaseOffset << ")" << std::endl;
				return;
			}
		//update each link-link signal's color
		links_map_iterator link_it = links_map_.begin();
		for(;link_it != links_map_.end() ; link_it++)
		{
			/*
			 * if no time lapse, don't change the color
			 * this is especialy important when a full cycle has passed and current cycle timer is zero!
			 * then instead of moving to the first phase , again the last phase may reset to green
			 */
			//
//			std::cout << "Phase " << name << " update: " << "lapse( " << lapse << ")=currentCycleTimer(" << currentCycleTimer << ") - phaseOffset(" << phaseOffset << ")" << std::endl;
			(*link_it).second.currColor = (*link_it).second.colorSequence.computeColor(lapse);
			if(((*link_it).second.currColor > sim_mob::FlashingGreen) || ((*link_it).second.currColor < sim_mob::Red))
			{
				o << "currentCycleTimer :" << currentCycleTimer << "  phaseOffset :" << phaseOffset  << "--->lapse :" << lapse << "\n creates out of range color";
				throw std::runtime_error(o.str());
			}
		}
		//update each crossing signal's color
		//common sense says there is only one crossing per link, but I kept a container for it just in case
		crossings_map_iterator crossing_it = crossings_map_.begin();
		for(;crossing_it != crossings_map_.end() ; crossing_it++)
		{
			if(lapse > 0) (*crossing_it).second.currColor = (*crossing_it).second.colorSequence.computeColor(lapse);
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
		for (green = 0;	color_it != (*link_it).second.colorSequence.ColorDuration.end(); color_it++) {
			if ((*color_it).first != Red) {
				green += (*color_it).second;
			}
		}
			if(max_green < green) max_green = green;//formula :)
		}
		return max_green * 1000;
	}
	void Phase::addCrossingMapping(sim_mob::Link * link,sim_mob::Crossing * crossing, ColorSequence cs)
	{
		sim_mob::Crossings crossing_(link,crossing);
		crossing_.colorSequence.clear();
		crossing_.colorSequence = cs;
		crossings_map_.insert(std::make_pair(crossing,crossing_));
	}
	void Phase::addCrossingMapping(sim_mob::Link * link,sim_mob::Crossing * crossing)
	{
		Crossings crossing_(link,crossing);
		crossings_map_.insert(std::make_pair(crossing,crossing_));
	}
	void Phase::addDefaultCrossings()
	{

	}

	sim_mob::RoadSegment * Phase::findRoadSegment(sim_mob::Link * link, sim_mob::MultiNode * node) const
	{
		sim_mob::RoadSegment *rs = 0;
		std::set<sim_mob::RoadSegment*>::iterator  itrs = (*link).getUniqueSegments().begin();
		for(;itrs !=(*link).getUniqueSegments().end(); itrs++)
		{
			if(node->canFindRoadSegment(*itrs))
			{
				rs = *itrs;
				break;
			}
		}

		return rs;
	}
	std::string Phase::createStringRepresentation() const
	{
		std::ostringstream output;
		if(links_map_.size() == 0 && crossings_map_.size() == 0) return 0;
		output << "\"Phase_" << name << "\"{";
		int i = 0;
		if(links_map_.size())
		{
			output << "\"Link\"{";
			links_map_iterator it = links_map_.begin();
			while(it != links_map_.end())
			{

				output << (*it).first << ":"; //linkFrom
				output << (*it).second.LinkTo;
				it++;
				if(it != links_map_.end())
					output << ",";

			}
			output << "}";
//			std::cout <<  output.str();
		}


		if(crossings_map_.size())
		{
			output << "\"Crossing\"{";
			crossings_map_iterator it = crossings_map_.begin();
			while(it != crossings_map_.end())
			{
				output << (*it).first ; //crossing *
				it++;
				if(it != crossings_map_.end())
					output << ",";

			}
			output << "}";
		}

		output << "}";
		return output.str();
	}
void Phase::initialize(){
	calculatePhaseLength();
	calculateGreen();
//	printColorDuration();
	std::cout << "phase: " << name << "   PhaseLength: " << phaseLength << "   offset: " << phaseOffset << std::endl;
}
void Phase::printColorDuration()
{
	for(links_map_iterator it = links_map_.begin()  ; it != links_map_.end(); it++)
	{
		ColorSequence cs = it->second.colorSequence;
		std::vector< std::pair<TrafficColor,std::size_t> > & cd = cs.getColorDuration();
		std::vector< std::pair<TrafficColor,std::size_t> >::iterator it_color = cd.begin();
		int greenIndex=-1, tempgreenIndex = -1;
		for(; it_color != cd.end(); it_color++)
		{
			std::cout << "	color id(" <<  sim_mob::getColor(it_color->first) << ") : " << it_color->second <<  std::endl;
		}
		std::cout << "----------------------------------------------------------------" << std::endl;
	}
}

void Phase::calculatePhaseLength(){
	phaseLength = parentPlan->getCycleLength() * percentage /100;

}

//amber, flashing green, red are fixed but green time is calculated by cycle length and percentage given to that phase
void Phase::calculateGreen(){
	/*
	 * here is the drill:
	 * 1.what is the amount of time that is assigned to this phase,(phaseLength might be already calculated)
	 * 2.find out how long the colors other than green will take
	 * 3.subtract them
	 * what is the output? yes, it is the green time. yes yes, i know! you are a Genuis!
	 */

	for(links_map_iterator it = links_map_.begin()  ; it != links_map_.end(); it++)
	{
		//1.what is the amount of time that is assigned to this phase
//		phaseLength is a member
		//2.find out how long the colors other than green will take
		ColorSequence & cs = it->second.colorSequence;
		std::vector< std::pair<TrafficColor,std::size_t> > & cd = cs.getColorDuration();
		std::vector< std::pair<TrafficColor,std::size_t> >::iterator it_color = cd.begin();
		size_t other_than_green = 0;
		int greenIndex=-1, tempgreenIndex = -1;
		for(int tempgreenIndex = 0; it_color != cd.end(); it_color++)
		{
			if(it_color->first != sim_mob::Green)
			{
				other_than_green += it_color->second;
			}
			else
				greenIndex = tempgreenIndex;//we need to know the location of green, right after this loop ends

			tempgreenIndex ++;
		}
		//3.subtract(the genius part)
		if(greenIndex > -1)
		{
			cs.getColorDuration().at(greenIndex).second = phaseLength - other_than_green;
		}
	}
}

void Phase::printPhaseColors(double currCycleTimer) const
{
	for(links_map_iterator it = links_map_.begin()  ; it != links_map_.end(); it++)
	{
//		std::cout << name << " (" << (*it).first << " : " << (*it).second.LinkTo << ") ::" << sim_mob::getColor((*it).second.currColor) << "  currCycleTimer: " << currCycleTimer < std::endl;
		std::cout << name << "(currCycleTimer: " << currCycleTimer<< " , phaseLength: " <<  phaseLength << ") ::"<< sim_mob::getColor((*it).second.currColor)  << std::endl;
	}


}
const std::string & Phase::getName() const
{
	return name;
}

std::string Phase::outputPhaseTrafficLight() const
{
	std::ostringstream output;
	if(links_map_.size() == 0 && crossings_map_.size() == 0) return 0;
	output << "\"Phase_" << name << "\"{";
	int i = 0;
	if(links_map_.size())
	{
		output << "\"Link\"{";
		links_map_iterator it = links_map_.begin();
		while(it != links_map_.end())
		{
			output << (*it).first << ":";             //linkFrom
			output << (*it).second.LinkTo << ":";     //linkTo
			output << (*it).second.currColor;         //currentColor
			it++;
			if(it != links_map_.end())
				output << ",";

		}
		output << "}";
	}


	if(crossings_map_.size())
	{
		output << "\"Crossing\"{";
		crossings_map_iterator it = crossings_map_.begin();
		while(it != crossings_map_.end())
		{
			output << (*it).first ;            //crossing *
			output << (*it).second.currColor ; // current color
			it++;
			if(it != crossings_map_.end())
				output << ",";

		}
		output << "}";
	}

	output << "}";
	return output.str();
}
}//namespace
