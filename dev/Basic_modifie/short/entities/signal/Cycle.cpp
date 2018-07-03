//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Cycle.hpp"

#include <cmath>
#include "Offset.hpp"

using namespace sim_mob;

//For convenience, redeclare some of the public static constants in Offset.
namespace
{
//Parameters for calculating next cycle length
const static double DSmax = sim_mob::Offset::DSmax;
const static double DSmed = sim_mob::Offset::DSmed;
const static double DSmin = sim_mob::Offset::DSmin;

const static double CLmax = sim_mob::Offset::CLmax;
const static double CLmed = sim_mob::Offset::CLmed;
const static double CLmin = sim_mob::Offset::CLmin;

//Parameters for calculating next Offset
const static double CL_low = sim_mob::Offset::CL_low;
const static double CL_up = sim_mob::Offset::CL_up;
const static double Off_low = sim_mob::Offset::Off_low;
const static double Off_up = sim_mob::Offset::Off_up;

const static double fixedCL = sim_mob::Offset::fixedCL;
}

void Cycle::updatePreviousCycleLen()
{
	previousCycleLength = currentCycleLength;
}

void Cycle::updateCurrentCycleLen()
{
	currentCycleLength = nextCycleLength;
}

double Cycle::getCurrentCycleLen()
{
	return currentCycleLength;
}

void Cycle::setCurrentCycleLen(double length)
{
	currentCycleLength = length;
}

double Cycle::getNextCycleLen()
{
	return nextCycleLength;
}

void Cycle::update(double maxDS)
{
	calcNextCycleLen(maxDS);
	updatePreviousCycleLen();
	updateCurrentCycleLen();
}

double Cycle::calcNextCycleLen(double maxDS)
{
	//Parameters in SCATS
	double RL0;
	double w1 = 0.45, w2 = 0.33, w3 = 0.22;

	//Calculate RL0
	if (maxDS <= DSmed)
	{
		RL0 = CLmin + (maxDS - DSmin) * (CLmed - CLmin) / (DSmed - DSmin);
	}
	else
	{
		RL0 = CLmed + (maxDS - DSmed) * (CLmax - CLmed) / (DSmax - DSmed);
	}

	int sign;
	double diff_CL;
	
	if (RL0 - currentCycleLength >= 0)
	{
		diff_CL = RL0 - currentCycleLength;
		sign = 1;
	}
	else
	{
		diff_CL = currentCycleLength - RL0;
		sign = -1;
	}

	//Modify the diff_CL0
	double diff_CL0;
	
	if (diff_CL <= 4)
	{
		diff_CL0 = diff_CL;
	}
	else if (diff_CL > 4 && diff_CL <= 8)
	{
		diff_CL0 = 0.5 * diff_CL + 2;
	}
	else
	{
		diff_CL0 = 0.25 * diff_CL + 4;
	}

	double RL1 = currentCycleLength + sign * diff_CL0;

	//RL is partly determined by its previous values
	double RL = w1 * RL1 + w2 * previousRealLength1 + w3 * previousRealLength2;

	//Update previous RL
	previousRealLength2 = previousRealLength1;
	previousRealLength1 = RL1;

	sign = (RL >= currentCycleLength) ? 1 : -1; //This is equivalent.

	//Set the maximum change as 6s
	if (std::abs(RL - currentCycleLength) <= 6)
	{
		nextCycleLength = RL;
	}
	else
	{
		nextCycleLength = currentCycleLength + sign * 6;
	}

	//When the maximum changes in last two cycle are both larger than 6s, the value can be set as 9s
	if (((nextCycleLength - currentCycleLength) >= 6 && (currentCycleLength - previousCycleLength) >= 6) ||
			((nextCycleLength - currentCycleLength) <= -6 && (currentCycleLength - previousCycleLength) <= -6))
	{
		if (std::abs(RL - currentCycleLength) <= 9)
		{
			nextCycleLength = RL;
		}
		else
		{
			nextCycleLength = currentCycleLength + sign * 9;
		}
	}

	if (nextCycleLength > CLmax)
	{
		nextCycleLength = CLmax;
	}
	else if (nextCycleLength < CLmin)
	{
		nextCycleLength = CLmin;
	}
	
	return nextCycleLength;
}
