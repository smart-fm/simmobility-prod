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
#include "entities/conflux/Conflux.hpp"
#include "entities/misc/TripChain.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"

using std::map;
using std::vector;
using std::set;
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


void sim_mob::WorkGroup::StartAllWorkGroups()
{
	//Sanity check
	if (!WorkGroup::FrameTickBarr) { throw std::runtime_error("Can't start all WorkGroups; no barrier."); }

	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		(*it)->startAll();
	}
}


void sim_mob::WorkGroup::WaitAllGroups()
{
	//Call each function in turn.
	WaitAllGroups_FrameTick();
	WaitAllGroups_FlipBuffers();
	WaitAllGroups_AuraManager();
	WaitAllGroups_MacroTimeTick();
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


void sim_mob::WorkGroup::FinalizeAllWorkGroups()
{
	//First, join and delete all WorkGroups
	for (vector<WorkGroup*>::iterator it=RegisteredWorkGroups.begin(); it!=RegisteredWorkGroups.end(); it++) {
		delete *it;
	}

	//Finally, reset all properties.
	WorkGroup::RegisteredWorkGroups.clear();
	WorkGroup::CurrBarrierCount = 1;
	WorkGroup::AuraBarrierNeeded = false;
	safe_delete_item(WorkGroup::FrameTickBarr);
	safe_delete_item(WorkGroup::BuffFlipBarr);
	safe_delete_item(WorkGroup::AuraMgrBarr);
}


////////////////////////////////////////////////////////////////////
// Normal methods (non-static)
////////////////////////////////////////////////////////////////////


sim_mob::WorkGroup::WorkGroup(unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr, PartitionManager* partitionMgr) :
	numWorkers(numWorkers), numSimTicks(numSimTicks), tickStep(tickStep), auraMgr(auraMgr), partitionMgr(partitionMgr),
	tickOffset(0), started(false), currTimeTick(0), nextTimeTick(0), loader(nullptr), nextWorkerID(0),
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
		this->macro_tick_barr = new boost::barrier(numWorkers+1);
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
	//Sanity checks
	if (started) { throw std::runtime_error("WorkGroups already started"); }
	if (!frame_tick_barr) { throw std::runtime_error("Can't startAll() on a WorkGroup with no barriers."); }
	started = true;

	//Stage any Agents that will become active within the first time tick (in time for the next tick)
	nextTimeTick = 0;
	currTimeTick = 0; //Will be reset later anyway.

	//Start all workers
	tickOffset = 0; //Always start with an update.
	for (vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
		(*it)->start();
	}
}


void sim_mob::WorkGroup::scheduleEntity(Agent* ag)
{
	//No-one's using DISABLE_DYNAMIC_DISPATCH anymore; we can eventually remove it.
	if (!loader) { throw std::runtime_error("Can't schedule an entity with dynamic dispatch disabled."); }

	//Schedule it to start later.
	loader->pending_source.push(ag);
}


void sim_mob::WorkGroup::stageEntities()
{
	//Even with dynamic dispatch enabled, some WorkGroups simply don't manage entities.
	if (ConfigParams::GetInstance().DynamicDispatchDisabled() || !loader) {
		return;
	}

	//Keep assigning the next entity until none are left.
	unsigned int nextTickMS = nextTimeTick*ConfigParams::GetInstance().baseGranMS;
	while (!loader->pending_source.empty() && loader->pending_source.top()->getStartTime() <= nextTickMS) {
		//Remove it.
		Agent* ag = loader->pending_source.top();
		loader->pending_source.pop();

		if (sim_mob::Debug::WorkGroupSemantics) {
			std::cout <<"Staging agent ID: " <<ag->getId() <<" in time for tick: " <<nextTimeTick <<"\n";
		}

		//Call its "load" function
		//TODO: Currently, only Person::load() is called (I think there was some bug in BusController).
		//      We should really call load for ANY Agent subclass. ~Seth
		Person* a = dynamic_cast<Person*>(ag);
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
sim_mob::Worker* sim_mob::WorkGroup::locateWorker(unsigned int linkID){
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
	//Sanity check
	if (!started) { throw std::runtime_error("WorkGroups not started; can't waitFrameTick()"); }

	if (tickOffset==0) {
		//New time tick.
		currTimeTick = nextTimeTick;
		nextTimeTick += tickStep;
		//frame_tick_barr->contribute();  //No.
	} else {
		//Tick on behalf of all your workers
		frame_tick_barr->contribute(workers.size());
	}
}

void sim_mob::WorkGroup::waitFlipBuffers()
{
	if (tickOffset==0) {
		//Stage Agent updates based on nextTimeTickToStage
		stageEntities();
		//Remove any Agents staged for removal.
		collectRemovedEntities();
		//buff_flip_barr->contribute(); //No.
	} else {
		//Tick on behalf of all your workers.
		buff_flip_barr->contribute(workers.size());
	}
}

void sim_mob::WorkGroup::waitAuraManager()
{
	//This barrier is optional.
	if (!aura_mgr_barr) {
		return;
	}

	if (tickOffset==0) {
		//Update the partition manager, if we have one.
		if (partitionMgr) {
			partitionMgr->crossPCBarrier();
			partitionMgr->crossPCboundaryProcess(currTimeTick);
			partitionMgr->crossPCBarrier();
			partitionMgr->outputAllEntities(currTimeTick);
		}

		//Update the aura manager, if we have one.
		if (auraMgr) {
			auraMgr->update();
		}

		//aura_mgr_barr->contribute();  //No.
	} else {
		//Tick on behalf of all your workers.
		aura_mgr_barr->contribute(workers.size());
	}
}

void sim_mob::WorkGroup::waitMacroTimeTick()
{
	//This countdown cycle is slightly different. Basically, we ONLY wait one time tick before the next update
	// period. Otherwise, we continue counting down, resetting at zero.
	if (tickOffset==1) {
		//One additional wait forces a synchronization before the next major time step.
		//This won't trigger when tickOffset is 1, since it will immediately decrement to 0.
		//NOTE: Be aware that we want to "wait()", NOTE "contribute()" here. (Maybe use a boost::barrier then?) ~Seth
		if (macro_tick_barr) {
			macro_tick_barr->wait();  //Yes
		}
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

// providing read only access to public for RegisteredWorkGroups. AuraManager requires this
const std::vector<sim_mob::WorkGroup*> sim_mob::WorkGroup::getRegisteredWorkGroups() {
	return sim_mob::WorkGroup::RegisteredWorkGroups;
}

/*
 * This method takes a conflux and assigns it to a worker and additionally tries to assign all the adjacent
 * confluxes to the same worker.
 *
 * Future work:
 * If this assignment performs badly, we might want to think of a heuristics based optimization algorithm
 * which improves this assignment. We can indeed model this problem as a graph partitioning problem. Each
 * worker is a partition; the confluxes can be modeled as the nodes of the graph; and the edges will represent
 * the flow of vehicles between confluxes. Our objective is to minimize the (expected) flow of agents from one
 * partition to the other. We can try to fit the Kernighan-Lin algorithm or Fiduccia-Mattheyses algorithm
 * for partitioning, if it works. This is a little more complex due to the variable flow rates of vehicles
 * (edge weights); might require more thinking and research.
 *
 * TODO: Must see if this assignment is acceptable and try to optimize if necessary.
 * ~ Harish
 */
void sim_mob::WorkGroup::assignConfluxToWorkers() {
	std::set<sim_mob::Conflux*> confluxes = ConfigParams::GetInstance().getConfluxes();
	int numConfluxesPerWorker;
	for(std::vector<Worker*>::iterator i = workers.begin(); i != workers.end(); i++) {
		numConfluxesPerWorker = (int)(confluxes.size() / workers.size());
		if(numConfluxesPerWorker > 0){
			assignConfluxToWorkerRecursive((*confluxes.begin()), (*i), numConfluxesPerWorker);
		}
	}
}

bool sim_mob::WorkGroup::assignConfluxToWorkerRecursive(
		sim_mob::Conflux* conflux, sim_mob::Worker* worker,
		int numConfluxesToAddInWorker)
{
	std::set<sim_mob::Conflux*> confluxes = ConfigParams::GetInstance().getConfluxes();
	bool workerFilled = false;
	if(numConfluxesToAddInWorker > 0)
	{
		worker->managedConfluxes.insert(conflux);
		confluxes.erase(conflux);

		std::set<const sim_mob::RoadSegment*> downStreamSegs = conflux->getDownstreamSegments();

		// assign the confluxes of the downstream MultiNodes to the same worker if possible
		for(std::set<const sim_mob::RoadSegment*>::const_iterator i = downStreamSegs.begin();
				i != downStreamSegs.end() && numConfluxesToAddInWorker > 0 && confluxes.size() > 0;
				i++)
		{
			// the set container for managedConfluxes takes care of eliminating duplicates
			std::pair<std::set<Conflux*>::iterator, bool> insertResult = worker->managedConfluxes.insert((*i)->getParentConflux());

			if (insertResult.second)
			{
				// One conflux was added by the insert. So...
				confluxes.erase((*i)->getParentConflux());
				numConfluxesToAddInWorker--;

				// set the worker pointer in the Conflux
				(*i)->getParentConflux()->setParentWorker(worker);
			}
		}

		// after inserting all confluxes of the downstream segments
		if(numConfluxesToAddInWorker > 0 && confluxes.size() > 0)
		{
			// recusive call
			assignConfluxToWorkerRecursive((*confluxes.begin()), worker, numConfluxesToAddInWorker);
		}
		else
		{
			workerFilled = true;
		}
	}
	return workerFilled;
}

/**
 * Determines the first road segment of the agent and puts the agent in the corresponding conflux.
 */
void sim_mob::WorkGroup::putAgentOnConflux(Agent* ag) {
	const sim_mob::RoadSegment* rdSeg = findStartingRoadSegment(ag);
	rdSeg->getParentConflux()->addAgent(ag);
}

const sim_mob::RoadSegment* sim_mob::WorkGroup::findStartingRoadSegment(Agent* ag) {
	std::vector<const sim_mob::TripChainItem*> agTripChain = dynamic_cast<sim_mob::Person*>(ag)->getTripChain();
	const sim_mob::TripChainItem* firstItem = agTripChain.front();

	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	std::string role = rf.GetTripChainMode(firstItem);

	vector<WayPoint> path;
	const sim_mob::RoadSegment* rdSeg = nullptr;
	if (role == "driver") {
		const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
		path = StreetDirectory::instance().shortestDrivingPath(*(firstSubTrip.fromLocation), *(firstSubTrip.toLocation));
	}
	else if (role == "pedestrian") {
		const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
		path = StreetDirectory::instance().shortestWalkingPath(firstSubTrip.fromLocation->location, firstSubTrip.toLocation->location);
	}
	else if (role == "busdriver") {
		throw std::runtime_error("Not implemented. BusTrip is not in master branch yet");
	}

	 // The first WayPoint in path is the Node you start at, and the second WayPoint is the first RoadSegment
	 // you will get into.
	if(path[1].type_ == WayPoint::ROAD_SEGMENT) {
		rdSeg = path.at(1).roadSegment_;
	}

	return rdSeg;
}




