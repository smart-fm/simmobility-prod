//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TripChain.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "entities/Person.hpp"
#include "geospatial/Node.hpp"


using std::string;
using namespace sim_mob;



sim_mob::TripChainItem::LocationType  sim_mob::TripChainItem::GetLocationTypeXML(std::string name)
{
	if (name == "LT_BUILDING") {
		return sim_mob::TripChainItem::LT_BUILDING;
	} else if (name == "LT_NODE") {
		return sim_mob::TripChainItem::LT_NODE;
	} else if (name == "LT_LINK") {
		return sim_mob::TripChainItem::LT_LINK;
	} else if (name == "LT_PUBLIC_TRANSIT_STOP") {
		return sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP;;
	}

	throw std::runtime_error("Unknown TripChain location type.");
}


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


const std::vector<sim_mob::SubTrip>& sim_mob::Trip::getSubTrips() const
{
	return subTrips;
}

std::vector<sim_mob::SubTrip>& sim_mob::Trip::getSubTripsRW()
{
	return subTrips;
}

void sim_mob::Trip::setSubTrips(const std::vector<sim_mob::SubTrip>& subTrips)
{
	this->subTrips = subTrips;
}


sim_mob::SubTrip::SubTrip(std::string entId, std::string type, unsigned int seqNumber,int requestTime,
		DailyTime start, DailyTime end, Node* from,
		std::string fromLocType, Node* to, std::string toLocType, std::string mode,
		bool isPrimary, std::string ptLineId) : Trip(entId, type, seqNumber, requestTime, start, end, "", from, fromLocType, to, toLocType),
		mode(mode) , isPrimaryMode(isPrimary), ptLineId(ptLineId), totalDistanceOD(0)
{
}


std::string sim_mob::TripChainItem::getPersonID() const
{
	return personID;
}

void sim_mob::TripChainItem::setPersonID(const std::string& val)
{
	personID = val;
}

void sim_mob::TripChainItem::setPersonID(int val)
{
	personID = boost::lexical_cast<std::string>(val);
}


TripChainItem::LocationType sim_mob::TripChainItem::getLocationType(
		string locType)
{
	locType.erase(remove_if(locType.begin(), locType.end(), isspace), locType.end());
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
    return (s1.getPersonID() == s2.getPersonID()) ;
}


bool sim_mob::operator!=(const SubTrip& s1, const SubTrip& s2)
{
    return !(s1 == s2);
}



