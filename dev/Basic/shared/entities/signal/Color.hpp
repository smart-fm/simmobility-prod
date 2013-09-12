//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include<vector>
#include"defaults.hpp"

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
//		ColorDuration.push_back(std::make_pair(Amber,3));//a portion of the total time of the phase length is taken by amber
//		ColorDuration.push_back(std::make_pair(Green,0));//Green is phase at green and should be calculated using the corresponding phase length
//		ColorDuration.push_back(std::make_pair(Red,1));//All red moment ususally takes 1 second
		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<TrafficColor,short> > ColorDurationInput, TrafficLightType TrafficColorType = Driver_Light) :
		ColorDuration(ColorDurationInput),
		type(TrafficColorType){}

	std::vector< std::pair<TrafficColor,short> > & getColorDuration();
	const TrafficLightType getTrafficLightType() const;

	void addColorPair(std::pair<TrafficColor,short> p);
	void addColorDuration(TrafficColor,short);
	void removeColorPair(int position);
	void clear();

	void changeColorDuration(std::size_t color,short duration);
	//computes the supposed color of the sequence after a give time lapse
	TrafficColor computeColor(double Duration);
	void setColorDuration(std::vector< std::pair<TrafficColor,short> >);
	void setTrafficLightType(TrafficLightType);
private:
	std::vector< std::pair<TrafficColor,short> > ColorDuration;
	TrafficLightType type;
//public:
//	void operator= (const  ColorSequence & c)
//	{
//		type = c.type;
//		ColorDuration = c.ColorDuration;
////		return this;
//	}


	friend class sim_mob::Phase;
};
}//namespcae
