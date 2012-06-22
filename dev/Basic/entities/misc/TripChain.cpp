/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TripChain.hpp"

using std::string;
using namespace sim_mob;

sim_mob::TripChainItem::~TripChainItem() {}
sim_mob::Activity::~Activity() {}
sim_mob::Trip::~Trip() {}
sim_mob::SubTrip::~SubTrip() {}

TripChainItem::LocationType sim_mob::TripChainItem::getLocationType(string locType)
{
	if(locType == "building"){
		return TripChainItem::LT_BUILDING;
	} else if(locType == "node"){
		return TripChainItem::LT_NODE;
	} else if(locType == "link"){
		return TripChainItem::LT_LINK;
	} else if(locType == "stop") {
		return TripChainItem::LT_PUBLIC_TRANSIT_STOP;
	} else {
		throw std::runtime_error("Unexpected location type.");
	}
}

TripChainItem::ItemType sim_mob::TripChainItem::getItemType(std::string itemType){
	if(itemType == "Activity") {
		return IT_ACTIVITY;
	} else if(itemType == "Trip") {
		return IT_TRIP;
	} else {
		throw std::runtime_error("Unknown trip chain item type.");
	}
}

void sim_mob::Trip::addSubTrip(sim_mob::SubTrip* aSubTrip) {
	subTrips.push_back(aSubTrip);
}



