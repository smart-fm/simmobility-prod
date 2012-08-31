/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "WorkGroup.hpp"

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

using std::map;
using std::vector;
using boost::barrier;
using boost::function;

using namespace sim_mob;

//Static initializations
vector<WorkGroup*> sim_mob::WorkGroup::RegisteredWorkGroups;
unsigned int sim_mob::WorkGroup::CurrBarrierCount = 1;
bool sim_mob::WorkGroup::AuraBarrierNeeded = false;
FlexiBarrier* sim_mob::WorkGroup::FrameTickBarr = nullptr;
FlexiBarrier* sim_mob::WorkGroup::BuffFlipBarr = nullptr;
FlexiBarrier* sim_mob::WorkGroup::AuraMgrBarr = nullptr;

////////////////////////////////////////////////////////////////////
// Static methods
////////////////////////////////////////////////////////////////////

WorkGroup* sim_mob::WorkGroup::NewWorkGroup(unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr, PartitionManager* partitionMgr)
{
	//Sanity check
	if (WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't add new work group; barriers have already been established."); }

	//Most of this involves passing paramters on to the WorkGroup itself, and then bookkeeping via static data.
	WorkGroup* res = new WorkGroup(numWorkers, numSimTicks, tickStep, auraMgr, partitionMgr);
	WorkGroup::CurrBarrierCount += numWorkers;
	if (auraMgr || partitionMgr) {
		WorkGroup::AuraBarrierNeeded = true;
	}

	WorkGroup::RegisteredWorkGroups.push_back(res);
	return res;
}


void sim_mob::WorkGroup::InitAllGroups()
{
	//Sanity check
	if (WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't init work groups; barriers have already been established."); }

	//Create a barrier for each of the three shared phases (aura manager optional)
	WorkGroup::FrameTickBarr = new FlexiBarrier(WorkGroup::CurrBarrierCount);
	WorkGroup::BuffFlipBarr = new FlexiBarrier(WorkGroup::CurrBarrierCount);
	if (WorkGroup::AuraBarrierNeeded) {
		WorkGroup::AuraMgrBarr = new FlexiBarrier(WorkGroup::CurrBarrierCount);
	}

	//Initialize each WorkGroup with these new barriers.
	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->initializeBarriers(FrameTickBarr, BuffFlipBarr, AuraMgrBarr);
	}
}


void sim_mob::WorkGroup::WaitAllGroups()
{
	//Call each function in turn.
	WaitAllGroups_FrameTick();
	WaitAllGroups_FlipBuffers();
	WaitAllGroups_MacroTimeTick();
	WaitAllGroups_AuraManager();
}

void sim_mob::WorkGroup::WaitAllGroups_FrameTick()
{
	//Sanity check
	if (!WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->waitFrameTick();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	FrameTickBarr->wait();
}

void sim_mob::WorkGroup::WaitAllGroups_FlipBuffers()
{
	//Sanity check
	if (!WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->waitFlipBuffers();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	BuffFlipBarr->wait();
}

void sim_mob::WorkGroup::WaitAllGroups_MacroTimeTick()
{
	//Sanity check
	if (!WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't tick WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->waitMacroTimeTick();
	}

	//NOTE: There is no need for a "wait()" here, since macro barriers are used internally.
}

void sim_mob::WorkGroup::WaitAllGroups_AuraManager()
{
	//We don't need this if there's no Aura Manager.
	if (!WorkGroup::AuraMgrBarr) {
		return;
	}

	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->waitAuraManager();
	}

	//Here is where we actually block, ensuring a tick-wide synchronization.
	AuraMgrBarr->wait();
}


////////////////////////////////////////////////////////////////////
// Normal methods (non-static)
////////////////////////////////////////////////////////////////////


sim_mob::WorkGroup::WorkGroup(unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr, PartitionManager* partitionMgr)
	: numWorkers(numWorkers), numSimTicks(numSimTicks), tickStep(tickStep), auraMgr(auraMgr), partitionMgr(partitionMgr), loader(nullptr),
	  frame_tick_barr(nullptr), buff_flip_barr(nullptr), aura_mgr_barr(nullptr), macro_tick_barr(nullptr)
{
}


sim_mob::WorkGroup::~WorkGroup()  //Be aware that this will hang if Workers are wait()-ing. But it prevents undefined behavior in boost.
{
	for (vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
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
	this->aura_mgr_barr = aura_mgr;

	//Now's a good time to create our macro barrier too.
	if (tickStep>1) {
		this->macro_tick_barr = new FlexiBarrier(numWorkers+1);
	}
}



void sim_mob::WorkGroup::initWorkers(EntityLoadParams* loader)
{
	this->loader = loader;

	//Init our worker list-backs
	const bool UseDynamicDispatch = !ConfigParams::GetInstance().DynamicDispatchDisabled();
	if (UseDynamicDispatch) {
		entToBeRemovedPerWorker.resize(numWorkers, vector<Entity*>());
	}

	//Init the workers themselves.
	for (size_t i=0; i<numWorkers; i++) {
		std::vector<Entity*>* entWorker = UseDynamicDispatch ? &entToBeRemovedPerWorker.at(i) : nullptr;
		workers.push_back(new Worker(this, frame_tick_barr, buff_flip_barr, aura_mgr_barr, macro_tick_barr, entWorker, numSimTicks, tickStep));
	}
}



void sim_mob::WorkGroup::startAll()
{
	//Stage any Agents that will become active within the first time tick (in time for the next tick)
	nextTimeTickToStage = 0;

	//Start all workers
	tickOffset = 0; //Always start with an update.
	for (vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
		(*it)->start();
	}
}


void sim_mob::WorkGroup::scheduleEntity(Person* ent)
{
	//No-one's using DISABLE_DYNAMIC_DISPATCH anymore; we can eventually remove it.
	if (!loader) { throw std::runtime_error("Can't schedule an entity with dynamic dispatch disabled."); }

	//Schedule it to start later.
	loader->pending_source.push(ent);
}


void sim_mob::WorkGroup::stageEntities()
{
	//Even with dynamic dispatch enabled, some WorkGroups simply don't manage entities.
	if (ConfigParams::GetInstance().DynamicDispatchDisabled() || !loader) {
		return;
	}

	//Keep assigning the next entity until none are left.
	unsigned int nextTickMS = nextTimeTickToStage*ConfigParams::GetInstance().baseGranMS;
	while (!loader->pending_source.empty() && loader->pending_source.top()->getStartTime() <= nextTickMS) {
		//Remove it.
		Person* ag = loader->pending_source.top();
		loader->pending_source.pop();

		if (sim_mob::Debug::WorkGroupSemantics) {
			std::cout <<"Staging agent ID: " <<ag->getId() <<" in time for tick: " <<nextTimeTickToStage <<"\n";
		}

		//Call its "load" function
		Agent* a = dynamic_cast<Agent*>(ag);
		if (a) {
			a->load(a->getConfigProperties());
			a->clearConfigProperties();
		}

		//Add it to our global list.
		loader->entity_dest.push_back(ag);

		//Find a worker to assign this to and send it the Entity to manage.
		assignAWorker(ag);
		//in the future, replaced by
		//assignAWorkerConstraint(ag);
	}
}


void sim_mob::WorkGroup::collectRemovedEntities()
{
	//Even with dynamic dispatch enabled, some WorkGroups simply don't manage entities.
	if (ConfigParams::GetInstance().DynamicDispatchDisabled() || !loader) {
		return;
	}

	//Each Worker has its own vector of Entities to post removal requests to.
	for (vector<vector <Entity*> >::iterator outerIt=entToBeRemovedPerWorker.begin(); outerIt!=entToBeRemovedPerWorker.end(); outerIt++) {
		for (vector<Entity*>::iterator it=outerIt->begin(); it!=outerIt->end(); it++) {
			//For each Entity, find it in the list of all_agents and remove it.
			std::vector<Entity*>::iterator it2 = std::find(loader->entity_dest.begin(), loader->entity_dest.end(), *it);
			if (it2!=loader->entity_dest.end()) {
				loader->entity_dest.erase(it2);
			}

			//Delete this entity
			delete *it;
		}

		//This worker's list of entries is clear
		outerIt->clear();
	}
}

//method to randomly assign links to workers
void sim_mob::WorkGroup::assignLinkWorker(){
	std::vector<Link*> allLinks = ConfigParams::GetInstance().getNetwork().getLinks();
	//randomly assign link to worker
	//each worker is expected to manage approximately the same number of links
	for(vector<sim_mob::Link*>::iterator it = allLinks.begin(); it!= allLinks.end();it++){
		Link* link = *it;
		Worker* w = workers.at(nextWorkerID);
		w->addLink(link);
		link->setCurrWorker(w);
		nextWorkerID = (++nextWorkerID) % workers.size();
	}
	//reset nextworkerID to 0
	nextWorkerID=0;
}

//method to assign agents on same link to the same worker
void sim_mob::WorkGroup::assignAWorkerConstraint(Entity* ag){
	Agent* agent = dynamic_cast<Agent*>(ag);
	if(agent){
		if(agent->originNode){
			Link* link = agent->originNode->getLinkLoc();
			link->getCurrWorker()->scheduleForAddition(ag);
		}
		else{
			LoopDetectorEntity* loopDetector = dynamic_cast<LoopDetectorEntity*>(ag);
			if(loopDetector){
				Link* link = (loopDetector->getNode()).getLinkLoc();
				link->getCurrWorker()->scheduleForAddition(ag);
			}
		}
	}
}

//method to find the worker which manages the specified linkID
sim_mob::Worker* sim_mob::WorkGroup::locateWorker(std::string linkID){
	std::vector<Link*> allLinks = ConfigParams::GetInstance().getNetwork().getLinks();
	for(vector<sim_mob::Link*>::iterator it = allLinks.begin(); it!= allLinks.end();it++){
		Link* link = *it;
		if(link->linkID==linkID){
			return link->getCurrWorker();
		}
	}
	return nullptr;
}

void sim_mob::WorkGroup::assignAWorker(Entity* ag)
{
	workers.at(nextWorkerID++)->scheduleForAddition(ag);
	nextWorkerID %= workers.size();
}


size_t sim_mob::WorkGroup::size()
{
	return workers.size();
}




sim_mob::Worker* sim_mob::WorkGroup::getWorker(int id)
{
	if (id<0) {
		return nullptr;
	}
	return workers.at(id);
}


void sim_mob::WorkGroup::waitFrameTick()
{
	///////////////////////////////////////////////
	// TODO: We need to contribute workers.size() if we're skipping this tick.
	///////////////////////////////////////////////

	if (tickOffset==0) {
		//Stay in sync with the workers.
		nextTimeTickToStage += tickStep;
		frame_tick_barr->contribute();
	}
}

void sim_mob::WorkGroup::waitFlipBuffers()
{
	///////////////////////////////////////////////
	// TODO: We need to contribute workers.size() if we're skipping this tick.
	///////////////////////////////////////////////

	if (tickOffset==0) {
		//Stage Agent updates based on nextTimeTickToStage
		stageEntities();
		//Remove any Agents staged for removal.
		collectRemovedEntities();
		buff_flip_barr->contribute();
	}
}

void sim_mob::WorkGroup::waitAuraManager()
{
	///////////////////////////////////////////////
	// TODO: We need to contribute workers.size() if we're skipping this tick.
	///////////////////////////////////////////////

	if (aura_mgr_barr && tickOffset==0) {
		aura_mgr_barr->contribute();
	}
}

void sim_mob::WorkGroup::waitMacroTimeTick()
{
	if (tickOffset==1) {
		//One additional wait forces a synchronization before the next major time step.
		//This won't trigger when tickOffset is 1, since it will immediately decrement to 0.
		//NOTE: Be aware that we want to "wait()", NOTE "contribute()" here. (Maybe use a boost::barrier then?) ~Seth
		macro_tick_barr->wait();
	} else if (tickOffset==0) {
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

	for (size_t i = 0; i < workers.size(); i++) {
		if (workers[i]->getAgentSize() < minimum_task) {
			minimum_task = workers[i]->getAgentSize();
			minimum_index = i;
		}
	}

	return minimum_index;
}

#endif





void sim_mob::WorkGroup::interrupt()
{
	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}



