//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{

/**
 * simple helper class to store an OD pair
 *
 * \author Harish Loganathan
 */
class OD_Pair
{
private:
	int origin;
	int destination;

public:
	OD_Pair(int org, int dest);
	virtual ~OD_Pair();

	bool operator ==(const OD_Pair& rhs) const;
	bool operator !=(const OD_Pair& rhs) const;

	bool operator >(const OD_Pair& rhs) const;
	bool operator <(const OD_Pair& rhs) const;

	int getDestination() const
	{
		return destination;
	}

	int getOrigin() const
	{
		return origin;
	}
};
}
