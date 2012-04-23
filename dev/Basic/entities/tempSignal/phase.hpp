//dont worry guys, i will create the cpp file(s) later.
#include<map>
#include<vector>
#include "geospatial/Link.hpp"
using namespace std;

namespace sim_mob
{
//Forward dseclaration
class Signal;
enum TrafficColor
{
    Red =1,    			///< Stop, do not go beyond the stop line.
    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
    Green = 3,   		///< Proceed either in the forward, left, or right direction.
    FlashingRed = 4,	///future use
    FlashingAmber = 5,	///future use
    FlashingGreen = 6	///future use
};

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
	Driver,
	Pedestrian
};

enum TrafficControlMode
{
	FIXED_TIME,
	ADAPTIVE
};

class ColorSequence
{
public:
	ColorSequence(size_t TrafficColorType = Driver)
	{
		ColorDuration.push_back(make_pair(Red,1));//All red moment ususally takes 1 second
		ColorDuration.push_back(make_pair(Amber,3));//a portion of the total time of the phase length is taken by amber
		ColorDuration.push_back(make_pair(Green,0));//Green is phase at green and should be calculated using the corresponding phase length
		type = TrafficColorType;
	}

	ColorSequence(std::vector< std::pair<size_t,size_t> > ColorDurationInput, TrafficColorType = Driver) :
		ColorDuration(ColorDurationInput),
		type(TrafficColorType){}

	std::vector< std::pair<size_t,size_t> > getColorDuration() { return ColorDuration; }
	TrafficLightType getTrafficLightType() { return type; }

	void addColorPair(std::pair<size_t,size_t> p)
	{
		ColorDuration.push_back(p);
	}

	void removeColorPair(int position = 0)
	{
		ColorDuration.erase(position);
	}

	void changeColorDuration(size_t color,size_t duration)
	{
		std::vector< std::pair<size_t,size_t> >::iterator it=ColorDuration.begin();
		for(it=ColorDuration.begin(); it!=ColorDuration.end(); it++)
			if((*it).first == color)
			{
				(*it).second = duration;
				break;
			}
	}
private:
	std::vector< std::pair<size_t,size_t> > ColorDuration;
	TrafficLightType type;
};

/*
 * in adaptive signal control, the cycle length, phase percentage and offset are likely to change
 * but the color sequence of a traffic signal, amber time etc are assumed to be intact.
 */
class Phase : private ColorSequence
{
public:
	Phase(std::vector< std::pair<size_t,size_t> > ColorDurationInput, TrafficColorType = Driver) : ColorSequence(ColorDurationInput, TrafficColorType) {}
	Phase(size_t cycleLengthInput = 0, size_t percentageInput = 0,TrafficColorType = Driver) :
		cycleLength(cycleLengthInput),
		percentage(percentageInput),
		ColorSequence(TrafficColorType) {}
	void setPercentage(size_t p)
	{
		percentage = p;
	}
	void setCycleLength(size_t c)
	{
		cycleLength = c;
	}
private:
	double cycleLength;//Don't get confused.Yes i know, cycleLength belongs to splitPlan not phase! this poor variable is just helping here. it might come handy
	size_t percentage;
	//The links that will get a green light at this phase

	/*vahid:
	 * In the phase class we need to store the links associated with a phase.
	 * Also, we need to
	 * the following map is used to store the links associated with this phase
	 * as well as the colors associated with those links.
	 */
	std::map<sim_mob::Link *, struct VehicleTrafficColors> links_colors;

	//The crossings that will get a green light at this phase
	std::set<sim_mob::Crossing *> crossings;
	friend class SplitPlan;
	friend class Signal;
};


class SplitPlan
{
private:
	size_t cycleLength;
	std::vector<Phase> phases;//order is important
public:
	SplitPlan(size_t cycleLengthInput = 0): cycleLength(cycleLengthInput){}
	size_t getCycleLength() {return cycleLength;}
	void setCycleLength(size_t c) {cycleLength = c;}
	std::vector<Phase> getPhases() { return phases_;}
//	void dummy(){
//		phases[0].links.find()
//	}
	friend class Signal;
};
}
