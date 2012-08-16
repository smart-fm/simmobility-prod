/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TripChain.hpp"
#include <algorithm>

using std::string;
using namespace sim_mob;

sim_mob::TripChainItem::TripChainItem(int entId, string type, DailyTime start,
		DailyTime end, unsigned int seqNumber) :
		entityID(entId), itemType(getItemType(type)), startTime(start), endTime(
				end), sequenceNumber(seqNumber)
{
}

sim_mob::Activity::Activity(string locType) : TripChainItem(), description(""), location(nullptr),
		locationType(getLocationType(locType)), isPrimary(false), isFlexible(false), isMandatory(false)
{
}

sim_mob::Trip::Trip(int entId, std::string type, unsigned int seqNumber,
		DailyTime start, DailyTime end, int tripId, Node* from,
		std::string fromLocType, Node* to, std::string toLocType) :
		TripChainItem(entId, type, start, end, seqNumber), tripID(tripId), fromLocation(
				from), fromLocationType(getLocationType(fromLocType)), toLocation(
				to), toLocationType(getLocationType(toLocType))
{
}


sim_mob::SubTrip::SubTrip(int entId, std::string type, unsigned int seqNumber,
		DailyTime start, DailyTime end, Node* from,
		std::string fromLocType, Node* to, std::string toLocType, std::string mode,
		bool isPrimary, std::string ptLineId) : Trip(entId, type, seqNumber, start, end, 0, from, fromLocType, to, toLocType),
		mode(mode) , isPrimaryMode(isPrimary), ptLineId(ptLineId)
{
}


TripChainItem::LocationType sim_mob::TripChainItem::getLocationType(
		string locType)
{
	locType.erase(remove_if(locType.begin(), locType.end(), isspace),
			locType.end());
	if (locType == "building") {
		return TripChainItem::LT_BUILDING;
	} else if (locType == "node") {
		return TripChainItem::LT_NODE;
	} else if (locType == "link") {
		return TripChainItem::LT_LINK;
	} else if (locType == "stop") {
		return TripChainItem::LT_PUBLIC_TRANSIT_STOP;
	} else {
		throw std::runtime_error("Unexpected location type.");
	}
}

TripChainItem::ItemType sim_mob::TripChainItem::getItemType(
		std::string itemType)
{
	itemType.erase(remove_if(itemType.begin(), itemType.end(), isspace),
			itemType.end());
	if (itemType == "Activity") {
		return IT_ACTIVITY;
	} else if (itemType == "Trip") {
		return IT_TRIP;
	} else {
		throw std::runtime_error("Unknown trip chain item type.");
	}
}

void sim_mob::Trip::addSubTrip(const sim_mob::SubTrip& aSubTrip)
{
	subTrips.push_back(aSubTrip);
}


bool sim_mob::operator==(const SubTrip& s1, const SubTrip& s2)
{
	//For now, just assume two items are equal if their entity IDs are equal.
    return (s1.entityID == s2.entityID);
}


bool sim_mob::operator!=(const SubTrip& s1, const SubTrip& s2)
{
    return !(s1 == s2);
}



