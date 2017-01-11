//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WorkGroup.hpp"

#include "GenConfig.h"

#include <boost/thread.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Agent.hpp"
#include "entities/AuraManager.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "partitions/PartitionManager.hpp"
#include "path/PathSetManager.hpp"
#include "workers/Worker.hpp"

using std::vector;

using namespace sim_mob;

sim_mob::WorkGroup::WorkGroup(unsigned int wgNum, unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr,
		PartitionManager* partitionMgr, PeriodicPersonLoader* periodicLoader, uint32_t simulationStart) :
		wgNum(wgNum), numWorkers(numWorkers), numSimTicks(numSimTicks), tickStep(tickStep), auraMgr(auraMgr), partitionMgr(partitionMgr), tickOffset(0), started(
				false), currTimeTick(0), nextTimeTick(0), loader(nullptr), nextWorkerID(0), frame_tick_barr(nullptr), buff_flip_barr(nullptr), msg_bus_barr(
				nullptr), macro_tick_barr(nullptr), profile(nullptr), periodicPersonLoader(periodicLoader), nextLoaderIdx(0), simulationStart(simulationStart)
{
	if (ConfigManager::GetInstance().CMakeConfig().ProfileAuraMgrUpdates())
	{
		profile = new ProfileBuilder();
	}
}

sim_mob::WorkGroup::~WorkGroup()  //Be aware that this will hang if Workers are wait()-ing. But it prevents undefined behavior in boost.
{
	//Delete/clear all Workers.
	for (vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
	{
		Worker* wk = *it;
		wk->interrupt();
		wk->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		wk->migrateAllOut(); //This ensures that Agents can safely delete themselves.
		delete wk;
	}
	workers.clear();

	//Delete/close all Log files.
	for (std::list<std::ostream*>::iterator it = managed_logs.begin(); it != managed_logs.end(); it++)
	{
		delete *it;
	}
	managed_logs.clear();

	//The only barrier we can delete is the non-shared barrier.
	//TODO: Find a way to statically delete the other barriers too (low priority; minor amount of memory leakage).
#ifndef SIMMOB_INTERACTIVE_MODE
	safe_delete_item(macro_tick_barr);
#endif

	//Clear the ProfileBuilder.
	safe_delete_item(profile);
}

void WorkGroup::addOutputFileNames(std::list<std::string>& res) const
{
	res.insert(res.end(), logFileNames.begin(), logFileNames.end());
}

Worker* WorkGroup::getLeastLoadedWorker(const vector<Worker*>& workers)
{
	Worker* res = nullptr;
	for (vector<Worker*>::const_iterator it = workers.begin(); it != workers.end(); it++)
	{
		if ((!res) || ((*it)->getAgentSize(true) < res->getAgentSize(true)))
		{
			res = *it;
		}
	}
	return res;
}

void sim_mob::WorkGroup::clear()
{
	for (vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
	{
		Worker* wk = *it;
		wk->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		wk->migrateAllOut(); //This ensures that Agents can safely delete themselves.
		delete wk;
	}
	workers.clear();

	//The only barrier we can delete is the non-shared barrier.
	//TODO: Find a way to statically delete the other barriers too (low priority; minor amount of memory leakage).
	safe_delete_item(macro_tick_barr);
}

void sim_mob::WorkGroup::initializeBarriers(FlexiBarrier* frame_tick, FlexiBarrier* buff_flip, FlexiBarrier* aura_mgr)
{
	//Shared barriers
	this->frame_tick_barr = frame_tick;
	this->buff_flip_barr = buff_flip;
	this->msg_bus_barr = aura_mgr;

	//Now's a good time to create our macro barrier too.
	if (tickStep > 1)
	{
		this->macro_tick_barr = new boost::barrier(numWorkers + 1);
	}
}

void sim_mob::WorkGroup::initWorkers(EntityLoadParams* loader)
{
	this->loader = loader;

	//Init our worker list-backs
	entToBeRemovedPerWorker.resize(numWorkers, vector<Entity*>());
	entToBeBredPerWorker.resize(numWorkers, vector<Entity*>());

	//Number Worker output threads something like:  "out_1_2.txt", where "1" is the WG number and "2" is the Worker number.
	std::stringstream prefixS;
	prefixS << "out_" << wgNum << "_";
	std::string prefix = prefixS.str();

	//Init the workers themselves.
	for (size_t i = 0; i < numWorkers; i++)
	{
		std::stringstream outFilePath;
		outFilePath << prefix << i << ".txt";
		std::ofstream* logFile = nullptr;

		ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();

		if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled() && config.ltParams.outputFiles.log_out_xx_files)
		{
			//TODO: Handle error case more gracefully.
			logFileNames.push_back(outFilePath.str());
			logFile = new std::ofstream(outFilePath.str().c_str());
			managed_logs.push_back(logFile);
		}

		std::vector<Entity*>* entWorker = &entToBeRemovedPerWorker.at(i);
		std::vector<Entity*>* entBredPerWorker = &entToBeBredPerWorker.at(i);

		workers.push_back(new Worker(this, logFile, frame_tick_barr, buff_flip_barr, msg_bus_barr, macro_tick_barr, entWorker, entBredPerWorker, numSimTicks, tickStep,simulationStart));
	}
}

void sim_mob::WorkGroup::startAll(bool singleThreaded)
{
	//Sanity checks
	if (started)
	{
		throw std::runtime_error("WorkGroups already started");
	}
	started = true;

	//TODO: Fix this; it's caused by that exception(...) trick used by the GUI in Worker::threaded_function_loop()
	if (singleThreaded && ConfigManager::GetInstance().CMakeConfig().InteractiveMode())
	{
		throw std::runtime_error("Can't run in single-threaded mode while INTERACTIVE_MODE is set.");
	}

	//Stage any Agents that will become active within the first time tick (in time for the next tick)
	nextTimeTick = 0;
	currTimeTick = 0; //Will be reset later anyway.

	//Start all workers
	tickOffset = 0; //Always start with an update.

	for (vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
	{
		(*it)->start();
	}
}

void sim_mob::WorkGroup::scheduleEntity(Entity* ag)
{
	//No-one's using DISABLE_DYNAMIC_DISPATCH anymore; we can eventually remove it.
	if (!loader)
	{
		throw std::runtime_error("Can't schedule an entity with dynamic dispatch disabled.");
	}

	//Schedule it to start later.
	loader->pending_source.push(ag);
}

void sim_mob::WorkGroup::stageEntities()
{
	//Even with dynamic dispatch, some WorkGroups simply don't manage entities.
	if (!loader)
	{
		return;
	}

	//Each Worker has its own vector of Entities to post addition requests to.
	for (vector<vector<Entity*> >::iterator outerIt = entToBeBredPerWorker.begin(); outerIt != entToBeBredPerWorker.end(); outerIt++)
	{
		for (vector<Entity*>::iterator it = outerIt->begin(); it != outerIt->end(); it++)
		{
			//schedule each Entity.
			scheduleEntity((*it));
		}

		//This worker's list of entries is clear
		outerIt->clear();
	}

	//Keep assigning the next entity until none are left.
	unsigned int nextTickMS = nextTimeTick * ConfigManager::GetInstance().FullConfig().baseGranMS();
	while (!loader->pending_source.empty() && loader->pending_source.top()->getStartTime() <= nextTickMS)
	{
		//Remove it.
		Entity* ag = loader->pending_source.top();
		loader->pending_source.pop();

		if (sim_mob::Debug::WorkGroupSemantics)
		{
			std::cout << "Staging Entity ID: " << ag->getId() << " in time for tick: " << nextTimeTick << "\n";
		}

		if (ConfigManager::GetInstance().FullConfig().RunningMidTerm())
		{
			loadPerson(ag);
		}
		else // short or long term
		{
			//Add it to our global list.
			if (loader->entity_dest.find(ag) != loader->entity_dest.end())
			{
				Warn() << "Attempting to add duplicate entity (" << ag << ") with ID: " << ag->getId() << "\n";
				continue;
			}
			loader->entity_dest.insert(ag);
			assignAWorker(ag);
		}
	}
}

void sim_mob::WorkGroup::collectRemovedEntities(std::set<sim_mob::Entity*>* removedAgents)
{
	//Even with dynamic dispatch, some WorkGroups simply don't manage entities.
	if (!loader)
	{
		return;
	}

	//Each Worker has its own vector of Entities to post removal requests to.
	for (vector<vector<Entity*> >::iterator outerIt = entToBeRemovedPerWorker.begin(); outerIt != entToBeRemovedPerWorker.end(); outerIt++)
	{
		for (vector<Entity*>::iterator it = outerIt->begin(); it != outerIt->end(); it++)
		{
			//For each Entity, find it in the list of all_agents and remove it.
			std::set<Entity*>::iterator it2 = loader->entity_dest.find(*it);
			if (it2 != loader->entity_dest.end())
			{
				loader->entity_dest.erase(it2);
			}

			//if parent existed, will inform parent to unregister this child if necessary
			if (Entity* parent = (*it)->parentEntity)
			{
				parent->unregisterChild((*it));
			}

			//If this Entity is an Agent, save its memory address.
			if (removedAgents)
			{
				Agent* ag = dynamic_cast<Agent*>(*it);
				if (ag)
				{
					removedAgents->insert(ag);
					continue;
				}
			}

			//Delete this entity. NOTE: This *only* occurs if the entity is not stored on the removedAgents list.
			delete *it;
		}

		//This worker's list of entries is clear
		outerIt->clear();
	}
}

void sim_mob::WorkGroup::assignAWorker(Entity* ag)
{
	if (ConfigManager::GetInstance().FullConfig().RunningShortTerm())
	{
		//Let the AuraManager know about this Entity.
		Agent* an_agent = dynamic_cast<Agent*>(ag);
		if (an_agent && !an_agent->isNonspatial())
		{
			AuraManager::instance().registerNewAgent(an_agent);
		}
	}

	//For now, just rely on static access to ConfigParams. (We can allow per-workgroup configuration later).
	ASSIGNMENT_STRATEGY strat = ConfigManager::GetInstance().FullConfig().defaultWrkGrpAssignment();
	if (strat == ASSIGN_ROUNDROBIN)
	{
		workers.at(nextWorkerID)->scheduleForAddition(ag);
	}
	else
	{
		getLeastLoadedWorker(workers)->scheduleForAddition(ag);
	}

	nextWorkerID = (nextWorkerID + 1) % workers.size();
}

bool sim_mob::WorkGroup::assignWorker(Entity* ag, unsigned int workerId)
{
	if (workerId >= workers.size())
	{
		return false;
	}
	workers.at(workerId)->scheduleForAddition(ag);
	return true;
}

size_t sim_mob::WorkGroup::size() const
{
	return workers.size();
}

sim_mob::Worker* sim_mob::WorkGroup::getWorker(int id)
{
	if (id < 0)
	{
		return nullptr;
	}
	return workers.at(id);
}

void sim_mob::WorkGroup::waitFrameTick(bool singleThreaded)
{
	//Sanity check
	if (!started)
	{
		throw std::runtime_error("WorkGroups not started; can't waitFrameTick()");
	}

	if (tickOffset == 0)
	{
		//If we are in single-threaded mode, pass this function call on to each Worker.
		if (singleThreaded)
		{
			for (std::vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
			{
				(*it)->perform_frame_tick();
			}
		}

		//New time tick.
		currTimeTick = nextTimeTick;
		nextTimeTick += tickStep;
		//frame_tick_barr->contribute();  //No.
	}
	else
	{
		//Tick on behalf of all your workers
		if (frame_tick_barr)
		{
			frame_tick_barr->contribute(workers.size());
		}
	}
}

void sim_mob::WorkGroup::waitFlipBuffers(bool singleThreaded, std::set<sim_mob::Entity*>* removedAgents)
{
	if (tickOffset == 0)
	{
		//If we are in single-threaded mode, pass this function call on to each Worker.
		if (singleThreaded)
		{
			for (std::vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
			{
				(*it)->perform_buff_flip();
			}
		}

		if (periodicPersonLoader && periodicPersonLoader->checkTimeForNextLoad())
		{
			periodicPersonLoader->loadPersonDemand();
		}
		//Stage Agent updates based on nextTimeTickToStage
		stageEntities();
		//Remove any Agents staged for removal.
		collectRemovedEntities(removedAgents);
	}
	else
	{
		//Tick on behalf of all your workers.
		if (buff_flip_barr)
		{
			buff_flip_barr->contribute(workers.size());
		}
	}
}

void sim_mob::WorkGroup::waitAuraManager(const std::set<sim_mob::Entity*>& removedAgents)
{
	//This barrier is optional.
	/*if (!aura_mgr_barr) {
	 return;
	 }*/

	if (tickOffset == 0)
	{
		//Update the partition manager, if we have one.
		if (partitionMgr)
		{
			partitionMgr->crossPCBarrier();
			partitionMgr->crossPCboundaryProcess(currTimeTick);
			partitionMgr->crossPCBarrier();
//			partitionMgr->outputAllEntities(currTimeTick);
		}

		//Update the aura manager, if we have one.
		if (auraMgr && ConfigManager::GetInstance().FullConfig().RunningShortTerm())
		{
			PROFILE_LOG_AURAMANAGER_UPDATE_BEGIN(profile, auraMgr, currTimeTick);

			auraMgr->update(removedAgents);

			PROFILE_LOG_AURAMANAGER_UPDATE_END(profile, auraMgr, currTimeTick);
		}

		//aura_mgr_barr->contribute();  //No.
	}
	else
	{
		//Tick on behalf of all your workers.
		if (msg_bus_barr)
		{
			msg_bus_barr->contribute(workers.size());
		}
	}
}

void sim_mob::WorkGroup::waitMacroTimeTick()
{
	//This countdown cycle is slightly different. Basically, we ONLY wait one time tick before the next update
	// period. Otherwise, we continue counting down, resetting at zero.
	if (tickOffset == 1)
	{
		//One additional wait forces a synchronization before the next major time step.
		//This won't trigger when tickOffset is 1, since it will immediately decrement to 0.
		//NOTE: Be aware that we want to "wait()", NOTE "contribute()" here. (Maybe use a boost::barrier then?) ~Seth
		if (macro_tick_barr)
		{
			macro_tick_barr->wait();
		}
	}
	else if (tickOffset == 0)
	{
		//Reset the countdown loop.
		tickOffset = tickStep;
	}

	//Continue counting down.
	tickOffset--;
}

#ifndef SIMMOB_DISABLE_MPI

void sim_mob::WorkGroup::removeAgentFromWorker(Entity* ag)
{
	ag->currWorker->migrateOut(*(ag));
}

void sim_mob::WorkGroup::addAgentInWorker(Entity * ag)
{
	int free_worker_id = getTheMostFreeWorkerID();
	getWorker(free_worker_id)->migrateIn(*(ag));
}

int sim_mob::WorkGroup::getTheMostFreeWorkerID() const
{
	int minimum_task = std::numeric_limits<int>::max();
	int minimum_index = 0;

	for (size_t i = 0; i < workers.size(); i++)
	{
		if (workers[i]->getAgentSize() < minimum_task)
		{
			minimum_task = workers[i]->getAgentSize();
			minimum_index = i;
		}
	}

	return minimum_index;
}

#endif

void sim_mob::WorkGroup::interrupt()
{
	for (std::vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++)
	{
		(*it)->interrupt();
	}
}

void sim_mob::WorkGroup::processMultiUpdateEntities(std::set<Entity*>& removedEntities)
{
	for (vector<Worker*>::iterator wrkr = workers.begin(); wrkr != workers.end(); wrkr++)
	{
		(*wrkr)->processMultiUpdateEntities(currTimeTick);
		(*wrkr)->removePendingEntities();
		//we must collect removed entities and procrastinate their deletion till they have handled all messages destined for them
		collectRemovedEntities(&removedEntities);
	}
}

void sim_mob::WorkGroup::registerLoaderEntity(Entity* loaderEntity)
{
	if (!loaderEntity)
	{
		throw std::runtime_error("loading entity passed for registration is NULL");
	}
	loaderEntities.push_back(loaderEntity);
}

void sim_mob::WorkGroup::loadPerson(Entity* person)
{
	if (person)
	{
		Entity* loaderEnt = loaderEntities[nextLoaderIdx];
		loaderEnt->registerChild(person);
		nextLoaderIdx = (nextLoaderIdx + 1) % loaderEntities.size();
	}
}
