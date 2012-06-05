#pragma once


namespace sim_mob {

enum TrafficColor
{
    Red =1,    			///< Stop, do not go beyond the stop line.
    Amber = 2,  		///< Slow-down, prepare to stop before the stop line.
    Green = 3,   		///< Proceed either in the forward, left, or right direction.
    FlashingRed = 4,	///future use
    FlashingAmber = 5,	///future use
    FlashingGreen = 6	///future use
};


//Private namespace
//TODO: Might want to move this private namespace out of the header file. ~Seth
namespace {
//parameters for calculating next cycle length
const double DSmax = 0.9, DSmed = 0.5, DSmin = 0.3;
const double CLmax = 140, CLmed = 100, CLmin = 60;

//parameters for calculating next Offset
const double CL_low = 70, CL_up = 120;
const double Off_low = 5, Off_up = 26;

const double fixedCL = 60;
}
}
