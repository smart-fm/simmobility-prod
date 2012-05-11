#pragma once
#include<vector>
#include"defaults.hpp"
namespace sim_mob
{
//Forward declarations
class Phase;

//moved to defaults.hpp for now
//enum TrafficColor
//{
//    Red =1,    			///< Stop, do not go beyond the stop line.
//    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
//    Green = 3,   		///< Proceed either in the forward, left, or right direction.
//    FlashingRed = 4,	///future use
//    FlashingAmber = 5,	///future use
//    FlashingGreen = 6	///future use
//};

//depricated
struct VehicleTrafficColors
{
    TrafficColor left;     ///< Traffic-color for the left direction.
    TrafficColor forward;  ///< Traffic-color for the forward direction.
    TrafficColor right;    ///< Traffic-color for the right direction.

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
	Pedestrian_Light
};

class ColorSequence
{
public:
	ColorSequence(TrafficLightType TrafficColorType = Driver_Light)
	{
		ColorDuration.push_back(std::make_pair(Red,1));//All red moment ususally takes 1 second
		ColorDuration.push_back(std::make_pair(Amber,3));//a portion of the total time of the phase length is taken by amber
		ColorDuration.push_back(std::make_pair(Green,0));//Green is phase at green and should be calculated using the corresponding phase length
		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<TrafficColor,std::size_t> > ColorDurationInput, TrafficLightType TrafficColorType = Driver_Light) :
		ColorDuration(ColorDurationInput),
		type(TrafficColorType){}

	std::vector< std::pair<TrafficColor,std::size_t> > getColorDuration();
	TrafficLightType getTrafficLightType();

	void addColorPair(std::pair<TrafficColor,std::size_t> p);

	void removeColorPair(int position);

	void changeColorDuration(std::size_t color,std::size_t duration);
	//computes the supposed color of the sequence after a give time lapse
	TrafficColor computeColor(double Duration);
private:
	std::vector< std::pair<TrafficColor,std::size_t> > ColorDuration;
	TrafficLightType type;

	friend class sim_mob::Phase;
};
}//namespcae
