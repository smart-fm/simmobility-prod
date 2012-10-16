/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoleFactory.hpp"

#include <stdexcept>

#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/misc/TripChain.hpp"

using namespace sim_mob;
using std::map;
using std::string;

void sim_mob::RoleFactory::registerRole(const std::string& name, const Role* prototype)
{
	if (prototypes.count(name)>0) {
		throw std::runtime_error("Duplicate role type.");
	}

	prototypes[name] = prototype;
}

const Role* sim_mob::RoleFactory::getPrototype(const string& name) const
{
	map<string, const Role*>::const_iterator it = prototypes.find(name);
	if (it!=prototypes.end()) {
		return it->second;
	}
	return nullptr;
}

bool sim_mob::RoleFactory::isKnownRole(const string& roleName) const
{
	return getPrototype(roleName);
}

string sim_mob::RoleFactory::GetTripChainMode(const sim_mob::TripChainItem* currTripChainItem)
{
	//This is a temporary function; it involves global knowledge of roles, so it's inelegant.
	// Also, our "modes" seem to be increasingly similar to our "roles", so there shouldn't be
	// two names for them
	const Trip* trip = dynamic_cast<const Trip*>(currTripChainItem);
	const Activity* act = dynamic_cast<const Activity*>(currTripChainItem);
	const BusTrip* bustrip = dynamic_cast<const BusTrip*>(currTripChainItem);
	if (trip && currTripChainItem->itemType==TripChainItem::IT_TRIP) {
		if (trip->getSubTrips().front().mode=="Car") {
			return "driver";
		} else if (trip->getSubTrips().front().mode=="Walk") {
			return "pedestrian";
		} else if (trip->getSubTrips().front().mode=="Bus") {
			return "busdriver";
		} else {
			throw std::runtime_error("Unknown Trip subclass.");
		}
	} else if (act && currTripChainItem->itemType==TripChainItem::IT_ACTIVITY) {
		return "activityRole";
	} else if (bustrip && currTripChainItem->itemType==TripChainItem::IT_BUSTRIP) {
		return "busdriver";
	} else { //Offer some protection
		throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
	}
}

map<string, bool> sim_mob::RoleFactory::getRequiredAttributes(const string& roleName) const
{
	//For now, all Roles have the same set of required attributes.
	map<string, bool> res;
	res["originPos"] = false;
	res["destPos"] = false;
	res["time"] = false;
	return res;
}

Role* sim_mob::RoleFactory::createRole(const string& name, Person* parent) const
{
	const Role* prot = getPrototype(name);
	if (!prot) {
		throw std::runtime_error("Unknown role type; cannot clone.");
	}

	return prot->clone(parent);
}

Role* sim_mob::RoleFactory::createRole(const TripChainItem* currTripChainItem, Person* parent) const
{
	string roleName = RoleFactory::GetTripChainMode(currTripChainItem);
	return createRole(roleName, parent);
}


