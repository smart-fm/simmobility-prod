/*
 * World.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#include "World.hpp"

namespace sim_mob{

namespace amod {

World::World():  currentTime(0) {
	// TODO Auto-generated constructor stub

}

World::~World() {
	// TODO Auto-generated destructor stub
}

void World::populate(std::vector<Location> &locs, std::vector<Vehicle> &vehs, std::vector<Customer> &custs) {
    for (auto l : locs) {
		locations[l.getId()] = l;
	}
    
    for (auto v : vehs) {
        vehicles[v.getId()] = v;
    }
    
    for (auto c : custs) {
        customers[c.getId()] = c;
    }
    
}

void World::addVehicle(const Vehicle &veh) {
	vehicles[veh.getId()] = veh;
}

void World::setVehicle(const Vehicle &veh) {
	vehicles[veh.getId()] = veh;
}

void World::addVehicles(const std::vector<Vehicle> &vehs) {
	for (auto v : vehs) {
		addVehicle(v);
	}
}


void World::removeVehicle(int vehId) {
    vehicles.erase(vehId);
}

void World::removeVehicles(std::vector<int> &vehIds) {
    for (auto id : vehIds) {
		removeVehicle(id);
	}
}

Vehicle World::getVehicle(int vehId) {
    auto it = vehicles.find(vehId);
	if (it != vehicles.end()) {
		return it->second;
	}
	return Vehicle();
}

Vehicle * World::getVehiclePtr(int vehId) {
    auto it = vehicles.find(vehId);
	if (it != vehicles.end()) {
		return &(it->second);
	}
	return nullptr;
}


void World::getVehicles(std::vector<Vehicle> *vehs) const {
	if (!vehs) {
		throw std::runtime_error("World::getVehicles vehs pointer is null");
	}
	for (auto it=vehicles.begin(); it != vehicles.end(); ++it) {
		vehs->push_back(it->second);
	}
}
    
void World::getVehicles(std::unordered_map<int, Vehicle>::const_iterator* bitr,
		std::unordered_map<int, Vehicle>::const_iterator* eitr) const {
	*bitr = vehicles.begin();
	*eitr = vehicles.end();
}


int World::getNumVehicles() const {
	return (int) vehicles.size();
}


void World::addCustomer(const Customer &cust) {
	customers[cust.getId()] = cust;
}

void World::setCustomer(const Customer &cust) {
	customers[cust.getId()] = cust;
}

void World::addCustomers(const std::vector<Customer> &custs) {
    for (auto v : custs) {
        addCustomer(v);
    }
}

void World::removeCustomer(int custId) {
    customers.erase(custId);
}

void World::removeCustomers(std::vector<int> &custIds) {
    for (auto id : custIds) {
        removeCustomer(id);
    }
}

Customer World::getCustomer(int custId) {
    auto it = customers.find(custId);
    if (it != customers.end()) {
        return it->second;
    } else {
        throw std::runtime_error("No such customer found");
    }
}

Customer * World::getCustomerPtr(int custId) {
    auto it = customers.find(custId);
    if (it != customers.end()) {
        return &(it->second);
    }
    return nullptr;
}

void World::getCustomers(std::unordered_map<int, Customer>::const_iterator* bitr,
		std::unordered_map<int, Customer>::const_iterator* eitr) {
	*bitr = customers.begin();
	*eitr = customers.end();
}


void World::getCustomers(std::vector<Customer> *custs) {
    if (!custs) {
        throw std::runtime_error("World::getCustomers custs pointer is null");
    }
    for (auto it=customers.begin(); it != customers.end(); ++it) {
        custs->push_back(it->second);
    }
}


int World::getNumCustomers() {
    return (int) customers.size();
}


void World::addLocation(const Location &loc) {
	locations[loc.getId()] = loc;
}

void World::setLocation(const Location &loc) {
	locations[loc.getId()] = loc;
}

void World::addLocations(const std::vector<Location> &locs) {
	for (auto loc : locs) {
		addLocation(loc);
	}
}

void World::removeLocation(int locId) {
    locations.erase(locId);
}

void World::removeLocations(std::vector<int> &locIds) {
    for (auto id : locIds) {
		removeLocation(id);
	}
}

Location World::getLocation(int locId) {
    auto it = locations.find(locId);
	if (it != locations.end()) {
		return it->second;
	}
	return Location();
}

Location * World::getLocationPtr(int locId) {
    auto it = locations.find(locId);
	if (it != locations.end()) {
		return &(it->second);
	}
	return nullptr;
}

void World::getLocations(std::vector<Location> *locs) {
	if (!locs) {
		throw std::runtime_error("World::getLocations: locs pointer is null");
	}
	for (auto it=locations.begin(); it != locations.end(); ++it) {
		locs->push_back(it->second);
	}
}

void World::getLocations(std::unordered_map<int, Location>::const_iterator* bitr,
		std::unordered_map<int, Location>::const_iterator* eitr) {
	*bitr = locations.begin();
	*eitr = locations.end();
}

int World::getNumLocations() {
	return (int) locations.size();
}

void World::addEvent(Event &event) {
    events.insert({event.id, event});
}

void World::setEvent(Event &event) {
	events.insert({event.id, event});
}

void World::addEvents(const std::vector<Event> &events) {
	for (auto event : events) {
		addEvent(event);
	}
}

void World::getEvents(std::vector<Event> *events_) {
    if (!events_) {
		throw std::runtime_error("World::getEvents: events pointer is null");
	}
	for (auto it=events.begin(); it != events.end(); ++it) {
        events_->push_back(it->second);
	}
}

void World::removeEvent(int evntId) {
    auto it = events.find(evntId);
	if (it != events.end()) {
		events.erase(it);
	}
}

void World::clearEvents() {
	events.clear();
}

int World::getNumEvents() {
	return (int) events.size();
}

double World::getCurrentTime() {
	return currentTime;
}

void World::setCurrentTime(double currTime) {
    currentTime = currTime;
}

}

}
