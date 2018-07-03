//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BufferedDataManager.hpp"

#include <cassert>
#include <algorithm>

using namespace sim_mob;
using std::vector;


sim_mob::BufferedBase::~BufferedBase() {
	assert(refCount==0);   //Error if refCount's not zero.
}


sim_mob::BufferedDataManager::~BufferedDataManager()
{
	//Stop managing all items
	while (managedData.begin() != managedData.end()) {
		stopManaging(*managedData.begin());
	}
}



void sim_mob::BufferedDataManager::beginManaging(BufferedBase* datum)
{
	//Only add if we're not managing it already.
	std::set<BufferedBase*>::iterator it = managedData.find(datum);
	if (it==managedData.end()) {
		managedData.insert(datum);

		//Helps with debugging.
		datum->refCount++;
	}
}

void sim_mob::BufferedDataManager::stopManaging(BufferedBase* datum)
{
	//Only remove if we are actually managing it.
	std::set<BufferedBase*>::iterator it = managedData.find(datum);
	if (it!=managedData.end()) {
		managedData.erase(it);

		//Helps with debugging.
		datum->refCount--;
	}
}

void sim_mob::BufferedDataManager::beginManaging(vector<BufferedBase*> data)
{
	for (vector<sim_mob::BufferedBase*>::iterator it=data.begin(); it!=data.end(); it++) {
		beginManaging(*it);
	}
}

void sim_mob::BufferedDataManager::stopManaging(vector<BufferedBase*> data)
{
	for (vector<sim_mob::BufferedBase*>::iterator it=data.begin(); it!=data.end(); it++) {
		stopManaging(*it);
	}
}


void sim_mob::BufferedDataManager::flip()
{
	for (std::set<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}



