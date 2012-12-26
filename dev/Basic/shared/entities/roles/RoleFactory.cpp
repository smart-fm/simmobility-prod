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
		std::cout << name << " found in the prototypes\n";
		const Role* role = it->second;
		return role;
	}
	return nullptr;
}

std::string const getRoleName(const Role* role)
{
//	for(std::map<std::string, const sim_mob::Role*>::iterator)
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
//	const Trip* trip = dynamic_cast<const Trip*>(currTripChainItem);
//	const Activity* act = dynamic_cast<const Activity*>(currTripChainItem);
//	const BusTrip* bustrip = dynamic_cast<const BusTrip*>(currTripChainItem);
	if (currTripChainItem->itemType==TripChainItem::IT_TRIP)
	{
		const Trip* trip = dynamic_cast<const Trip*>(currTripChainItem);
		if(trip)
		{
		if (trip->getSubTrips().front().mode=="Car") {
			return "driver";
		} else if (trip->getSubTrips().front().mode=="Walk") {
			return "pedestrian";
		} else if (trip->getSubTrips().front().mode=="Bus") {
			return "busdriver";
		}
		else if(trip->getSubTrips().front().mode=="BusTravel")
		{
			return "BusPassenger";
		}
		else {
			throw std::runtime_error("Unknown Trip subclass.");
		}
	}
	}else if (currTripChainItem->itemType==TripChainItem::IT_ACTIVITY) {
		if(dynamic_cast<const Activity*>(currTripChainItem))
			return "activityRole";
		else{
			throw std::runtime_error("Unknown Activity subclass.");
		}

	} else if (currTripChainItem->itemType==TripChainItem::IT_BUSTRIP) {
		if(dynamic_cast<const BusTrip*>(currTripChainItem))
			return "busdriver";
		else{
			throw std::runtime_error("Unknown bus trip subclass.");
		}
	} else { //Offer some protection
		throw std::runtime_error("Trip/Activity/bustrip mismatch, or unknown TripChainItem subclass.");
	}
}

string sim_mob::RoleFactory::GetSubTripMode(const sim_mob::SubTrip &subTrip)
{
		if (subTrip.mode=="Car")    return "driver";
		if (subTrip.mode=="Walk")   return "pedestrian";
		if (subTrip.mode=="BusTravel")   return "BusPassenger";
		if (subTrip.mode=="Bus")    return "busdriver";
		std::cout << " throwing error\n";
		throw std::runtime_error("Unknown SubTrip mode.");
}

//gets mode of transfer from tripchain item and converts it to a mapping understandable by rolfactory and person classes, that is all
const std::string sim_mob::RoleFactory::GetTripChainItemMode(const sim_mob::TripChainItem *tripChainItem,const sim_mob::SubTrip *subTrip) const{
	if(tripChainItem->itemType == sim_mob::TripChainItem::IT_BUSTRIP) return "busdriver";
	const std::string roleName = tripChainItem->getMode(subTrip);//(subTrip ? tripChainItem->getMode(subTrip) : "");
	std::cout << "tripChainItem->personid " << (tripChainItem)->personID << std::endl;
	std::cout << "rolename = " << roleName << std::endl;
//	std::cout << "subTrip->mode = " << subTrip->mode << std::endl;
	if (roleName == "Car")
		return "driver";
	if (roleName == "Walk")
		return "pedestrian";
	if (roleName == "Bus")
		return "busdriver";
	if (roleName == "BusTravel")
		return "BusPassenger";
	if (roleName == "Activity")
		return "activityRole";
	std::ostringstream out;
	out << "Unknown SubTrip mode : " << roleName;
	throw std::runtime_error(out.str());
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


Role* sim_mob::RoleFactory::createRole(const TripChainItem* currTripChainItem,const sim_mob::SubTrip *subTrip_, Person* parent) const
{
	string roleName = RoleFactory::GetTripChainItemMode(currTripChainItem,subTrip_ );
	return createRole(roleName, parent);
}

Role* sim_mob::RoleFactory::createRole(const string& name, Person* parent) const
{
	const Role* prot = getPrototype(name);
	if (!prot) {
		throw std::runtime_error("Unknown role type; cannot clone.");
	}
	Role* role = prot->clone(parent);
	return role;
}

//Role* sim_mob::RoleFactory::createRole(const TripChainItem* currTripChainItem, Person* parent) const
//{
//	string roleName = RoleFactory::GetTripChainMode(currTripChainItem);
//	return createRole(roleName, parent);
//}
//
//Role* sim_mob::RoleFactory::createRole(const sim_mob::SubTrip &subTrip_, Person* parent) const
//{
//	string roleName = RoleFactory::GetSubTripMode(subTrip_);
//	return createRole(roleName, parent);
//}


