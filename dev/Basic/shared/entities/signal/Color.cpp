/*a mark from new ws*/
#include "Color.hpp"
namespace sim_mob
{

TrafficColor ColorSequence::computeColor(double Duration)
{

	short sum = 0;
	std::vector< std::pair<TrafficColor,short> >::iterator it = ColorDuration.begin();
	for(; it != ColorDuration.end(); it++)
	{
		sum += (*it).second;
		if(Duration < sum )
			{

//				if((*it).first == sim_mob::Green)
//				{
//					std::cout << "Returning color Green" <<  std::endl;
//				}
//				else
//					std::cout << "Returning color " << (*it).first << std::endl;
				return (*it).first;
			}
	}
	//the return inside the loop must execute befor the loop exits otherwise something is wrong!
//	return ColorDuration[ColorDuration.size() -1].first; //will return the last color in the sequence if there is an error!
//	std::cout << "Didn't find anything for duration " << Duration << " (finally compared to " << sum << " )" << std::endl;
	return sim_mob::Red;
}

void ColorSequence::setColorDuration(std::vector< std::pair<TrafficColor,short> > cs)
{
	ColorDuration = cs;
}

void ColorSequence::setTrafficLightType(TrafficLightType t)
{
	type = t;
}
std::vector< std::pair<TrafficColor,short> > & ColorSequence::getColorDuration() { return ColorDuration; }
const TrafficLightType ColorSequence::getTrafficLightType() const { return type; }

void ColorSequence::addColorDuration(TrafficColor color,short duration)
{
	ColorDuration.push_back(std::make_pair(color,duration));

}
void ColorSequence::addColorPair(std::pair<TrafficColor,short> p)
{
	ColorDuration.push_back(p);
}

void ColorSequence::removeColorPair(int position = 0)
{
	ColorDuration.erase(ColorDuration.begin() + position );
}
void ColorSequence::clear()
{
	ColorDuration.clear();
}

void ColorSequence::changeColorDuration(std::size_t color,short duration)
{
	std::vector< std::pair<TrafficColor,short> >::iterator it=ColorDuration.begin();
	for(it=ColorDuration.begin(); it!=ColorDuration.end(); it++)
		if((*it).first == color)
		{
			(*it).second = duration;
			break;
		}
}

}//namespace
