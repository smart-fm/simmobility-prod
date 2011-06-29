/**
 * \file stubs.h
 * Placeholder functions. If you find a function here, it will certainly be replaced
 * and relocated later.
 */

#pragma once

#include <vector>
#include <iostream>

#include "simple_classes.h"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Region.hpp"
#include "workers/Worker.hpp"


//namespace sim_mob {} //This is a temporary file, so it exists outside the namespace


//Function stubs
void setConfiguration() {
	std::cout <<"Server is configured." <<std::endl;
}
void loadNetwork() {
	std::cout <<"Network has been loaded." <<std::endl;
}
void loadSingleTripChain(const sim_mob::Agent* const ag, const TripChain* const tc) {
	trivial(tc->id);
}
void createSingleAgent(const sim_mob::Agent* const ag) {
	trivial(ag->getId()); //Trivial. Presumably, we'd set an agent's other properties here.
}
void createSingleChoiceSet(ChoiceSet* const cs, unsigned int newID) {
	cs->id = newID;
}
void createSingleVehicle(Vehicle* const v, unsigned int newID) {
	v->id = newID;
}



//Example of using a Worker with a functional pointer instead of sub-classing.
void load_trip_chain(sim_mob::Worker<TripChain>& wk, frame_t frameNumber)
{
	//Using functional pointers instead of inheritance means we have to cast from void*
	for (std::vector<TripChain*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		loadSingleTripChain(NULL, *it);   //At the moment, no way to link from agents to trip chains.
	}
}

void load_agents(sim_mob::Worker<sim_mob::Agent>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Agent*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		createSingleAgent(*it);   //At the moment, no way to link from agents to trip chains.
	}
}

void load_choice_sets(sim_mob::Worker<ChoiceSet>& wk, frame_t frameNumber)
{
	for (std::vector<ChoiceSet*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		createSingleChoiceSet(*it, (*it)->id);   //At the moment, no way to link from agents to trip chains.
	}
}

void load_vehicles(sim_mob::Worker<Vehicle>& wk, frame_t frameNumber)
{
	for (std::vector<Vehicle*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		createSingleVehicle(*it, (*it)->id);   //At the moment, no way to link from agents to trip chains.
	}
}



void agentDecomposition(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId()); //Trivial. Possibly move agents later.
	}
}

void updateVehicleQueue(std::vector<Vehicle>& vehicles) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<vehicles.size(); i++) {
		vehicles[i].id = vehicles[i].id; //Trivial. Will update queues later.
	}
}

void updateTrafficInfo(std::vector<sim_mob::Region>& regions) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<regions.size(); i++) {
		trivial(regions[i].getId()); //Trivial. Update other properties later.
	}
}

void updateSurveillanceData(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId()); //Trivial. Later we will collate data and send it to surveillance systems.
	}
}

void updateGUI(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will update the GUI
	}
}

void saveStatistics(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will log all agent data.
	}
}

void saveStatisticsToDB(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will save all statistis to the database.
	}
}




//Quick double-check
bool checkIDs(const std::vector<sim_mob::Agent*>& agents, const std::vector<TripChain>& trips, const std::vector<ChoiceSet>& choiceSets, const std::vector<Vehicle>& vehicles) {
	std::string error = "";
	for (size_t i=0; i<agents.size(); i++) {
		if (agents[i]->getId() != i)
			error = "Agent ID";
	}
	for (size_t i=0; i<trips.size(); i++) {
		if (trips[i].id != i)
			error = "Trip Chain ID";
	}
	for (size_t i=0; i<choiceSets.size(); i++) {
		if (choiceSets[i].id != i)
			error = "Choice Set ID";
	}
	for (size_t i=0; i<vehicles.size(); i++) {
		if (vehicles[i].id != i)
			error = "Vehicle ID";
	}

	if (error.empty())
		return true;
	else {
		std::cout <<"Error, invalid " <<error <<std::endl;
		return false;
	}
}




