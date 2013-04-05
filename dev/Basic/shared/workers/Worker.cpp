/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

#include "GenConfig.h"

#include <queue>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>


using std::set;
using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;

#include "workers/WorkGroup.hpp"
#include "util/OutputUtil.hpp"
#include "conf/simpleconf.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Worker::Worker(WorkGroup* parent, FlexiBarrier* frame_tick, FlexiBarrier* buff_flip, FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, std::vector<Entity*>* entityBredList, uint32_t endTick, uint32_t tickStep)
    : BufferedDataManager(),
      frame_tick_barr(frame_tick), buff_flip_barr(buff_flip), aura_mgr_barr(aura_mgr), macro_tick_barr(macro_tick),
      endTick(endTick),
      tickStep(tickStep),
      parent(parent),
      entityRemovalList(entityRemovalList),
      entityBredList(entityBredList),
      debugMsg(std::stringstream::out),
      profile(nullptr)
{
	//Currently, we need at least these two barriers or we will get synchronization problems.
	// (Internally, though, we don't technically need them.)
	if (!frame_tick || !buff_flip) {
		throw std::runtime_error("Can't create a Worker with a null frame_tick or buff_flip barrier.");
	}

	//Initialize our profile builder, if applicable.
	if (ConfigParams::GetInstance().ProfileWorkerUpdates()) {
		profile = new ProfileBuilder();
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

	//Clear/write our Profile log data
	safe_delete_item(profile);
}


void sim_mob::Worker::addEntity(Entity* entity)
{
	//Save this entity in the data vector.
	managedEntities.push_back(entity);
	std::ostringstream out ;
	out <<  "worker [" << this << "] addentity[" << entity << ":" << entity->getId() << "] Done\n"<< std::endl;
//	std::cout << out.str();

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
		std::ostringstream out ;
		out << "worker::scheduleForAddition[" << this << "] calling migrateIn()\n ";
		std::cout << out.str();
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

void sim_mob::Worker::scheduleForBred(Entity* entity)
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		//Nothing to be done.
	} else {
		//Save for later
		toBeBred.push_back(entity);
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
	int i = 0;
	std::ostringstream out ;
	out.str("");
	for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
//		out << "Worker[" << this << "]::addPendingEntities->iteration " << i++ << " calling migrateIn\n";
		std::cout << out.str();
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

void sim_mob::Worker::breedPendingEntities()
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}

	for (vector<Entity*>::iterator it=toBeBred.begin(); it!=toBeBred.end(); it++) {
		//Remove it from our global list.
		if (!entityBredList) {
			throw std::runtime_error("Attempting to breed an entity from parent that doesn't allow it.");
		}

		//(*it)->currWorker = this;
		entityBredList->push_back(*it);
	}
	toBeBred.clear();
}


void sim_mob::Worker::barrier_mgmt()
{
	///TODO: Using ConfigParams here is risky, since unit-tests may not have access to an actual config file.
	///      We might want to remove this later, but most of our simulator relies on ConfigParams anyway, so
	///      this will be a major cleanup effort anyway.
	const uint32_t msPerFrame = ConfigParams::GetInstance().baseGranMS;

	sim_mob::ControlManager* ctrlMgr = nullptr;
	if (ConfigParams::GetInstance().InteractiveMode()) {
		ctrlMgr = ConfigParams::GetInstance().getControlMgr();
	}

	uint32_t currTick = 0;
#ifdef SIMMOB_INTERACTIVE_MODE
	for (bool active=true; active;) {
		//Short-circuit if we're in "pause" mode.
		if (ctrlMgr->getSimState() == PAUSE) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
			continue;
		}

		//Sleep again. (Why? ~Seth)
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));

		//Add Agents as required.
		addPendingEntities();

		//Perform all our Agent updates, etc.
		perform_main(timeslice(currTick, currTick*msPerFrame));

		//Remove Agents as requires
		removePendingEntities();

		//Advance local time-step.
		currTick += tickStep;

		//get stop cmd, stop loop
		if (ctrlMgr->getSimState() == STOP) {
			while (ctrlMgr->getEndTick() < 0) {
				ctrlMgr->setEndTick(currTick+2);
			}
			endTick = ctrlMgr->getEndTick();
		}
		active = (endTick==0 || currTick<endTick);

		//NOTE: Is catching an exception actually a good idea, or should we just rely
		//      on STRICT_AGENT_ERRORS?
		try {
			//First barrier
			if (frame_tick_barr) {
				frame_tick_barr->wait();
			}

			//Now flip all remaining data.
			perform_flip();

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
		} catch(...) {
			std::cout<<"thread out"<<std::endl;
			return;
		}

	}
#else
	for (bool active=true; active;) {
		PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, *this, currTick, (managedEntities.size()+toBeAdded.size()));

		//Add Agents as required.
		addPendingEntities();

		//Perform all our Agent updates, etc.
		perform_main(timeslice(currTick, currTick*msPerFrame));

		//Remove Agents as requires
		removePendingEntities();

		// breed new children entities from parent
		breedPendingEntities();

		PROFILE_LOG_WORKER_UPDATE_END(profile, *this, currTick);

		//Advance local time-step.
		currTick += tickStep;
		active = (endTick==0 || currTick<endTick);

		//First barrier
		if (frame_tick_barr) {
			frame_tick_barr->wait();
		}

		 //Now flip all remaining data.
		perform_flip();

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
#endif
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

	//TODO: This should be integrated into Person::getSubscriptionList()
	Person* person = dynamic_cast<Person*>(&ag);
	if(person)	{
		Role* role = person->getRole();
		if(role){
			stopManaging(role->getDriverRequestParams().asVector());
		}
	}

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		LogOut("Removing Entity " <<ag.getId() <<" from worker: " <<this <<std::endl);
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
	std::stringstream out;
	out << "worker [" << this << "]::migrateIn calling addEntity for agent[" << &ag<< ":" << ag.getId() << "]" << std::endl;
//	std::cout << out.str();
	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = this;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		LogOut("Adding Entity " <<ag.getId() <<" to worker: " <<this <<std::endl);
	}
}



//TODO: It seems that beginManaging() and stopManaging() can also be called during update?
//      May want to dig into this a bit more. ~Seth
void sim_mob::Worker::perform_main(timeslice currTime)
{
#ifndef SIMMOB_USE_CONFLUXES

	 //All Entity workers perform the same tasks for their set of managedEntities.
	for (vector<Entity*>::iterator it=managedEntities.begin(); it!=managedEntities.end(); it++) {
//		std::cout<< "calling a worker(" << this <<")::perform_main at frame " << currTime.frame() << std::endl;
		UpdateStatus res = (*it)->update(currTime);
		if (res.status == UpdateStatus::RS_DONE) {
			//This Entity is done; schedule for deletion.
			scheduleForRemoval(*it);

			//xuyan:it can be removed from Sim-Tree
			(*it)->can_remove_by_RTREE = true;
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
#else

	//All workers perform the same tasks for their set of managedConfluxes.
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		UpdateStatus res = (*it)->update(currTime);

		if (res.status == UpdateStatus::RS_DONE) {
			//This Entity is done; schedule for deletion.
			scheduleForRemoval(*it);
		}
		else if (res.status == UpdateStatus::RS_CONTINUE) {
			//Still going, but we may have properties to start/stop managing
			for (set<BufferedBase*>::iterator it=res.toRemove.begin(); it!=res.toRemove.end(); it++) {
				stopManaging(*it);
			}
			for (set<BufferedBase*>::iterator it=res.toAdd.begin(); it!=res.toAdd.end(); it++) {
				beginManaging(*it);
			}
		}
		else {
			throw std::runtime_error("Unknown/unexpected update() return status.");
		}
	}
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		(*it)->updateAndReportSupplyStats(currTime);
		(*it)->reportLinkTravelTimes(currTime);
		(*it)->resetSegmentFlows();
		(*it)->resetLinkTravelTimes(currTime);
	}

#endif
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
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		(*it)->handoverDownstreamAgents();
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
