#pragma once

#include <vector>
#include <iostream>

#include "simple_classes.h"
#include "entities/Agent.hpp"

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
	tc.id = ag.getId();
}
void createSingleAgent(const Agent& ag) {
	trivial(ag.getId()); //Trivial. Presumably, we'd set an agent's other properties here.
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
	trivial(a.getId()); //Trivial. Will update shortest path later.
}


