//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RoleFactory.hpp"

#include <stdexcept>

#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/misc/BusTrip.hpp"

using namespace sim_mob;
using std::map;
using std::string;

sim_mob::RoleFactory::~RoleFactory()
{
	std::map<std::string, const sim_mob::Role *>::iterator itPrototypes = prototypes.begin();
	while(itPrototypes != prototypes.end())
	{
		safe_delete_item(itPrototypes->second);
		itPrototypes++;
	}
	prototypes.clear();
}

void sim_mob::RoleFactory::registerRole(const std::string& name, const Role* prototype)
{
	if (prototypes.count(name)>0) {
		throw std::runtime_error("Duplicate role type.");
	}

	prototypes[name] = prototype;
}

void sim_mob::RoleFactory::registerRole(const Role* prototype)
{
	if (prototypes.count(prototype->getRoleName())>0) {
		throw std::runtime_error("Duplicate role type.");
	}

	if ((prototype->getRoleName().size() == 0)||(prototype->getRoleName().empty())) {
		throw std::runtime_error("Invalid Key for Role type.");
	}

	prototypes[prototype->getRoleName()] = prototype;
}

const Role* sim_mob::RoleFactory::getPrototype(const string& name) const
{
	map<string, const Role*>::const_iterator it = prototypes.find(name);
	if (it!=prototypes.end()) {
		const Role* role = it->second;
		return role;
	}
	return nullptr;
}

bool sim_mob::RoleFactory::isKnownRole(const string& roleName) const
{
	return getPrototype(roleName);
}

string sim_mob::RoleFactory::GetRoleName(const std::string mode)
{
	if(mode=="Car" || mode=="Taxi") { return "driver"; }
	if(mode=="Walk") { return "pedestrian"; }
	if(mode=="Bus") { return "busdriver"; }
	if(mode=="BusTravel") { return "passenger"; }
	if(mode=="WaitingBusActivity") { return "waitBusActivity"; }
	if(mode=="Motorcycle") { return "biker"; }
	if(mode=="Activity") { return "activityRole"; }
	throw std::runtime_error("unknown SubTrip mode: " + mode);
}

//gets mode of transfer from tripchain item and converts it to a mapping understandable by role factory and person classes, that is all
const std::string sim_mob::RoleFactory::GetTripChainItemRoleName(const sim_mob::TripChainItem* tripChainItem, const sim_mob::SubTrip& subTrip) const{
	//This is a temporary function; it involves global knowledge of roles, so it's inelegant.
	// Also, our "modes" seem to be increasingly similar to our "roles", so there shouldn't be
	// two names for them
	if (tripChainItem->itemType==TripChainItem::IT_TRIP) { return GetRoleName(subTrip.getMode()); }
	else if(tripChainItem->itemType==TripChainItem::IT_ACTIVITY) { return "activityRole"; }
	else if(tripChainItem->itemType==TripChainItem::IT_BUSTRIP) { return "busdriver"; }
	throw std::runtime_error("unknown TripChainItem type");
}

map<string, bool> sim_mob::RoleFactory::getRequiredAttributes(const string& roleName) const
{
	//For now, all Roles have the same set of required attributes.
	map<string, bool> res;
	res["originPos"] = false;
	res["destPos"] = false;
	//res["time"] = false;
	return res;
}


Role* sim_mob::RoleFactory::createRole(const TripChainItem* currTripChainItem, const sim_mob::SubTrip* subTrip, Person* parent) const
{
	string roleName = RoleFactory::GetTripChainItemRoleName(currTripChainItem, *subTrip);
	return createRole(roleName, parent);
}

Role* sim_mob::RoleFactory::createRole(const string& name, Person* parent) const
{
	const Role* prot = getPrototype(name);
	if (!prot) {
		throw std::runtime_error("Unknown role type; cannot clone.");
	}
	Role* role = prot->clone(parent);
	role->make_frame_tick_params(parent->currTick);
	return role;
}


