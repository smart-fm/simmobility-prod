/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "TripChain.hpp"
#include "entities/Person.hpp"
#include <algorithm>

using std::string;
using namespace sim_mob;

sim_mob::TripChainItem::TripChainItem(std::string entId, string type, DailyTime start,
		DailyTime end, unsigned int seqNumber, int requestTm) :
		personID(entId), itemType(getItemType(type)), startTime(start), endTime(end), sequenceNumber(seqNumber), requestTime(requestTm)
{
}

sim_mob::Activity::Activity(string locType) : TripChainItem(), description(""), location(nullptr),
		locationType(getLocationType(locType)), isPrimary(false), isFlexible(false), isMandatory(false)
{
}

sim_mob::Trip::Trip(std::string entId, std::string type, unsigned int seqNumber, int requestTime,
		DailyTime start, DailyTime end, std::string tripId, Node* from,
		std::string fromLocType, Node* to, std::string toLocType) :
		TripChainItem(entId, type, start, end, seqNumber,requestTime), tripID(tripId), fromLocation(
				from), fromLocationType(getLocationType(fromLocType)), toLocation(to), toLocationType(getLocationType(toLocType))
{
	if( fromLocationType == TripChainItem::LT_NODE )
	{
		fromLocation = WayPoint( (Node*)from );
	}
	else if( fromLocationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP )
	{
		fromLocation = WayPoint( (BusStop*)from );
	}

	if( toLocationType == TripChainItem::LT_NODE )
	{
		toLocation = WayPoint( (Node*)to );
	}
	else if( toLocationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP )
	{
		toLocation = WayPoint( (BusStop*)to );
	}
}


sim_mob::SubTrip::SubTrip(std::string entId, std::string type, unsigned int seqNumber,int requestTime,
		DailyTime start, DailyTime end, Node* from,
		std::string fromLocType, Node* to, std::string toLocType, std::string mode,
		bool isPrimary, std::string ptLineId) : Trip(entId, type, seqNumber, requestTime, start, end, "", from, fromLocType, to, toLocType),
		mode(mode) , isPrimaryMode(isPrimary), ptLineId(ptLineId), stop(nullptr)
{
}
/*
sim_mob::FMODTrip::FMODTrip(std::string entId, std::string type, unsigned int seqNumber,int requestTime,
		DailyTime start, DailyTime end, Node* from,
		std::string fromLocType, Node* to, std::string toLocType,
		std::string mode, STOP stopIn, bool isPrimary, std::string ptLineId) : SubTrip(entId, type, seqNumber, requestTime, start, end,
				from, fromLocType, to, toLocType, mode, isPrimary, ptLineId), stop(stopIn)
{

}
*/
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
		std::stringstream msg;
		msg <<"Unexpected location type: \"" <<locType <<"\"";
		throw std::runtime_error(msg.str().c_str());
	}
}
//sim_mob::TripChainItem::LocationType sim_mob::TripChainItem::getLocationType()
//{
//	return LocationType;
//}
TripChainItem::ItemType sim_mob::TripChainItem::getItemType(std::string itemType)
{
	itemType.erase(remove_if(itemType.begin(), itemType.end(), isspace),
			itemType.end());
	if (itemType == "Activity") {
		return IT_ACTIVITY;
	} else if (itemType == "Trip") {
		return IT_TRIP;
	} else if (itemType == "BusTrip") {
		return IT_BUSTRIP;
	}else if(itemType == "FMOD" ) {
		return IT_FMODSIM;
	} else {
		throw std::runtime_error("Unknown trip chain item type.");
	}
}
bool sim_mob::Activity::setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip * subtrip) {
	person->originNode = person->destNode = WayPoint(location);
	return true;
}

bool sim_mob::Trip::setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip * subtrip) {
	const sim_mob::SubTrip &subTrip_ = (subtrip  ? *subtrip : subTrips.front());
	person->originNode = subTrip_.fromLocation;
	person->destNode = subTrip_.toLocation;
	return true;
}
void sim_mob::Trip::addSubTrip(const sim_mob::SubTrip& subTrip)
{
	subTrips.push_back(subTrip);
}
const std::string sim_mob::Trip::getMode(const sim_mob::SubTrip *subTrip) const{
	if(!subTrip)
		throw std::runtime_error("Invalid subtrip supplied");
	return subTrip->getMode();
}

const std::string sim_mob::SubTrip::getMode() const {
		//std::cout << "Mode for subtrip " << this << " from " << this->fromLocation->getID() << " to " << this->toLocation->getID() << " is " << mode << std::endl;
		return mode;
	}
bool sim_mob::operator==(const SubTrip& s1, const SubTrip& s2)
{
	//For now, just assume two items are equal if their entity IDs are equal.
    return (s1.personID == s2.personID) ;
}


bool sim_mob::operator!=(const SubTrip& s1, const SubTrip& s2)
{
    return !(s1 == s2);
}



