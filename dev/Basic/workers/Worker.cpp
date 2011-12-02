/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

#include <queue>

using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;

#include "WorkGroup.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;



//////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////


void sim_mob::Worker::addEntity(Entity* entity)
{
	//Save this entity in the data vector.
	data.push_back(entity);
}


void sim_mob::Worker::remEntity(Entity* entity)
{
	//Remove this entity from the data vector.
	typename std::vector<Entity*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end()) {
		data.erase(it);
	}
}


std::vector<Entity*>& sim_mob::Worker::getEntities() {
	return data;
}


void sim_mob::Worker::scheduleForAddition(Entity* entity)
{
	//Save for later
	toBeAdded.push_back(entity);
}


void sim_mob::Worker::scheduleForRemoval(Entity* entity)
{
	//Save for later
	toBeRemoved.push_back(entity);
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////



sim_mob::Worker::Worker(SimpleWorkGroup<Entity>* parent, ActionFunction* action, boost::barrier* internal_barr, boost::barrier* external_barr, frame_t endTick, frame_t tickStep, bool auraManagerActive)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      tickStep(tickStep),
      auraManagerActive(auraManagerActive),
      parent(parent),
      active(/*this, */false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
	this->beginManaging(&active);

	//Test
	if (!internal_barr || !external_barr) {
		throw std::runtime_error("Worker won't function correctly with a non-null barrier.");
	}
}


sim_mob::Worker::~Worker()
{
	//Clear all tracked entitites
	while (!data.empty()) {
		remEntity(data[0]);
	}

	//Clear all tracked data
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}
}


void sim_mob::Worker::start()
{
	active.force(true);
	currTick = 0;
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



void sim_mob::Worker::barrier_mgmt()
{
	for (;active.get();) {
		//Add Agents as required.
		for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
			//Ensure we're on the same page
			WorkGroup* wg = dynamic_cast<WorkGroup*>(parent);
			if (!wg) {
				throw std::runtime_error("Simple workers cannot add Entities at arbitrary times.");
			}

			//Migrate its Buffered properties.
			migrateOut(**it);
			//wg->migrate(**it, this);
		}
		toBeAdded.clear();

		//Perform all our Agent updates, etc.
		perform_main(currTick);

		//Remove Agents as requires
		for (vector<Entity*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
			//Ensure we're on the same page
			WorkGroup* wg = dynamic_cast<WorkGroup*>(parent);
			if (!wg) {
				throw std::runtime_error("Simple workers cannot remove Entities at arbitrary times.");
			}

			//Migrate out its buffered properties.
			migrateIn(**it);
			//wg->migrate(**it, nullptr);

			//Remove it from our global list. Requires locking
			Agent* ag = dynamic_cast<Agent*>(*it);
			if (ag) {
				boost::mutex::scoped_lock local_lock(sim_mob::Agent::all_agents_lock);
				parent->agToBeRemoved.push_back(ag);
			}

			//Delete this entity
			//delete *it;  //NOTE: For now, I'm leaving it in memory to make debugging slightly eaier. ~Seth
		}
		toBeRemoved.clear();

		//Advance local time-step. This must be done before the barrier or "active" could get out of sync.
		currTick += tickStep;
		this->active.set(endTick==0 || currTick<endTick);

		if (internal_barr) {
			internal_barr->wait();
		}

		//Now flip all remaining data.
		perform_flip();

		if (external_barr) {
			external_barr->wait();
		}

        // Wait for the AuraManager
		if (auraManagerActive) {
			if (external_barr) {
				external_barr->wait();
			}
		}
	}
}



void sim_mob::Worker::migrateOut(Entity& ag)
{
	//Sanity check
	if (ag.currWorker != this) {
		throw std::runtime_error("Error: Entity has somehow switched workers.");
	}

	//Simple migration
	remEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = nullptr;

	//Remove this entity's Buffered<> types from our list
	stopManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		Agent* agent = dynamic_cast<Agent*>(&ag);
		if (agent && dynamic_cast<Person*>(agent)) {
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<"Removing Agent " <<agent->getId() <<" from worker: " <<this <<std::endl;
		}
	}
}



void sim_mob::Worker::migrateIn(Entity& ag)
{
	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = this;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		Agent* agent = dynamic_cast<Agent*>(&ag);
		if (agent && dynamic_cast<Person*>(agent)) {
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<"Adding Agent " <<agent->getId() <<" to worker: " <<this <<" at requested time: " <<agent->startTime <<std::endl;
		}
	}
}



void sim_mob::Worker::perform_main(frame_t frameNumber)
{
	if (action) {
		(*action)(*this, frameNumber);
	}
}


void sim_mob::Worker::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}


