/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TripChain.hpp"

using std::string;
using namespace sim_mob;

TripChainItem::LocationType sim_mob::TripChainItem::getLocationType(string locType)
{
	if(locType == "building") return TripChainItem::LT_BUILDING;
	else if(locType == "node") return TripChainItem::LT_NODE;
	else if(locType == "link") return TripChainItem::LT_LINK;
	else if(locType == "stop") return TripChainItem::LT_PUBLIC_TRANSIT_STOP;
	throw std::runtime_error("Unexpected location type.");
}
