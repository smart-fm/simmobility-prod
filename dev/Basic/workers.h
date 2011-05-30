#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <iostream>

#include "simple_classes.h"
#include "constants.h"
#include "stubs.h"





/*void updateSignalStatus(std::vector<Region>& regions) {
	//Make workers; we will "join" them all later.
	boost::thread* workers = new boost::thread[regions.size()];
	for (size_t i=0; i<regions.size(); i++) {
		//workers[i] = boost::thread(boost::bind(&updateSingleRegionSignals, boost::ref(regions[i])));
	}

	//Join, delete
	for (size_t i=0; i<regions.size(); i++)
		workers[i].join();
	delete [] workers;
}*/


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


