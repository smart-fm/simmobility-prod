//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ReRouteEventArgs.hpp"

using std::string;

sim_mob::event::ReRouteEventArgs::ReRouteEventArgs(const string& blacklistRegion) : blacklistRegion(blacklistRegion)
{
}

sim_mob::event::ReRouteEventArgs::~ReRouteEventArgs()
{
}

string sim_mob::event::ReRouteEventArgs::getBlacklistRegion() const
{
	return blacklistRegion;
}
