/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "WorkGroupManager.hpp"

#include "GenConfig.h"

//For debugging
#include <stdexcept>
#include <boost/thread.hpp>
#include "util/OutputUtil.hpp"

#include "conf/simpleconf.hpp"

#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "entities/AuraManager.hpp"
#include "partitions/PartitionManager.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/misc/TripChain.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Node.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"


using std::map;
using std::vector;
using std::set;
using boost::barrier;
using boost::function;

using namespace sim_mob;


WorkGroupManager::~WorkGroupManager()
{
	//First, join and delete all WorkGroups
	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		delete *it;
	}

	//Finally, delete all barriers.
	safe_delete_item(frameTickBarr);
	safe_delete_item(buffFlipBarr);
	safe_delete_item(auraMgrBarr);
}



WorkGroup* sim_mob::WorkGroupManager::newWorkGroup(unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr, PartitionManager* partitionMgr)
{
	//Sanity check
	if (frameTickBarr) { throw std::runtime_error("Can't add new work group; barriers have already been established."); }

	//Most of this involves passing paramters on to the WorkGroup itself, and then bookkeeping via static data.
	WorkGroup* res = new WorkGroup(numWorkers, numSimTicks, tickStep, auraMgr, partitionMgr);
	currBarrierCount += numWorkers;
	if (auraMgr || partitionMgr) {
		auraBarrierNeeded = true;
	}

	registeredWorkGroups.push_back(res);
	return res;
}


void sim_mob::WorkGroupManager::setSingleThreadMode(bool enable)
{
	//TODO: Mighgt be overly restrictive; perhaps only "start" needs to be preempted.
	if (!registeredWorkGroups.empty()) { throw std::runtime_error("Can't change to/from single-threaded mode once WorkGroups have been registered."); }

	singleThreaded = enable;
}


void sim_mob::WorkGroupManager::initAllGroups()
{
	//Sanity check
	if (frameTickBarr) { throw std::runtime_error("Can't init work groups; barriers have already been established."); }

	//Create a barrier for each of the three shared phases (aura manager optional)
	frameTickBarr = new FlexiBarrier(currBarrierCount);
	buffFlipBarr = new FlexiBarrier(currBarrierCount);
	if (auraBarrierNeeded) {
		auraMgrBarr = new FlexiBarrier(currBarrierCount);
	}

	//Initialize each WorkGroup with these new barriers.
	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->initializeBarriers(frameTickBarr, buffFlipBarr, auraMgrBarr);
	}
}


void sim_mob::WorkGroupManager::startAllWorkGroups()
{
	//Sanity check
	if (!frameTickBarr) { throw std::runtime_error("Can't start all WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->startAll();
	}
}


void sim_mob::WorkGroupManager::waitAllGroups()
{
	//Call each function in turn.
	waitAllGroups_FrameTick();
	waitAllGroups_FlipBuffers();
	waitAllGroups_AuraManager();
	waitAllGroups_MacroTimeTick();
}

void sim_mob::WorkGroupManager::waitAllGroups_FrameTick()
{
	//Sanity check
	if (!frameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->waitFrameTick();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	frameTickBarr->wait();
}

void sim_mob::WorkGroupManager::waitAllGroups_FlipBuffers()
{
	//Sanity check
	if (!frameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->waitFlipBuffers();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	buffFlipBarr->wait();
}

void sim_mob::WorkGroupManager::waitAllGroups_MacroTimeTick()
{
	//Sanity check
	if (!frameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->waitMacroTimeTick();
	}

	//NOTE: There is no need for a "wait()" here, since macro barriers are used internally.
}

void sim_mob::WorkGroupManager::waitAllGroups_AuraManager()
{
	//We don't need this if there's no Aura Manager.
	if (!auraMgrBarr) {
		return;
	}

	for (vector<WorkGroup*>::iterator it=registeredWorkGroups.begin(); it!=registeredWorkGroups.end(); it++) {
		(*it)->waitAuraManager();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	auraMgrBarr->wait();
}


const std::vector<sim_mob::WorkGroup*> sim_mob::WorkGroupManager::getRegisteredWorkGroups()
{
	return registeredWorkGroups;
}
