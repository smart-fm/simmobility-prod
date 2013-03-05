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


void sim_mob::xml::Activity_t_pimpl::pre ()
{
	model = sim_mob::Activity();
}

sim_mob::TripChainItem* sim_mob::xml::Activity_t_pimpl::post_Activity_t ()
{
	sim_mob::Activity* res = new sim_mob::Activity(model);//here, model only helps as a factory object

	//Retrieve a temporary item, copy over.
	sim_mob::TripChainItem* temp = TripChainItem_t_pimpl::post_TripChainItem_t ();
	res->personID = temp->personID;
	res->itemType = temp->itemType;
	res->sequenceNumber = temp->sequenceNumber;
	res->startTime = temp->startTime;
	res->endTime = temp->endTime;
	delete temp;

	return res;
}

void sim_mob::xml::Activity_t_pimpl::description (const ::std::string& value)
{
	model.description = value;
}

void sim_mob::xml::Activity_t_pimpl::location (unsigned int value)
{
	model.location = book.getNode(value);
}

void sim_mob::xml::Activity_t_pimpl::locationType (std::string value)
{
	model.locationType = GetLocationType(value);
}

void sim_mob::xml::Activity_t_pimpl::isPrimary (bool value)
{
	model.isPrimary = value;
}

void sim_mob::xml::Activity_t_pimpl::isFlexible (bool value)
{
	model.isFlexible = value;
}

void sim_mob::xml::Activity_t_pimpl::isMandatory (bool value)
{
	model.isMandatory = value;
}


