//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{

class Offset
{
private:
	double currOffset;
	double nextOffset;
	
public:
	Offset() : currOffset(0), nextOffset(0)
	{
	}
	
	double getcurrOffset() const;
	double getnextOffset() const;	
	
	/**
	 * Updates the offset
	 * @param nextCL the next cycle length
	 */
	void update(double nextCL);
	
	/**
	 * Uses next cycle length to calculate next offset
	 * @param nextCL the next cycle length
	 */
	void calcNextOffset(double nextCL);
	
	/**
	 * Sets the next offset as the current offset when the cycle changes
	 */
	void updateCurrOffset();

	//Parameters for calculating next cycle length
	const static double DSmax;
	const static double DSmed;
	const static double DSmin;

	const static double CLmax;
	const static double CLmed;
	const static double CLmin;

	//Parameters for calculating next Offset
	const static double CL_low;
	const static double CL_up;
	const static double Off_low;
	const static double Off_up;

	const static double fixedCL;
} ;

}
