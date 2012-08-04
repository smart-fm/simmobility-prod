/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "RoleFactory.hpp"

using namespace sim_mob;
using std::map;
using std::string;


bool isKnownRole(const string& roleName) const
{
	throw std::runtime_error("Not implemented yet");
}

map<string, bool> getRequiredAttributes(const string& roleName) const
{
	throw std::runtime_error("Not implemented yet");
}

Role* createRole(const string& name, const map<string, string>& props) const
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

