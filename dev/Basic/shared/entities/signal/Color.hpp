//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include<vector>
#include"defaults.hpp"
#include <boost/assign/list_of.hpp>
#include <map>

namespace sim_mob
{
//Forward declarations
class Phase;

enum TrafficColor
{
	InvalidTrafficColor = -1,
    Red =1,    			///< Stop, do not go beyond the stop line.
    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
    Green = 3,   		///< Proceed either in the forward, left, or right direction.
    FlashingRed = 4,	///future use
    FlashingAmber = 5,	///future use
    FlashingGreen = 6	///future use
};

static  std::map<TrafficColor,std::string> trafficColorMap =
				boost::assign::map_list_of
				(InvalidTrafficColor,"InvalidTrafficColor")
				(Red,"Red")
				(Amber,"Amber")
				(Green,"Green")
				(FlashingRed,"FlashingRed")
				(FlashingAmber,"FlashingAmber")
				(FlashingGreen,"FlashingGreen");

//depricated
struct VehicleTrafficColors
{
	sim_mob::TrafficColor left;     ///< Traffic-color for the left direction.
	sim_mob::TrafficColor forward;  ///< Traffic-color for the forward direction.
	sim_mob::TrafficColor right;    ///< Traffic-color for the right direction.

    /// Constructor.
    VehicleTrafficColors(TrafficColor l, TrafficColor f, TrafficColor r)
      : left(l)
      , forward(f)
      , right(r)
    {
    }
};
enum TrafficLightType
{
	Driver_Light,
	Pedestrian_Light,
	InvalidTrafficLightType
};

class ColorSequence
{
public:
	ColorSequence(TrafficLightType TrafficColorType = Driver_Light)
	{
		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<TrafficColor,int> > ColorDurationInput, TrafficLightType TrafficColorType = Driver_Light) :
		ColorDuration(ColorDurationInput),
		type(TrafficColorType){}

	const std::vector< std::pair<TrafficColor,int> > & getColorDuration()const;
	const TrafficLightType getTrafficLightType() const;

	void addColorPair(std::pair<TrafficColor,int> p);
	void addColorDuration(TrafficColor,int);
	void removeColorPair(int position);
	void clear();

	void changeColorDuration(std::size_t color,int duration);
	static std::string getTrafficLightColorString(const TrafficColor&);
	//computes the supposed color of the sequence after a give time lapse
	TrafficColor computeColor(double Duration);
	void setColorDuration(std::vector< std::pair<TrafficColor,int> >);
	void setTrafficLightType(TrafficLightType);

private:
	std::vector< std::pair<TrafficColor,int> > ColorDuration;
	TrafficLightType type;
	friend class sim_mob::Phase;
};
}//namespcae
