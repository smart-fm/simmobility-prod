//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{

class Cycle
{
private:
	/**Length of the previous cycle*/
	double previousCycleLength;
	
	/**Length of the current cycle*/
	double currentCycleLength;
	
	/**Length of the next cycle*/
	double nextCycleLength;

	/**Real length of the previous cycle length*/
	double previousRealLength1;
	
	/**Real length of the cycle before the previous cycle*/
	double previousRealLength2;
	
	/**
	 * Stores the current cycle length as the previous cycle length when a cycle changes
	 */
	void updatePreviousCycleLen();
	
	/**
	 * Stores the next cycle length as the current cycle length when a cycle changes
	 */
	void updateCurrentCycleLen();

public:
	Cycle() : previousCycleLength(0), currentCycleLength(0), nextCycleLength(0), previousRealLength1(0), previousRealLength2(0)
	{
	}

	double getCurrentCycleLen();
	void setCurrentCycleLen(double length);
	
	double getNextCycleLen();
	
	/**
	 * This method determines the next cycle length using the maximum DS across all lanes. 
	 * @param maxDS maximum DS across all lanes
	 * @return 
	 */
	double calcNextCycleLen(double maxDS);
	
	/**
	 * Updates the cycle using the given DS (which is the max DS across all lanes)
	 * @param maxDS maximum DS across all lanes
	 */
	void update(double maxDS);
};

}
