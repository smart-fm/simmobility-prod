#include "Color.hpp"
namespace sim_mob
{

TrafficColor ColorSequence::computeColor(double Duration)
{
	std::size_t sum = 0;
	std::vector< std::pair<TrafficColor,std::size_t> >::iterator it = ColorDuration.begin();

	for(; it != ColorDuration.end(); it++)
	{
		sum += (*it).second;
		if(Duration < sum ) return (*it).first;
	}
	//the return inside the loop must execute befor the loop exits otherwise something is wrong!
	return (*it).first; //will return the last color in the sequence if there is an error!
}

std::vector< std::pair<TrafficColor,std::size_t> > ColorSequence::getColorDuration() { return ColorDuration; }
TrafficLightType ColorSequence::getTrafficLightType() { return type; }

void ColorSequence::addColorPair(std::pair<TrafficColor,std::size_t> p)
{
	ColorDuration.push_back(p);
}

void ColorSequence::removeColorPair(int position = 0)
{
	ColorDuration.erase(ColorDuration.begin() + position );
}

void ColorSequence::changeColorDuration(std::size_t color,std::size_t duration)
{
	std::vector< std::pair<TrafficColor,std::size_t> >::iterator it=ColorDuration.begin();
	for(it=ColorDuration.begin(); it!=ColorDuration.end(); it++)
		if((*it).first == color)
		{
			(*it).second = duration;
			break;
		}
}

}//namespace
