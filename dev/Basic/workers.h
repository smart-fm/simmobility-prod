#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <iostream>

#include "simple_classes.h"
#include "constants.h"
#include "stubs.h"


//Load trip chains in parallel
/*void loadTripChains(const std::vector<Agent>& agents, std::vector<TripChain>& trips)
{
	//Fill with empty objects.
	trips.resize(agents.size());

	//Make workers; we will "join" them all later.
	boost::thread* workers = new boost::thread[agents.size()];
	for (size_t i=0; i<agents.size(); i++) {
		workers[i] = boost::thread((boost::bind(&loadSingleTripChain, agents[i], boost::ref(trips[i]))));
	}

	//Join, delete
	for (size_t i=0; i<agents.size(); i++)
		workers[i].join();
	delete [] workers;
}*/


//Load agents, choice sets, and vehicles in parallel
/*void loadAgentsChoiceSetsAndVehicles(std::vector<Agent>& agents, std::vector<ChoiceSet>& choiceSets, std::vector<Vehicle>& vehicles) {
	//Fill with empty objects.
	choiceSets.resize(agents.size());
	vehicles.resize(agents.size());    //Note: The number of vehicles is just assumed to be the number of agents for now.

	//Make workers; we will "join" them all later.
	boost::thread* workers = new boost::thread[agents.size()+choiceSets.size()+vehicles.size()];

	//Create all agents
	for (size_t i=0; i<agents.size(); i++) {
		workers[i] = boost::thread(boost::bind(&createSingleAgent, boost::ref(agents[i])));
	}

	//Create all choice sets
	for (size_t i=0; i<choiceSets.size(); i++) {
		workers[agents.size()+i] = boost::thread(boost::bind(&createSingleChoiceSet, boost::ref(choiceSets[i]), i));
	}

	//Create all vehicles
	for (size_t i=0; i<vehicles.size(); i++) {
		workers[agents.size()+choiceSets.size()+i] = boost::thread(boost::bind(&createSingleVehicle, boost::ref(vehicles[i]), i));
	}

	//Join, delete
	for (size_t i=0; i<agents.size(); i++)
		workers[i].join();
	delete [] workers;
}*/





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

/*void updateAndAdvancePhase(std::vector<Agent>& agents) {
	//NOTE: This is marked as not boost::threadable, but I am treating it as boost::threadable for now.
	boost::thread* workers = new boost::thread[agents.size()];
	for (size_t i=0; i<agents.size(); i++) {
		if (agents[i].currMode==DRIVER) {
			workers[i] = boost::thread(boost::bind(&updateDriverBehavior, boost::ref(agents[i])));
		} else if (agents[i].currMode==PEDESTRIAN || agents[i].currMode==CYCLIST) {
			workers[i] = boost::thread(boost::bind(&updatePedestrianBehavior, boost::ref(agents[i])));
		} else if (agents[i].currMode==PASSENGER) {
			workers[i] = boost::thread(boost::bind(&updatePassengerBehavior, boost::ref(agents[i])));
		}

		//NOTE: I added "path choice" into each of the "worker" functions.
	}

	//Join, delete
	for (size_t i=0; i<agents.size(); i++)
		workers[i].join();
	delete [] workers;
}*/


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
