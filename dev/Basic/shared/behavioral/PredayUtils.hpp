//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

namespace sim_mob
{

enum class TravelTimeMode { TT_PRIVATE, TT_PUBLIC };

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

enum class VehicleOwnershipOption
{
	NO_VEHICLE,
	ONE_PLUS_MOTOR_ONLY,
	ONE_OP_CAR_W_WO_MOTOR,
	ONE_NORMAL_CAR_ONLY,
	ONE_NORMAL_CAR_AND_ONE_PLUS_MOTOR,
	TWO_PLUS_NORMAL_CAR_W_WO_MOTOR,
	INVALID
};
}
