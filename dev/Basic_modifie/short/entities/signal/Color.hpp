//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/assign/list_of.hpp>
#include <map>
#include <vector>

namespace sim_mob
{

class Phase;

/**Defines the various traffic light colours*/
enum TrafficColor
{
	TRAFFIC_COLOUR_INVALID = -1,
	TRAFFIC_COLOUR_RED = 1,
	TRAFFIC_COLOUR_AMBER = 2,
	TRAFFIC_COLOUR_GREEN = 3
};

static std::map<TrafficColor, std::string> trafficColorMap =
		boost::assign::map_list_of(TRAFFIC_COLOUR_INVALID, "InvalidTrafficColor")(TRAFFIC_COLOUR_RED, "Red")(TRAFFIC_COLOUR_AMBER, "Amber")(TRAFFIC_COLOUR_GREEN, "Green");

enum TrafficLightType
{
	TRAFFIC_LIGHT_TYPE_INVALID,
	TRAFFIC_LIGHT_TYPE_DRIVER,
	TRAFFIC_LIGHT_TYPE_PEDESTRIAN
};

class ColorSequence
{
private:
	/***/
	std::vector< std::pair<TrafficColor, int> > colourDurations;
	
	/***/
	TrafficLightType type;	
	
public:
	ColorSequence(TrafficLightType TrafficColorType = TRAFFIC_LIGHT_TYPE_DRIVER)
	{
		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<TrafficColor, int> > ColorDurationInput, TrafficLightType TrafficColorType = TRAFFIC_LIGHT_TYPE_DRIVER) :
	colourDurations(ColorDurationInput), type(TrafficColorType)
	{
	}

	const std::vector< std::pair<TrafficColor, int> >& getColorDuration()const;
	void setColorDuration(std::vector< std::pair<TrafficColor, int> >);
	
	const TrafficLightType getTrafficLightType() const;
	void setTrafficLightType(TrafficLightType);
	
	/**
	 * Creates a colour duration pair and adds it to the vector
	 * @param colour the colour
	 * @param duration the duration
	 */
	void addColorDuration(TrafficColor colour, int duration);
	
	/**
	 * Clears the vector of colour durations
	 */
	void clearColorDurations();

	/**
	 * Changes the duration of the given colour to the given value
	 * @param color the colour for which the duration is to be changed
	 * @param duration the duration value to be set
	 */
	void changeColorDuration(std::size_t color, int duration);
	
	/**
	 * Gets the string representation of the given traffic colour
	 * @param colour
	 * @return string representation of colour
	 */
	static std::string getTrafficLightColor(const TrafficColor& colour);
	
	/**
	 * Computes the supposed colour of the sequence after a given time lapse 
	 * @param duration time lapse duration
	 * @return colour
	 */
	TrafficColor computeColor(double duration);
	
	friend class sim_mob::Phase;
} ;
}
