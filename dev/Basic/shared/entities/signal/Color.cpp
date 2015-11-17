//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Color.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;

const std::vector< std::pair<TrafficColor, int> > & ColorSequence::getColorDuration() const
{
	return colourDurations;
}

void ColorSequence::setColorDuration(std::vector< std::pair<TrafficColor, int> > durations)
{
	colourDurations = durations;
}

const TrafficLightType ColorSequence::getTrafficLightType() const
{
	return type;
}

void ColorSequence::setTrafficLightType(TrafficLightType trafficLightType)
{
	type = trafficLightType;
}

void ColorSequence::addColorDuration(TrafficColor color, int duration)
{
	colourDurations.push_back(std::make_pair(color, duration));
}

void ColorSequence::clearColorDurations()
{
	colourDurations.clear();
}

void ColorSequence::changeColorDuration(std::size_t color, int duration)
{
	std::vector< std::pair<TrafficColor, int> >::iterator it;
	for (it = colourDurations.begin(); it != colourDurations.end(); ++it)
	{
		if ((*it).first == color)
		{
			(*it).second = duration;
			break;
		}
	}
}

std::string ColorSequence::getTrafficLightColor(const TrafficColor &value)
{
	if (trafficColorMap.find(value) == trafficColorMap.end())
	{
		return trafficColorMap[TRAFFIC_COLOUR_INVALID];
	}

	return trafficColorMap[value];
}

TrafficColor ColorSequence::computeColor(double duration)
{
	short sum = 0;

	for (std::vector< std::pair<TrafficColor, int> >::iterator itDurations = colourDurations.begin(); itDurations != colourDurations.end(); ++itDurations)
	{
		sum += (*itDurations).second;
		if (duration < sum)
		{
			return (*itDurations).first;
		}
	}
	
	//The return inside the loop must execute before the loop exits otherwise something is wrong!
	Warn() << "ColorSequence::computeColor(): colour duration (" << duration << ") > sum of colour durations (" << sum << ")";
	Warn() << "Returning TRAFFIC_COLOUR_RED...";
	
	return TRAFFIC_COLOUR_RED;
}