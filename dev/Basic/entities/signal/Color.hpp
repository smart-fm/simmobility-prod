#pragma once
#include<vector>
#include"defaults.hpp"
namespace sim_mob
{
//Forward declarations
class Phase;

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
//		ColorDuration.push_back(std::make_pair(Amber,3));//a portion of the total time of the phase length is taken by amber
//		ColorDuration.push_back(std::make_pair(Green,0));//Green is phase at green and should be calculated using the corresponding phase length
//		ColorDuration.push_back(std::make_pair(Red,1));//All red moment ususally takes 1 second
//		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<TrafficColor,std::size_t> > ColorDurationInput, TrafficLightType TrafficColorType = Driver_Light) :
		ColorDuration(ColorDurationInput),
		type(TrafficColorType){}

	std::vector< std::pair<TrafficColor,std::size_t> > & getColorDuration();
	TrafficLightType getTrafficLightType();

	void addColorPair(std::pair<TrafficColor,std::size_t> p);
	void addColorDuration(TrafficColor,std::size_t);
	void removeColorPair(int position);
	void clear();

	void changeColorDuration(std::size_t color,std::size_t duration);
	//computes the supposed color of the sequence after a give time lapse
	TrafficColor computeColor(double Duration);
	void setColorDuration(std::vector< std::pair<TrafficColor,std::size_t> >);
private:
	std::vector< std::pair<TrafficColor,std::size_t> > ColorDuration;
	TrafficLightType type;

	friend class sim_mob::Phase;
};
}//namespcae
