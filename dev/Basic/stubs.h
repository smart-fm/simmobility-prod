#pragma once

#include <vector>
#include <iostream>

#include "simple_classes.h"


//Function stubs
void loadUserConf(std::vector<Agent>& agents, std::vector<Region>& regions) {
	for (size_t i=0; i<20; i++)
		agents.push_back(Agent(i));
	for (size_t i=0; i<5; i++)
		regions.push_back(Region(i));
	std::cout <<"Configuration file loaded." <<std::endl;
}
void setConfiguration() {
	std::cout <<"Server is configured." <<std::endl;
}
void loadNetwork() {
	std::cout <<"Network has been loaded." <<std::endl;
}
void loadSingleTripChain(const Agent& ag, TripChain& tc) {
	tc.id = ag.id;
}
void createSingleAgent(const Agent& ag) {
	trivial(ag.id); //Trivial. Presumably, we'd set an agent's other properties here.
}
void createSingleChoiceSet(ChoiceSet& cs, unsigned int newID) {
	cs.id = newID;
}
void createSingleVehicle(Vehicle& v, unsigned int newID) {
	v.id = newID;
}
void updateSingleRegionSignals(Region& r) {
	for (size_t i=0; i<r.signals.size(); i++) {
		r.signals[i].id = r.signals[i].id; //Trivial. Will update signal's other properties.
	}
}
void updateSingleShortestPath(Agent& a) {
	trivial(a.id); //Trivial. Will update shortest path later.
}
void pathChoice(Agent& a) {
	trivial(a.id); //Trivial. Will update path choice later.
}
void updateDriverBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update driver behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);
	}
}
void updatePedestrianBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update pedestrian behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);
	}
}
void updatePassengerBehavior(Agent& a) {
	trivial(a.id); //Trivial. Will update passenger behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.id)) {
		pathChoice(a);  //NOTE: Do passengers need to do this?
	}
}
