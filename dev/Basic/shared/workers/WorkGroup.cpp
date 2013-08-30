//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WorkGroup.hpp"

#include "GenConfig.h"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <boost/thread.hpp>

#include "conf/simpleconf.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "entities/AuraManager.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/misc/TripChain.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Link.hpp"
#include "logging/Log.hpp"
#include "partitions/PartitionManager.hpp"
#include "workers/Worker.hpp"
#include "event/EventBusSystem.hpp"

using std::vector;

using namespace sim_mob;


sim_mob::WorkGroup::WorkGroup(unsigned int wgNum, unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, AuraManager* auraMgr, PartitionManager* partitionMgr) :
	wgNum(wgNum), numWorkers(numWorkers), numSimTicks(numSimTicks), tickStep(tickStep), auraMgr(auraMgr), partitionMgr(partitionMgr),
	tickOffset(0), started(false), currTimeTick(0), nextTimeTick(0), loader(nullptr), nextWorkerID(0),
	frame_tick_barr(nullptr), buff_flip_barr(nullptr), aura_mgr_barr(nullptr), macro_tick_barr(nullptr)
{
}


sim_mob::WorkGroup::~WorkGroup()  //Be aware that this will hang if Workers are wait()-ing. But it prevents undefined behavior in boost.
{
	//Delete/clear all Workers.
	for (vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
		Worker* wk = *it;
		wk->interrupt();
		wk->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		wk->migrateAllOut(); //This ensures that Agents can safely delete themselves.
		delete wk;
	}
	workers.clear();

	//Delete/close all Log files.
	for (std::list<std::ostream*>::iterator it=managed_logs.begin(); it!=managed_logs.end(); it++) {
		delete *it;
	}
	managed_logs.clear();

	//The only barrier we can delete is the non-shared barrier.
	//TODO: Find a way to statically delete the other barriers too (low priority; minor amount of memory leakage).
#ifndef SIMMOB_INTERACTIVE_MODE
	safe_delete_item(macro_tick_barr);
#endif
}


void WorkGroup::addOutputFileNames(std::list<std::string>& res) const
{
	res.insert(res.end(), logFileNames.begin(), logFileNames.end());
}


Worker* WorkGroup::GetLeastCongestedWorker(const vector<Worker*>& workers) {
	Worker* res = nullptr;
	for (vector<Worker*>::const_iterator it=workers.begin(); it!=workers.end(); it++) {
		if ((!res) || ((*it)->getAgentSize(true) < res->getAgentSize(true))) {
			res = *it;
		}
	}
	return res;
}

void sim_mob::WorkGroup::clear()
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
		entToBeBredPerWorker.resize(numWorkers, vector<Entity*>());
	}

	//Number Worker output threads something like:  "out_1_2.txt", where "1" is the WG number and "2" is the Worker number.
	std::stringstream prefixS;
	prefixS <<"out_" <<wgNum <<"_";
	std::string prefix = prefixS.str();

	//Init the workers themselves.
	for (size_t i=0; i<numWorkers; i++) {
		std::stringstream outFilePath;
		outFilePath <<prefix <<i <<".txt";
		std::ofstream* logFile = nullptr;
		if (ConfigParams::GetInstance().OutputEnabled()) {
			//TODO: Handle error case more gracefully.
			logFileNames.push_back(outFilePath.str());
			logFile = new std::ofstream(outFilePath.str().c_str());
			managed_logs.push_back(logFile);
		}

		std::vector<Entity*>* entWorker = UseDynamicDispatch ? &entToBeRemovedPerWorker.at(i) : nullptr;
		std::vector<Entity*>* entBredPerWorker = UseDynamicDispatch ? &entToBeBredPerWorker.at(i) : nullptr;
		workers.push_back(new Worker(this, logFile, frame_tick_barr, buff_flip_barr, aura_mgr_barr, macro_tick_barr, entWorker, entBredPerWorker, numSimTicks, tickStep));
	}
}



void sim_mob::WorkGroup::startAll(bool singleThreaded)
{
	//Sanity checks
	if (started) { throw std::runtime_error("WorkGroups already started"); }
	//if (!frame_tick_barr) { throw std::runtime_error("Can't startAll() on a WorkGroup with no barriers."); }
	started = true;

	//TODO: Fix this; it's caused by that exception(...) trick used by the GUI in Worker::threaded_function_loop()
	if (singleThreaded && ConfigParams::GetInstance().InteractiveMode()) {
		throw std::runtime_error("Can't run in single-threaded mode while INTERACTIVE_MODE is set.");
	}

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

	//Each Worker has its own vector of Entities to post addition requests to.
	for (vector<vector <Entity*> >::iterator outerIt=entToBeBredPerWorker.begin(); outerIt!=entToBeBredPerWorker.end(); outerIt++) {
		for (vector<Entity*>::iterator it=outerIt->begin(); it!=outerIt->end(); it++) {
			//schedule each Entity.
			scheduleEntity( dynamic_cast<Agent*>(*it) );
		}

		//This worker's list of entries is clear
		outerIt->clear();
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

		//Find a worker/conflux to assign this to and send it the Entity to manage.
		if (ConfigParams::GetInstance().UsingConfluxes()) {
			putAgentOnConflux(ag);
		} else {
			assignAWorker(ag);
		}
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

			//if parent existed, will inform parent to unregister this child if necessary
			if( Entity* parent = (*it)->parentEntity )
				parent->unregisteredChild( (*it) );

			//Delete this entity
			delete *it;
		}

		//This worker's list of entries is clear
		outerIt->clear();
	}
}

//method to randomly assign links to workers
/*void sim_mob::WorkGroup::assignLinkWorker(){
	std::vector<Link*> allLinks = ConfigParams::GetInstance().getNetwork().getLinks();
	//randomly assign link to worker
	//each worker is expected to manage approximately the same number of links
	for(vector<sim_mob::Link*>::iterator it = allLinks.begin(); it!= allLinks.end();it++){
		Link* link = *it;
		Worker* w = workers.at(nextWorkerID);
		//w->addLink(link);
		link->setCurrWorker(w);
		nextWorkerID = (++nextWorkerID) % workers.size();
	}
	//reset nextworkerID to 0
	nextWorkerID=0;
}*/

//method to assign agents on same link to the same worker
void sim_mob::WorkGroup::assignAWorkerConstraint(Entity* ag){
	Agent* agent = dynamic_cast<Agent*>(ag);
	if(agent){
		if(agent->originNode.node_){
			const Link* link = StreetDirectory::instance().getLinkLoc(agent->originNode.node_);
			link->getCurrWorker()->scheduleForAddition(ag);
		}
		else{
			LoopDetectorEntity* loopDetector = dynamic_cast<LoopDetectorEntity*>(ag);
			if(loopDetector){
				const Link* link = StreetDirectory::instance().getLinkLoc(&loopDetector->getNode());
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
	//For now, just rely on static access to ConfigParams.
	// (We can allow per-workgroup configuration later).
	ASSIGNMENT_STRATEGY strat = ConfigParams::GetInstance().defaultWrkGrpAssignment;
	if (strat == ASSIGN_ROUNDROBIN) {
		workers.at(nextWorkerID)->scheduleForAddition(ag);
	} else {
		GetLeastCongestedWorker(workers)->scheduleForAddition(ag);
	}

	//Increase "nextWorkID", even if we're not using it.
	nextWorkerID = (nextWorkerID+1)%workers.size();
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


void sim_mob::WorkGroup::waitFrameTick(bool singleThreaded)
{
	//Sanity check
	if (!started) { throw std::runtime_error("WorkGroups not started; can't waitFrameTick()"); }

	if (tickOffset==0) {
		//If we are in single-threaded mode, pass this function call on to each Worker.
		if (singleThreaded) {
			for (std::vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
				(*it)->perform_frame_tick();
			}
		}

		//New time tick.
		currTimeTick = nextTimeTick;
		nextTimeTick += tickStep;
		//frame_tick_barr->contribute();  //No.
	} else {
		//Tick on behalf of all your workers
		if (frame_tick_barr) {
			frame_tick_barr->contribute(workers.size());
		}
	}
}

void sim_mob::WorkGroup::waitFlipBuffers(bool singleThreaded)
{
	if (tickOffset==0) {
		//If we are in single-threaded mode, pass this function call on to each Worker.
		if (singleThreaded) {
			for (std::vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
				(*it)->perform_buff_flip();
			}
		}

		//Stage Agent updates based on nextTimeTickToStage
		stageEntities();
		//Remove any Agents staged for removal.
		collectRemovedEntities();
		//buff_flip_barr->contribute(); //No.

	} else {
		//Tick on behalf of all your workers.
		if (buff_flip_barr) {
			buff_flip_barr->contribute(workers.size());
		}
	}
}

void sim_mob::WorkGroup::waitAuraManager()
{
	//This barrier is optional.
	/*if (!aura_mgr_barr) {
		return;
	}*/

	if (tickOffset==0) {
		//Update the partition manager, if we have one.
		if (partitionMgr) {
			partitionMgr->crossPCBarrier();
			partitionMgr->crossPCboundaryProcess(currTimeTick);
			partitionMgr->crossPCBarrier();
//			partitionMgr->outputAllEntities(currTimeTick);
		}

		//Update the aura manager, if we have one.
		if (auraMgr) {
			auraMgr->update();
		}

		//aura_mgr_barr->contribute();  //No.
	} else {
		//Tick on behalf of all your workers.
		if (aura_mgr_barr) {
			aura_mgr_barr->contribute(workers.size());
		}
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
	for (std::vector<Worker*>::iterator it=workers.begin(); it!=workers.end(); it++) {
		(*it)->interrupt();
	}
}


/*
 * This method takes a conflux and assigns it to a worker. It additionally tries to assign all the adjacent
 * confluxes to the same worker.
 *
 * Future work:
 * If this assignment performs badly, we might want to think of a heuristics based optimization algorithm
 * which improves this assignment. We can indeed model this problem as a graph partitioning problem. Each
 * worker is a partition; the confluxes can be modeled as the nodes of the graph; and the edges will represent
 * the flow of vehicles between confluxes. Our objective is to minimize the (expected) flow of agents from one
 * partition to the other. We can try to fit the Kernighan-Lin algorithm or Fiduccia-Mattheyses algorithm
 * for partitioning, if it works. This is a little more complex due to the variable flow rates of vehicles
 * (edge weights); might require more thinking.
 *
 * TODO: Must see if this assignment is acceptable and try to optimize if necessary.
 * ~ Harish
 */
void sim_mob::WorkGroup::assignConfluxToWorkers() {
	std::set<sim_mob::Conflux*>& confluxes = ConfigParams::GetInstance().getConfluxes();
	int numConfluxesPerWorker = (int)(confluxes.size() / workers.size());
	for(std::vector<Worker*>::iterator i = workers.begin(); i != workers.end(); i++) {
		if(numConfluxesPerWorker > 0){
			assignConfluxToWorkerRecursive((*confluxes.begin()), (*i), numConfluxesPerWorker);
		}
	}
	if(confluxes.size() > 0) {
		//There can be up to (workers.size() - 1) confluxes for which the parent worker is unassigned.
		//Assign these to the last worker which has all its upstream confluxes.
		sim_mob::Worker* worker = workers.back();
		for(std::set<sim_mob::Conflux*>::iterator i = confluxes.begin(); i!=confluxes.end(); i++) {
			if (worker->beginManagingConflux(*i)) {
				(*i)->setParentWorker(worker);
				(*i)->currWorkerProvider = worker;
			}
		}
		confluxes.clear();
	}

	for(std::vector<Worker*>::iterator iWorker = workers.begin(); iWorker != workers.end(); iWorker++) {
		for(std::set<Conflux*>::iterator iConflux = (*iWorker)->managedConfluxes.begin(); iConflux != (*iWorker)->managedConfluxes.end(); iConflux++) {
			// begin managing properties of the conflux
			(*iWorker)->beginManaging((*iConflux)->getSubscriptionList());
		}
	}
}

void sim_mob::WorkGroup::processVirtualQueues() {
	for(vector<Worker*>::iterator wrkr = workers.begin(); wrkr != workers.end(); wrkr++) {
		(*wrkr)->processVirtualQueues();
	}
}

void sim_mob::WorkGroup::outputSupplyStats() {
	for(vector<Worker*>::iterator wrkr = workers.begin(); wrkr != workers.end(); wrkr++) {
		(*wrkr)->outputSupplyStats(currTimeTick);
	}
}

bool sim_mob::WorkGroup::assignConfluxToWorkerRecursive(
		sim_mob::Conflux* conflux, sim_mob::Worker* worker,
		int numConfluxesToAddInWorker)
{
	typedef std::set<const sim_mob::RoadSegment*> SegmentSet;

	std::set<sim_mob::Conflux*>& confluxes = ConfigParams::GetInstance().getConfluxes();
	bool workerFilled = false;

	if(numConfluxesToAddInWorker > 0)
	{
		if (worker->beginManagingConflux(conflux)) {
			confluxes.erase(conflux);
			numConfluxesToAddInWorker--;
			conflux->setParentWorker(worker);
			conflux->currWorkerProvider = worker;
		}

		SegmentSet downStreamSegs = conflux->getDownstreamSegments();

		// assign the confluxes of the downstream MultiNodes to the same worker if possible
		for(SegmentSet::const_iterator i = downStreamSegs.begin();
				i != downStreamSegs.end() && numConfluxesToAddInWorker > 0 && confluxes.size() > 0;
				i++)
		{
			if(!(*i)->getParentConflux()->getParentWorker()) {
				// insert this conflux if it has not already been assigned to another worker
				// the set container for managedConfluxes takes care of eliminating duplicates
				if (worker->beginManagingConflux((*i)->getParentConflux()))
				{
					// One conflux was added by the insert. So...
					confluxes.erase((*i)->getParentConflux());
					numConfluxesToAddInWorker--;
					// set the worker pointer in the Conflux
					(*i)->getParentConflux()->setParentWorker(worker);
					(*i)->getParentConflux()->currWorkerProvider = worker;
				}
			}
		}

		// after inserting all confluxes of the downstream segments
		if(numConfluxesToAddInWorker > 0 && confluxes.size() > 0)
		{
			// recusive call
			workerFilled = assignConfluxToWorkerRecursive((*confluxes.begin()), worker, numConfluxesToAddInWorker);
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
	sim_mob::Person* person = dynamic_cast<sim_mob::Person*>(ag);
	if(person) {
		const sim_mob::RoadSegment* rdSeg = findStartingRoadSegment(person);
		if(rdSeg) {
			rdSeg->getParentConflux()->addAgent(person,rdSeg);
		}
		else {
			Print() << "\n Agent ID: " << person->getId() << "| Agent DB_id:" << person->getDatabaseId() << " : has no Path. Not added into the simulation";
		}
	}
}

const sim_mob::RoadSegment* sim_mob::WorkGroup::findStartingRoadSegment(Person* p) {
	/*
	 * TODO: This function must be re-written to get the starting segment without establishing the entire path.
	 */
	std::vector<sim_mob::TripChainItem*> agTripChain = p->getTripChain();
	const sim_mob::TripChainItem* firstItem = agTripChain.front();

	const RoleFactory& rf = ConfigParams::GetInstance().getRoleFactory();
	std::string role = rf.GetTripChainMode(firstItem);

	StreetDirectory& stdir = StreetDirectory::instance();

	vector<WayPoint> path;
	const sim_mob::RoadSegment* rdSeg = nullptr;
	if (role == "driver") {
		const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
		path = stdir.SearchShortestDrivingPath(stdir.DrivingVertex(*firstSubTrip.fromLocation.node_), stdir.DrivingVertex(*firstSubTrip.toLocation.node_));
	}
	else if (role == "pedestrian") {
		const sim_mob::SubTrip firstSubTrip = dynamic_cast<const sim_mob::Trip*>(firstItem)->getSubTrips().front();
		path = stdir.SearchShortestWalkingPath(stdir.WalkingVertex(*firstSubTrip.fromLocation.node_), stdir.WalkingVertex(*firstSubTrip.toLocation.node_));
	}
	else if (role == "busdriver") {
		//throw std::runtime_error("Not implemented. BusTrip is not in master branch yet");
		const BusTrip* bustrip =dynamic_cast<const BusTrip*>(*(p->currTripChainItem));
		vector<const RoadSegment*> pathRoadSeg = bustrip->getBusRouteInfo().getRoadSegments();
		std::cout << "BusTrip path size = " << pathRoadSeg.size() << std::endl;
		std::vector<const RoadSegment*>::iterator itor;
		for(itor=pathRoadSeg.begin(); itor!=pathRoadSeg.end(); itor++){
			path.push_back(WayPoint(*itor));
		}
	}

	/*
	 * path.size() > 0 is checked because SimMobility is not fully equipped to load all feasible paths in the entire Singapore network.
	 * Sometimes, due to network issues, the shortest path algorithm may fail to return a path.
	 * TODO: This condition check must be removed when the network issues are fixed. ~ Harish
	 */
	if(path.size() > 0) {
		 // The first WayPoint in path is the Node you start at, and the second WayPoint is the first RoadSegment
		 // you will get into.
		if (role == "busdriver") {
			if(path[0].type_ == WayPoint::ROAD_SEGMENT) {
			rdSeg = path.at(0).roadSegment_;
			}
		}
		else {
			if(path[1].type_ == WayPoint::ROAD_SEGMENT) {
				rdSeg = path.at(1).roadSegment_;
			}
		}
	}

	return rdSeg;
}




