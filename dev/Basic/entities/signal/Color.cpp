/*a mark from new ws*/
#include "Color.hpp"
namespace sim_mob
{

TrafficColor ColorSequence::computeColor(double Duration)
{
	std::size_t sum = 0;
//	std::cout << "Inside computecolor " ; getchar();
	if(ColorDuration.size() > 0) std::cout << "ColorDuration.size > 0 ";
	else
		 std::cout << "ColorDuration.size <= 0 ";
//	getchar();
//	std::cout << "ColorDuration.size=" << ColorDuration.size() << std::endl; getchar();
	std::vector< std::pair<TrafficColor,std::size_t> >::iterator it = ColorDuration.begin();
//	std::cout << "Inside computecolor-ColorDuration.begin\n"; getchar();
	for(; it != ColorDuration.end(); it++)
	{
//		std::cout << "Inside computecolor-loop\n"; getchar();
		sum += (*it).second;
//		std::cout << "Inside computecolor-loop-if\n"; getchar();
		if(Duration < sum )
			{
//				std::cout<< "getting out computecolor_\n"; getchar();
				return (*it).first;
			}
	}
	//the return inside the loop must execute befor the loop exits otherwise something is wrong!
//	std::cout<< "getting out computecolor\n"; getchar();
	return (*it).first; //will return the last color in the sequence if there is an error!
}

void ColorSequence::setColorDuration(std::vector< std::pair<TrafficColor,std::size_t> > cs)
{
	ColorDuration = cs;
}

std::vector< std::pair<TrafficColor,std::size_t> > ColorSequence::getColorDuration() { return ColorDuration; }
TrafficLightType ColorSequence::getTrafficLightType() { return type; }

void ColorSequence::addColorDuration(TrafficColor color,std::size_t duration)
{
	ColorDuration.push_back(std::make_pair(color,duration));

}
void ColorSequence::addColorPair(std::pair<TrafficColor,std::size_t> p)
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
