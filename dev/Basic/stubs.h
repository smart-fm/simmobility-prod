/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file stubs.h
 * Placeholder functions. If you find a function here, it will certainly be replaced
 * and relocated later.
 */

#pragma once

#include <vector>
#include <iostream>

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
void createSingleAgent(const sim_mob::Agent* const ag) {
	trivial(ag->getId()); //Trivial. Presumably, we'd set an agent's other properties here.
}


//Example of using a Worker with a functional pointer instead of sub-classing.
void load_agents(sim_mob::Worker<sim_mob::Agent>& wk, frame_t frameNumber)
{
	for (std::vector<sim_mob::Agent*>::iterator it=wk.getEntities().begin(); it!=wk.getEntities().end(); it++) {
		createSingleAgent(*it);   //At the moment, no way to link from agents to trip chains.
	}
}


void agentDecomposition(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId()); //Trivial. Possibly move agents later.
	}
}

void updateSurveillanceData(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId()); //Trivial. Later we will collate data and send it to surveillance systems.
	}
}

void updateGUI(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will update the GUI
	}
}

void saveStatistics(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will log all agent data.
	}
}

void saveStatisticsToDB(std::vector<sim_mob::Agent*>& agents) {
	//Marked as not threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i]->getId());  //Trivial. Later we will save all statistis to the database.
	}
}




/**
 * Simple sanity check on Agent IDs. Checks that IDs start at 0, end at size(agents)-1,
 *   and contain every value in between. Order is not important.
 */
bool CheckAgentIDs(const std::vector<sim_mob::Agent*>& agents) {
	std::set<int> agent_ids;
	bool foundZero = false;
	bool foundMax = false;
	for (size_t i=0; i<agents.size(); i++) {
		int id = agents[i]->getId();
		agent_ids.insert(id);
		if (id==0) {
			foundZero = true;
		}
		if (id+1==static_cast<int>(agents.size())) {
			foundMax = true;
		}
	}
	if (agents.size()!=agent_ids.size() || !foundZero || !foundMax) {
		std::cout <<"Error, invalid Agent ID: " <<(agents.size()!=agent_ids.size()) <<","
			<<!foundZero <<"," <<!foundMax <<std::endl;
		return false;
	}

	return true;
}




