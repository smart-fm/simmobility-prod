//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PredayUtils.hpp"

namespace sim_mob
{

OD_Pair::OD_Pair(int org, int dest) : origin(org), destination(dest)
{
}

OD_Pair::~OD_Pair()
{
}

bool OD_Pair::operator ==(const OD_Pair& rhs) const
{
	return ((origin == rhs.origin) && (destination == rhs.destination));
}

bool OD_Pair::operator !=(const OD_Pair& rhs) const
{
	return !(*this == rhs);
}

bool OD_Pair::operator >(const OD_Pair& rhs) const
{
	if (origin > rhs.origin)
	{
		return true;
	}
	if (origin == rhs.origin && destination > rhs.destination)
	{
		return true;
	}
	return false;
}

bool OD_Pair::operator <(const OD_Pair& rhs) const
{
	if (origin < rhs.origin)
	{
		return true;
	}
	if (origin == rhs.origin && destination < rhs.destination)
	{
		return true;
	}
	return false;
}

} //end namespace sim_mob




