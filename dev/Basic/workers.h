#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <iostream>

#include "simple_classes.h"
#include "constants.h"
#include "stubs.h"





void updateSignalStatus(std::vector<Region>& regions) {
	//Make workers; we will "join" them all later.
	boost::thread* workers = new boost::thread[regions.size()];
	for (size_t i=0; i<regions.size(); i++) {
		workers[i] = boost::thread(boost::bind(&updateSingleRegionSignals, boost::ref(regions[i])));
	}

	//Join, delete
	for (size_t i=0; i<regions.size(); i++)
		workers[i].join();
	delete [] workers;
}


void updateTrafficInfo(std::vector<Region>& regions) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<regions.size(); i++) {
		regions[i].id = regions[i].id; //Trivial. Update other properties later.
	}
}

void calculateTimeDependentShortestPath(std::vector<Agent>& agents) {
	//Make workers; we will "join" them all later.
	boost::thread* workers = new boost::thread[agents.size()];
	for (size_t i=0; i<agents.size(); i++) {
		workers[i] = boost::thread(boost::bind(&updateSingleShortestPath, boost::ref(agents[i])));
	}

	//Join, delete
	for (size_t i=0; i<agents.size(); i++)
		workers[i].join();
	delete [] workers;
}

void agentDecomposition(std::vector<Agent>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i].getId()); //Trivial. Possibly move agents later.
	}
}

void updateVehicleQueue(std::vector<Vehicle>& vehicles) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<vehicles.size(); i++) {
		vehicles[i].id = vehicles[i].id; //Trivial. Will update queues later.
	}
}


void updateSurveillanceData(std::vector<Agent>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i].getId()); //Trivial. Later we will collate data and send it to surveillance systems.
	}
}

void updateGUI(std::vector<Agent>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i].getId());  //Trivial. Later we will update the GUI
	}
}

void saveStatistics(std::vector<Agent>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i].getId());  //Trivial. Later we will log all agent data.
	}
}

void saveStatisticsToDB(std::vector<Agent>& agents) {
	//Marked as not boost::threadable.
	for (size_t i=0; i<agents.size(); i++) {
		trivial(agents[i].getId());  //Trivial. Later we will save all statistis to the database.
	}
}
