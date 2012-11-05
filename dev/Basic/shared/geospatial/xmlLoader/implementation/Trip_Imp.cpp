#include "geo10-pimpl.hpp"

using namespace sim_mob::xml;

namespace {

//Helper: Convert a location type string to an object of that type.
//TODO: This code is copied at several different locations; it should go in the top-level RoadNetwork classes as a public static function.
sim_mob::TripChainItem::LocationType  GetLocationType(std::string name)
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

} //End unnamed namespace


void sim_mob::xml::Trip_t_pimpl::pre ()
{
	model = sim_mob::Trip();
}

sim_mob::TripChainItem* sim_mob::xml::Trip_t_pimpl::post_Trip_t ()
{
	//NOTE: I'm not sure if the model would ever be null. It should have been created in pre(), not in tripID(). ~Seth
	//if(!model) { return 0; }
	sim_mob::Trip* res = new sim_mob::Trip(model);

	//Copy over temporary properties.
	sim_mob::TripChainItem* temp = TripChainItem_t_pimpl::post_TripChainItem_t ();
	res->personID = temp->personID;
	res->itemType = temp->itemType;
	res->sequenceNumber = temp->sequenceNumber;
	res->startTime = temp->startTime;
	res->endTime = temp->endTime;
	delete temp;

	return res;
}


void sim_mob::xml::Trip_t_pimpl::tripID (long long value)
{
	model.tripID = value;
}

void sim_mob::xml::Trip_t_pimpl::fromLocation (unsigned int value)
{
	model.fromLocation = book.getNode(value);
}

void sim_mob::xml::Trip_t_pimpl::fromLocationType (std::string value)
{
	model.fromLocationType = GetLocationType(value);
}

void sim_mob::xml::Trip_t_pimpl::toLocation (unsigned int value)
{
	model.toLocation = book.getNode(value);
}

void sim_mob::xml::Trip_t_pimpl::toLocationType (std::string value)
{
	model.toLocationType = GetLocationType(value);
}

void sim_mob::xml::Trip_t_pimpl::subTrips (std::vector<sim_mob::SubTrip> value)
{
	model.setSubTrips(value);
}

