/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

#include <queue>
#include <sstream>


using std::set;
using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;

#include "workers/WorkGroup.hpp"
#include "util/OutputUtil.hpp"
#include "conf/simpleconf.hpp"
#include "entities/conflux/Conflux.hpp"

using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Worker::Worker(WorkGroup* parent, FlexiBarrier* frame_tick, FlexiBarrier* buff_flip, FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, frame_t endTick, frame_t tickStep)
    : BufferedDataManager(),
      frame_tick_barr(frame_tick), buff_flip_barr(buff_flip), aura_mgr_barr(aura_mgr), macro_tick_barr(macro_tick),
      endTick(endTick),
      tickStep(tickStep),
      parent(parent),
      entityRemovalList(entityRemovalList)
{
	//Currently, we need at least these two barriers or we will get synchronization problems.
	// (Internally, though, we don't technically need them.)
	if (!frame_tick || !buff_flip) {
		throw std::runtime_error("Can't create a Worker with a null frame_tick or buff_flip barrier.");
	}
}


sim_mob::Worker::~Worker()
{
	//Clear all tracked entitites
	while (!managedEntities.empty()) {
		remEntity(managedEntities.front());
	}

	//Clear all tracked data
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}
}


void sim_mob::Worker::addEntity(Entity* entity)
{
	//Save this entity in the data vector.
	managedEntities.push_back(entity);
}


void sim_mob::Worker::remEntity(Entity* entity)
{
	//Remove this entity from the data vector.
	std::vector<Entity*>::iterator it = std::find(managedEntities.begin(), managedEntities.end(), entity);
	if (it!=managedEntities.end()) {
		managedEntities.erase(it);
	}
}


std::vector<Entity*>& sim_mob::Worker::getEntities() {
	return managedEntities;
}


void sim_mob::Worker::scheduleForAddition(Entity* entity)
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		//Add it now.
		migrateIn(*entity);
	} else {
		//Save for later
		toBeAdded.push_back(entity);
	}
}


void sim_mob::Worker::scheduleForRemoval(Entity* entity)
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		//Nothing to be done.
	} else {
		//Save for later
		toBeRemoved.push_back(entity);
	}
}



void sim_mob::Worker::start()
{
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}


void sim_mob::Worker::join()
{
	main_thread.join();
}


void sim_mob::Worker::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}


void sim_mob::Worker::addPendingEntities()
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}

	for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
		//Migrate its Buffered properties.
		migrateIn(**it);
	}
	toBeAdded.clear();
}

void sim_mob::Worker::removePendingEntities()
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}

	for (vector<Entity*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
		//Migrate out its buffered properties.
		migrateOut(**it);

		//Remove it from our global list.
		if (!entityRemovalList) {
			throw std::runtime_error("Attempting to remove an entity from a WorkGroup that doesn't allow it.");
		}
		entityRemovalList->push_back(*it);
	}
	toBeRemoved.clear();
}



void sim_mob::Worker::barrier_mgmt()
{
	frame_t currTick = 0;
	for (bool active=true; active;) {
		//Add Agents as required.
		addPendingEntities();

		//Perform all our Agent updates, etc.
		perform_main(currTick);

		//Remove Agents as requires
		removePendingEntities();

		//Advance local time-step.
		currTick += tickStep;
		active = (endTick==0 || currTick<endTick);

		//First barrier
		if (frame_tick_barr) {
			frame_tick_barr->wait();
		}

		/* TODO: Uncomment this when confluxes are made configurable again. ~ Harish
		 * //Now flip all remaining data.
		perform_flip();
		*/
		// handover agents which have crossed conflux boundaries
		perform_handover();

		//Second barrier
		if (buff_flip_barr) {
			buff_flip_barr->wait();
		}

        // Wait for the AuraManager
		if (aura_mgr_barr) {
			aura_mgr_barr->wait();
		}

		//If we have a macro barrier, we must wait exactly once more.
		//  E.g., for an Agent with a tickStep of 10, we wait once at the end of tick0, and
		//  once more at the end of tick 9.
		//NOTE: We can't wait (or we'll lock up) if the "extra" tick will never be triggered.
		bool extraActive = (endTick==0 || (currTick-1)<endTick);
		if (macro_tick_barr && extraActive) {
			macro_tick_barr->wait();
		}
	}
}


void sim_mob::Worker::migrateAllOut()
{
	while (!managedEntities.empty()) {
		migrateOut(*managedEntities.back());
	}
}


void sim_mob::Worker::migrateOut(Entity& ag)
{
	//Sanity check
	if (ag.currWorker != this) {
		std::stringstream msg;
		msg <<"Error: Entity (" <<ag.getId() <<") has somehow switched workers: " <<ag.currWorker <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	remEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = nullptr;

	//Remove this entity's Buffered<> types from our list
	stopManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"Removing Entity " <<ag.getId() <<" from worker: " <<this <<std::endl;
#endif
	}
}



void sim_mob::Worker::migrateIn(Entity& ag)
{
	//Sanity check
	if (ag.currWorker) {
		std::stringstream msg;
		msg <<"Error: Entity is already being managed: " <<ag.currWorker <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = this;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"Adding Entity " <<ag.getId() <<" to worker: " <<this <<std::endl;
#endif
	}
}



//TODO: It seems that beginManaging() and stopManaging() can also be called during update?
//      May want to dig into this a bit more. ~Seth
void sim_mob::Worker::perform_main(frame_t frameNumber)
{
	/* TODO: Commenting this for now. Will have to enable this back when confluxes are configurable. ~ Harish
	 //All Entity workers perform the same tasks for their set of managedEntities.
	for (vector<Entity*>::iterator it=managedEntities.begin(); it!=managedEntities.end(); it++) {
		UpdateStatus res = (*it)->update(frameNumber);
		if (res.status == UpdateStatus::RS_DONE) {
			//This Entity is done; schedule for deletion.
			scheduleForRemoval(*it);
		} else if (res.status == UpdateStatus::RS_CONTINUE) {
			//Still going, but we may have properties to start/stop managing
			for (set<BufferedBase*>::iterator it=res.toRemove.begin(); it!=res.toRemove.end(); it++) {
				stopManaging(*it);
			}
			for (set<BufferedBase*>::iterator it=res.toAdd.begin(); it!=res.toAdd.end(); it++) {
				beginManaging(*it);
			}
		} else {
			throw std::runtime_error("Unknown/unexpected update() return status.");
		}
	}
	*/

	//All workers perform the same tasks for their set of managedConfluxes.
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		UpdateStatus res = (*it)->update(frameNumber);
	}
}

bool sim_mob::Worker::isThisLinkManaged(unsigned int linkID){
	for(vector<Link*>::iterator it=managedLinks.begin(); it!=managedLinks.end();it++){
		if((*it)->linkID==linkID){
			return true;
		}
	}
	return false;
}
void sim_mob::Worker::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}

void sim_mob::Worker::perform_handover() {
	// Agents to be handed over are in the downstream segments's SegmentStats
	typedef std::map<const sim_mob::RoadSegment*, sim_mob::AgentKeeper*> SegKeperMap;
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		SegKeperMap handoverKeepersMap = (*it)->getSegmentAgentsDownstream();
		for(SegKeperMap::iterator i = handoverKeepersMap.begin(); i != handoverKeepersMap.end(); i++)
		{
			sim_mob::Conflux* targetConflux = (*i).first->getParentConflux();
			sim_mob::AgentKeeper* downStreamAgKeeper = (*i).second;
			targetConflux->absorbAgentsAndUpdateCounts(downStreamAgKeeper);
		}
	}
}

//Methods to manage list of links managed by the worker
//added by Jenny
void sim_mob::Worker::addLink(Link* link)
{
	//Save this entity in the data vector.
	managedLinks.push_back(link);
}


void sim_mob::Worker::remLink(Link* link)
{
	//Remove this entity from the data vector.
	std::vector<Link*>::iterator it = std::find(managedLinks.begin(), managedLinks.end(), link);
	if (it!=managedLinks.end()) {
		managedLinks.erase(it);
	}
}
bool sim_mob::Worker::isLinkManaged(Link* link)
{
	//Remove this entity from the data vector.
	std::vector<Link*>::iterator it = std::find(managedLinks.begin(), managedLinks.end(), link);
	if (it!=managedLinks.end()) {
		return true;
	}
	return false;
}
