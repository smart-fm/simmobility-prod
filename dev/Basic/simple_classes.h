/**
 * \file simple_classes.h
 * Simple class definitions. If you find a class here, it will be moved to its own *.cpp,*.hpp
 * files later (or even removed).
 */
#pragma once

//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace

#include "buffering/BufferedDataManager.hpp"


//Class stubs
struct TripChain {
	unsigned int id;
	TripChain(unsigned int id=0) : id(id) {}

	//TEMP fix: WorkGroups manage subscriptions; perhaps we can delegate this to agents?
	std::vector<sim_mob::BufferedBase*> tmp_list;
	std::vector<sim_mob::BufferedBase*>& getSubscriptionList() { return tmp_list; }
};
struct ChoiceSet {
	unsigned int id;
	ChoiceSet(unsigned int id=0) : id(id) {}

	//TEMP fix: WorkGroups manage subscriptions; perhaps we can delegate this to agents?
	std::vector<sim_mob::BufferedBase*> tmp_list;
	std::vector<sim_mob::BufferedBase*>& getSubscriptionList() { return tmp_list; }
};
struct Vehicle {
	unsigned int id;
	Vehicle(unsigned int id=0) : id(id) {}

	//TEMP fix: WorkGroups manage subscriptions; perhaps we can delegate this to agents?
	std::vector<sim_mob::BufferedBase*> tmp_list;
	std::vector<sim_mob::BufferedBase*>& getSubscriptionList() { return tmp_list; }
};


//Temp config file classes
struct Point {
//	unsigned int xPos;
//	unsigned int yPos;
	double xPos;
	double yPos;
};



