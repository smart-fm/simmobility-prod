//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Worker.hpp"

#include "GenConfig.h"

#include <iostream>
#include <queue>
#include <sstream>
#include <algorithm>
#include <deque>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include "conf/simpleconf.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/roles/Role.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/Person.hpp"
#include "entities/profile/ProfileBuilder.hpp"
#include "network/ControlManager.hpp"
#include "logging/Log.hpp"
#include "workers/WorkGroup.hpp"
#include "util/FlexiBarrier.hpp"
#include "util/LangHelpers.hpp"
#include "message/MessageBus.hpp"

using std::set;
using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;
using namespace sim_mob;
using namespace sim_mob::event;

typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Worker::MgmtParams::MgmtParams() :
	msPerFrame(ConfigParams::GetInstance().baseGranMS()),
	ctrlMgr(ConfigParams::GetInstance().InteractiveMode()?ConfigParams::GetInstance().getControlMgr():nullptr),
	currTick(0),
	active(true)
{}


bool sim_mob::Worker::MgmtParams::extraActive(uint32_t endTick) const
{
	return endTick==0 || ((currTick-1)<endTick);
}



sim_mob::Worker::Worker(WorkGroup* parent, std::ostream* logFile,  FlexiBarrier* frame_tick, FlexiBarrier* buff_flip, FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, std::vector<Entity*>* entityBredList, uint32_t endTick, uint32_t tickStep)
    : logFile(logFile),
      frame_tick_barr(frame_tick), buff_flip_barr(buff_flip), aura_mgr_barr(aura_mgr), macro_tick_barr(macro_tick),
      endTick(endTick), tickStep(tickStep), parent(parent), entityRemovalList(entityRemovalList), entityBredList(entityBredList),
      profile(nullptr)
{
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


const std::vector<Entity*>& sim_mob::Worker::getEntities() const
{
	return managedEntities;
}


std::ostream* sim_mob::Worker::getLogFile() const
{
	return logFile;
}


void sim_mob::Worker::scheduleForAddition(Entity* entity)
{
/*	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		std::ostringstream out ;
		out << "worker::scheduleForAddition[" << this << "] calling migrateIn()\n ";
		std::cout << out.str();
		//Add it now.
		migrateIn(*entity);
	} else {*/
		//Save for later
		toBeAdded.push_back(entity);
	/*}*/
}


void sim_mob::Worker::scheduleForRemoval(Entity* entity)
{
/*	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		//Nothing to be done.
	} else {*/
		//Save for later
		toBeRemoved.push_back(entity);
	/*}*/
}

void sim_mob::Worker::scheduleForBred(Entity* entity)
{
/*	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		//Nothing to be done.
	} else {*/
		//Save for later
		toBeBred.push_back(entity);
	/*}*/
}


void sim_mob::Worker::start()
{
	//Just in case...
	loop_params = MgmtParams();

	//A Worker will silently fail to start if it has no frame_tick barrier.
	if (frame_tick_barr) {
		main_thread = boost::thread(boost::bind(&Worker::threaded_function_loop, this));
	}
}


void sim_mob::Worker::join()
{
	if (main_thread.joinable()) {
		main_thread.join();
	}
}


void sim_mob::Worker::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}

int sim_mob::Worker::getAgentSize(bool includeToBeAdded) 
{ 
	return managedEntities.size() + (includeToBeAdded?toBeAdded.size():0);
}


void sim_mob::Worker::addPendingEntities()
{
	/*if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}*/
	int i = 0;
	for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
		//Migrate its Buffered properties.
		migrateIn(**it);
                messaging::MessageBus::RegisterHandler((*it));
	}
	toBeAdded.clear();
}

void sim_mob::Worker::removePendingEntities()
{
	/*if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}*/

	for (vector<Entity*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
		//Migrate out its buffered properties.
		migrateOut(**it);

		//Remove it from our global list.
		if (!entityRemovalList) {
			throw std::runtime_error("Attempting to remove an entity from a WorkGroup that doesn't allow it.");
		}
		entityRemovalList->push_back(*it);
                messaging::MessageBus::UnRegisterHandler((*it));
	}
	toBeRemoved.clear();
}

void sim_mob::Worker::processVirtualQueues() {
#ifdef SIMMOB_USE_CONFLUXES
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		(*it)->processVirtualQueues();
	}
#endif
}

void sim_mob::Worker::outputSupplyStats(uint32_t currTick) {
#ifdef SIMMOB_USE_CONFLUXES
	for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++)
	{
		const uint32_t msPerFrame = ConfigParams::GetInstance().baseGranMS;
		timeslice currTime = timeslice(currTick, currTick*msPerFrame);
		(*it)->updateAndReportSupplyStats(currTime);
		(*it)->reportLinkTravelTimes(currTime);
		(*it)->resetSegmentFlows();
		(*it)->resetLinkTravelTimes(currTime);
	}
#endif
}

void sim_mob::Worker::breedPendingEntities()
{
	/*if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}*/

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


void sim_mob::Worker::perform_frame_tick()
{
	MgmtParams& par = loop_params;
	PROFILE_LOG_WORKER_UPDATE_BEGIN(profile, *this, par.currTick, (managedEntities.size()+toBeAdded.size()));

	//Short-circuit if we're in "pause" mode.
	if (ConfigParams::GetInstance().InteractiveMode()) {
		while (par.ctrlMgr->getSimState() == PAUSE) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
	}

	//Add Agents as required.
	addPendingEntities();
        
	//Perform all our Agent updates, etc.
	update_entities(timeslice(par.currTick, par.currTick*par.msPerFrame));

	//Remove Agents as requires
	removePendingEntities();

	// breed new children entities from parent
	//TODO: This name has *got* to change. ~Seth
	breedPendingEntities();

	PROFILE_LOG_WORKER_UPDATE_END(profile, *this, par.currTick);

	//Advance local time-step.
	par.currTick += tickStep;

	//Every so many time ticks, flush the ProfileBuilder (just so *some* output is retained if the program is killed).
	const uint32_t TickAmt = 100; //"Every X ticks"
#if defined (SIMMOB_PROFILE_ON) && defined (SIMMOB_PROFILE_WORKER_UPDATES)
	if ((par.currTick/tickStep)%TickAmt==0) {
		profile->flushLogFile();
	}
#endif

	//If a "stop" request is received in Interactive mode, end on the next 2 time ticks.
	if (ConfigParams::GetInstance().InteractiveMode()) {
		if (par.ctrlMgr->getSimState() == STOP) {
			while (par.ctrlMgr->getEndTick() < 0) {
				par.ctrlMgr->setEndTick(par.currTick+2);
			}
			endTick = par.ctrlMgr->getEndTick();
		}
	}

	//Check if we are still active or are done.
	par.active = (endTick==0 || par.currTick<endTick);
}

void sim_mob::Worker::perform_buff_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}


void sim_mob::Worker::threaded_function_loop()
{
        // Register thread on MessageBus.
        messaging::MessageBus::RegisterThread();
    
	///NOTE: Please keep this function simple. In fact, you should not have to add anything to it.
	///      Instead, add functionality into the sub-functions (perform_frame_tick(), etc.).
	///      This is needed so that singleThreaded mode can be implemented easily. ~Seth
	while (loop_params.active) {
                messaging::MessageBus::ThreadDispatchMessages();
		perform_frame_tick();

		//Now wait for our barriers. Interactive mode wraps this in a try...catch(all); hence the ifdefs.
#ifdef SIMMOB_INTERACTIVE_MODE
		//NOTE: Is catching an exception actually a good idea, or should we just rely
		//      on STRICT_AGENT_ERRORS?
		try {
#endif
			//First barrier
			if (frame_tick_barr) {
				frame_tick_barr->wait();
			}

			//Now flip all remaining data.
			perform_buff_flip();

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
			if (macro_tick_barr && loop_params.extraActive(endTick)) {
				macro_tick_barr->wait();
			}

#ifdef SIMMOB_INTERACTIVE_MODE
		} catch(...) {
			std::cout<<"thread out"<<std::endl;
			return;
		}
#endif
	}
        // Register thread from MessageBus.
        messaging::MessageBus::UnRegisterThread();
}

namespace {
///This class performs the operator() function on an Entity, and is meant to be used inside of a for_each loop.
struct EntityUpdater {
	EntityUpdater(Worker& wrk, timeslice currTime) : wrk(wrk), currTime(currTime) {}
	virtual ~EntityUpdater() {}

	Worker& wrk;
	timeslice currTime;

	virtual void operator() (sim_mob::Entity* entity) {
		UpdateStatus res = entity->update(currTime);
			if (res.status == UpdateStatus::RS_DONE) {
				//This Entity is done; schedule for deletion.
				wrk.scheduleForRemoval(entity);
			} else if (res.status == UpdateStatus::RS_CONTINUE) {
				//Still going, but we may have properties to start/stop managing
				for (set<BufferedBase*>::iterator it=res.toRemove.begin(); it!=res.toRemove.end(); it++) {
					wrk.stopManaging(*it);
				}
				for (set<BufferedBase*>::iterator it=res.toAdd.begin(); it!=res.toAdd.end(); it++) {
					wrk.beginManaging(*it);
				}
			} else {
				throw std::runtime_error("Unknown/unexpected update() return status.");
			}
	}
};

///This class extends EntityUpdater, allowing it to skip calling operator() on a certain Entity sub-class
///  (which is tested for using a dynamic_cast).
template <class Restricted>
struct RestrictedEntityUpdater : public EntityUpdater {
	RestrictedEntityUpdater(Worker& wrk, timeslice currTime) : EntityUpdater(wrk, currTime) {}
	virtual ~RestrictedEntityUpdater() {}

	virtual void operator() (sim_mob::Entity* entity) {
		//Exclude the restricted type.
		if(!dynamic_cast<Restricted*>(entity)) {
			EntityUpdater::operator ()(entity);
		}
	}
};

template <typename T>
struct ContainerDeleter {
	ContainerDeleter() {}
	virtual ~ContainerDeleter() {}

	virtual void operator() (T* t) {
		safe_delete_item(t);
	}
};
} //End un-named namespace.

void sim_mob::Worker::migrateAllOut()
{
	while (!managedEntities.empty()) {
		migrateOut(*managedEntities.back());
	}
	for (std::set<Conflux*>::iterator cfxIt = managedConfluxes.begin(); cfxIt != managedConfluxes.end(); cfxIt++) {

		migrateOutConflux(**cfxIt);
		//Debugging output
		if (Debug::WorkGroupSemantics) {
			PrintOut("Removing Conflux " << (*cfxIt)->getMultiNode()->getID() <<" from worker: " <<this <<std::endl);
		}
	}
	std::for_each(managedConfluxes.begin(), managedConfluxes.end(), ContainerDeleter<sim_mob::Conflux>()); // Delete all confluxes
}

void sim_mob::Worker::migrateOut(Entity& ag)
{
	//Sanity check
	if (ag.currWorkerProvider != this) {
		std::stringstream msg;
		msg <<"Error: Entity (" <<ag.getId() <<") has somehow switched workers: " <<ag.currWorkerProvider <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	remEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorkerProvider = nullptr;

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
		PrintOut("Removing Entity " <<ag.getId() <<" from worker: " <<this <<std::endl);
	}
}

void sim_mob::Worker::migrateOutConflux(Conflux& cfx) {
	std::deque<sim_mob::Person*> cfxPersons = cfx.getAllPersons();
	for(std::deque<sim_mob::Person*>::iterator pIt = cfxPersons.begin(); pIt != cfxPersons.end(); pIt++) {
		Person* person = *pIt;
		person->currWorkerProvider = nullptr;
		stopManaging(person->getSubscriptionList());
		Role* role = person->getRole();
		if(role){
			stopManaging(role->getDriverRequestParams().asVector());
		}
		//Debugging output
		if (Debug::WorkGroupSemantics) {
			PrintOut("Removing Entity " <<person->getId() << " from conflux: " << cfx.getMultiNode()->getID() <<std::endl);
		}
	}
	//std::for_each(cfxPersons.begin(), cfxPersons.end(), ContainerDeleter<sim_mob::Person>()); // Delete all persons

	//Now deal with the conflux itself
	cfx.currWorkerProvider = nullptr;
	stopManaging(cfx.getSubscriptionList());
}

void sim_mob::Worker::migrateIn(Entity& ag)
{
	//Sanity check
	if (ag.currWorkerProvider) {
		std::stringstream msg;
		msg <<"Error: Entity is already being managed: " <<ag.currWorkerProvider <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorkerProvider = this;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		PrintOut("Adding Entity " <<ag.getId() <<" to worker: " <<this <<"\n");
	}
}


//TODO: It seems that beginManaging() and stopManaging() can also be called during update?
//      May want to dig into this a bit more. ~Seth
void sim_mob::Worker::update_entities(timeslice currTime)
{
	//Confluxes require an additional set of updates.
	if (ConfigParams::GetInstance().UsingConfluxes()) {
		for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++) {
			(*it)->resetOutputBounds();
		}
		//All workers perform the same tasks for their set of managedConfluxes.
		std::for_each(managedConfluxes.begin(), managedConfluxes.end(), EntityUpdater(*this, currTime));

		for (std::set<Conflux*>::iterator it = managedConfluxes.begin(); it != managedConfluxes.end(); it++) {
			(*it)->updateAndReportSupplyStats(currTime);
			(*it)->reportLinkTravelTimes(currTime);
			(*it)->resetSegmentFlows();
			(*it)->resetLinkTravelTimes(currTime);
		}
	}

	//Updating of managed entities occurs regardless of whether or not confluxes are enabled.
	std::for_each(managedEntities.begin(), managedEntities.end(), RestrictedEntityUpdater<Conflux>(*this, currTime));
}


bool sim_mob::Worker::beginManagingConflux(Conflux* cf)
{
	return managedConfluxes.insert(cf).second;
}


