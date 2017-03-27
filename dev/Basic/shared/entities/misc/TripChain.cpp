//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "TripChain.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "entities/Person.hpp"
#include "geospatial/network/Node.hpp"


using std::string;
using namespace sim_mob;

sim_mob::TripChainItem::LocationType sim_mob::TripChainItem::GetLocationTypeXML(std::string name)
{
	if (name == "LT_BUILDING")
	{
		return sim_mob::TripChainItem::LT_BUILDING;
	}
	else if (name == "LT_NODE")
	{
		return sim_mob::TripChainItem::LT_NODE;
	}
	else if (name == "LT_LINK")
	{
		return sim_mob::TripChainItem::LT_LINK;
	}
	else if (name == "LT_PUBLIC_TRANSIT_STOP")
	{
		return sim_mob::TripChainItem::LT_PUBLIC_TRANSIT_STOP;
		;
	}

	throw std::runtime_error("Unknown TripChain location type.");
}

sim_mob::TripChainItem::TripChainItem(
		string purpose, //purpose needs to be the first argument for convenience
		std::string entId,
		string type,
		DailyTime start,
		DailyTime end,
		unsigned int seqNumber,
		int requestTm,
		std::string mode,
		unsigned int edge) :
		personID(entId), itemType(getItemType(type)), purpose(getItemPurpose(purpose)), startTime(start), endTime(end),
		sequenceNumber(seqNumber), requestTime(requestTm), travelMode(mode),edgeId(edge)
{
}

sim_mob::TripChainItem::~TripChainItem()
{
}

const std::string sim_mob::TripChainItem::getMode() const
{
	return travelMode;
}

sim_mob::Activity::Activity(string locType, std::string purpose) :
		TripChainItem(purpose),
		isPrimary(false), isFlexible(false), isMandatory(false), location(nullptr)
{
	destinationType = getLocationType(locType);
}

sim_mob::Activity::~Activity()
{
}

sim_mob::Trip::Trip(std::string entId,
		std::string type,
		unsigned int seqNumber,
		int requestTime,
		DailyTime start,
		DailyTime end,
		std::string tripId,
		const Node* from,
		std::string fromLocType,
		const Node* to,
		std::string toLocType,
		std::string mode,
		std::string purpose) :
				TripChainItem(purpose, entId, type, start, end, seqNumber, requestTime, mode), tripID(tripId)
{
	origin = WayPoint(from);
	destination = WayPoint(to);
	originType = getLocationType(fromLocType);
	destinationType = getLocationType(toLocType);
	
	if (originType == TripChainItem::LT_NODE)
	{
		origin = WayPoint((Node*) from);
	}
	/*else if (originType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		origin = WayPoint((BusStop*) from);
	}*/

	if (destinationType == TripChainItem::LT_NODE)
	{
		destination = WayPoint((Node*) to);
	}
	/*else if (destinationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		destination = WayPoint((BusStop*) to);
	}*/
}

sim_mob::Trip::~Trip()
{
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

sim_mob::SubTrip::SubTrip(std::string entId,
		std::string type,
		unsigned int seqNumber,
		int requestTime,
		DailyTime start,
		DailyTime end,
		const Node* from,
		std::string fromLocType,
		const Node* to,
		std::string toLocType,
		std::string mode,
		bool isPrimary,
		std::string ptLineId) :
				Trip(entId, type, seqNumber, requestTime, start, end, "", from, fromLocType, to, toLocType, mode),
				isPT_Walk(false),walkTime(0.0), ptLineId(ptLineId), cbdTraverseType(sim_mob::TravelMetric::CBD_NONE),
				isTT_Walk(false)
{
}

sim_mob::SubTrip::~SubTrip()
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
	if (locType == "building")
	{
		return TripChainItem::LT_BUILDING;
	}
	else if (locType == "node")
	{
		return TripChainItem::LT_NODE;
	}
	else if (locType == "link")
	{
		return TripChainItem::LT_LINK;
	}
	else if (locType == "stop")
	{
		return TripChainItem::LT_PUBLIC_TRANSIT_STOP;
	}
	else
	{
		std::stringstream msg;
		msg << "Unexpected location type: \"" << locType << "\"";
		throw std::runtime_error(msg.str().c_str());
	}
}

StopType sim_mob::TripChainItem::getItemPurpose(std::string purpose)
{
	purpose.erase(remove_if(purpose.begin(), purpose.end(), isspace), purpose.end());
	if (purpose == "Work")
	{
		return WORK;
	}
	else if (purpose == "Education")
	{
		return EDUCATION;
	}
	else if (purpose == "Shop")
	{
		return SHOP;
	}
	else if (purpose == "Other")
	{
		return OTHER;
	}
	else
	{
		return NULL_STOP;
	}
}

TripChainItem::ItemType sim_mob::TripChainItem::getItemType(std::string itemType)
{
	itemType.erase(remove_if(itemType.begin(), itemType.end(), isspace), itemType.end());
	if (itemType == "Activity")
	{
		return IT_ACTIVITY;
	}
	else if (itemType == "Trip")
	{
		return IT_TRIP;
	}
	else if (itemType == "BusTrip")
	{
		return IT_BUSTRIP;
	}
	else if (itemType == "WaitingBusActivity")
	{
		return IT_WAITBUSACTIVITY;
	}
	else if( itemType == "WaitingTrainActivity"){
		return IT_WAITTRAINACTIVITY;
	}
	else
	{
		throw std::runtime_error("Unknown trip chain item type.");
	}
}

bool sim_mob::Activity::setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip * subtrip)
{
	person->originNode = person->destNode = destination;
	return true;
}

bool sim_mob::Trip::setPersonOD(sim_mob::Person *person, const sim_mob::SubTrip * subtrip)
{
	const sim_mob::SubTrip &subTrip_ = (subtrip ? *subtrip : subTrips.front());
	person->originNode = subTrip_.origin;
	person->destNode = subTrip_.destination;
	return true;
}

void sim_mob::Trip::addSubTrip(const sim_mob::SubTrip& subTrip)
{
	subTrips.push_back(subTrip);
}

const std::string sim_mob::SubTrip::getBusLineID() const
{
	return ptLineId;
}

bool sim_mob::operator==(const SubTrip& s1, const SubTrip& s2)
{
	//For now, just assume two items are equal if their entity IDs are equal.
	return (s1.getPersonID() == s2.getPersonID());
}

bool sim_mob::operator!=(const SubTrip& s1, const SubTrip& s2)
{
	return !(s1 == s2);
}
