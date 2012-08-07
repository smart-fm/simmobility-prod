/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoleFactory.hpp"

#include <stdexcept>

#include "entities/roles/Role.hpp"
#include "entities/misc/TripChain.hpp"

using namespace sim_mob;
using std::map;
using std::string;


bool sim_mob::RoleFactory::isKnownRole(const string& roleName) const
{
	throw std::runtime_error("Not implemented yet");
}

string sim_mob::RoleFactory::getTripChainMode(const std::string& roleName) const
{
	//"Car" or "Walk", but we can have more like "BusDriver" later...
	throw std::runtime_error("Not implemented yet");
}

map<string, bool> sim_mob::RoleFactory::getRequiredAttributes(const string& roleName) const
{
	throw std::runtime_error("Not implemented yet");
}

Role* sim_mob::RoleFactory::createRole(const string& name, const map<string, string>& props) const
{
	throw std::runtime_error("Not implemented yet");

	//TODO: The main function should "register" types, so that we aren't required
	//      to do all of this "if" checking. This also abstracts Roles, which we'll need anyway
	//      for the Short/Mid term.
	/*if (p.type == ENTITY_DRIVER) {
		res->changeRole(new Driver(res, config.mutexStategy));
	} else if (p.type == ENTITY_PEDESTRIAN) {
		res->changeRole(new Pedestrian(res));
	} else if (p.type == ENTITY_BUSDRIVER) {
		res->changeRole(new BusDriver(res, config.mutexStategy));
	} else if (p.type == ENTITY_ACTIVITYPERFORMER){
		// First trip chain item is Activity when Person is generated from Pending
		res->changeRole(new ActivityPerformer(res, dynamic_cast<const sim_mob::Activity&>(*(p.entityTripChain.front()))));
	} else {
		throw std::runtime_error("PendingEntity currently only supports Drivers, Pedestrians and Activity performers.");
	}*/
}

Role* sim_mob::RoleFactory::createRole(const TripChainItem* const currTripChainItem) const
{
	throw std::runtime_error("Not implemented yet");

	//TODO: We need to register activities as well as trips. Something like this:
	//
	//if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_TRIP){
	//	if (this->currSubTrip->mode == "Car") {
	//		return new Driver(this);
	//	} else if (this->currSubTrip->mode == "Walk") {
	//		return new Pedestrian(this);
	//	} else {
	//		throw std::runtime_error("Unknown role type for trip chain role change.");
	//	}
	//} else if(this->currTripChainItem->itemType == sim_mob::TripChainItem::IT_ACTIVITY){
	//	const Activity& currActivity = dynamic_cast<const Activity&>(*currTripChainItem);
	//	return new ActivityPerformer(this, currActivity);
	//} else {
	//	throw std::runtime_error("Unknown item type in trip chain");
	//}
}


