/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

using std::vector;
using boost::barrier;
using boost::function;

#include "WorkGroup.hpp"
#include "entities/Agent.hpp"

using namespace sim_mob;



//////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////

template <class EntityType>
void sim_mob::Worker<EntityType>::addEntity(EntityType* entity)
{
	//Save this entity in the data vector.
	data.push_back(entity);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::remEntity(EntityType* entity)
{
	//Remove this entity from the data vector.
	typename std::vector<EntityType*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end()) {
		data.erase(it);
	}
}

template <class EntityType>
std::vector<EntityType*>& sim_mob::Worker<EntityType>::getEntities() {
	return data;
}

template <class EntityType>
void sim_mob::Worker<EntityType>::scheduleForRemoval(EntityType* entity)
{
	toBeRemoved.push_back(entity);
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::Worker<EntityType>::Worker(ActionFunction* action, boost::barrier* internal_barr, boost::barrier* external_barr, frame_t endTick, frame_t tickStep, bool auraManagerActive)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      tickStep(tickStep),
      auraManagerActive(auraManagerActive),
      active(/*this, */false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
	this->beginManaging(&active);
}

template <class EntityType>
sim_mob::Worker<EntityType>::~Worker()
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

template <class EntityType>
void sim_mob::Worker<EntityType>::start()
{
	active.force(true);
	currTick = 0;
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}

template <class EntityType>
void sim_mob::Worker<EntityType>::join()
{
	main_thread.join();
}

template <class EntityType>
void sim_mob::Worker<EntityType>::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::barrier_mgmt()
{
	for (;active.get();) {
		perform_main(currTick);

		if (internal_barr)
			internal_barr->wait();

		//Advance local time-step
		currTick += tickStep;
		if (endTick>0 && currTick>=endTick) {
			this->active.set(false);
		}

		//Now remove and delete all Entities marked for deletion.
		for (typename std::vector<EntityType*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
			//Migrate this Entity off of its current Worker. Since its current worker is "this"
			// worker, there is no race condition.
			sim_mob::Agent::TMP_AgentWorkGroup->migrate(*it, -1);

			//Remove this Agent (if it is one) from the list of discoverable Agents.
			sim_mob::Agent* ag = dynamic_cast<sim_mob::Agent*>(*it);
			if (ag) {
				// Currently, this requires a lock.
				boost::mutex::scoped_lock local_lock(sim_mob::Agent::all_agents_lock);
				std::vector<Agent*>::iterator it = std::find(Agent::all_agents.begin(),Agent::all_agents.end(), ag);
				if (it!=Agent::all_agents.end()) {
					Agent::all_agents.erase(it);
				}
			}

			//Delete the object pointed to by our iterator
			delete (*it);
		}
		toBeRemoved.clear();

		//Now flip all remaining data.
		perform_flip();

		if (external_barr) {
			external_barr->wait();
		}

        // Wait for the AuraManager
		if (auraManagerActive) {
			if (external_barr)
				external_barr->wait();
		}
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::perform_main(frame_t frameNumber)
{
	if (action)
		(*action)(*this, frameNumber);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}



//////////////////////////////////////////////
// Manual template instantiation: Entity
//////////////////////////////////////////////
template sim_mob::Worker<sim_mob::Entity>::Worker(sim_mob::Worker<sim_mob::Entity>::ActionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, frame_t endTick=0, frame_t tickStep=0, bool auraManagerActive=false);
template sim_mob::Worker<sim_mob::Entity>::~Worker();

template void sim_mob::Worker<sim_mob::Entity>::start();
template void sim_mob::Worker<sim_mob::Entity>::interrupt();
template void sim_mob::Worker<sim_mob::Entity>::join();

template void sim_mob::Worker<sim_mob::Entity>::addEntity(Entity* entity);
template void sim_mob::Worker<sim_mob::Entity>::remEntity(Entity* entity);
template std::vector<Entity*>& sim_mob::Worker<sim_mob::Entity>::getEntities();
template void sim_mob::Worker<sim_mob::Entity>::scheduleForRemoval(Entity* entity);

template void sim_mob::Worker<sim_mob::Entity>::perform_main(frame_t frameNumber);
template void sim_mob::Worker<sim_mob::Entity>::perform_flip();
template void sim_mob::Worker<sim_mob::Entity>::barrier_mgmt();


//////////////////////////////////////////////
// Manual template instantiation: Agent
//////////////////////////////////////////////
template sim_mob::Worker<sim_mob::Agent>::Worker(sim_mob::Worker<sim_mob::Agent>::ActionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, frame_t endTick=0, frame_t tickStep=0, bool auraManagerActive=false);
template sim_mob::Worker<sim_mob::Agent>::~Worker();

template void sim_mob::Worker<sim_mob::Agent>::start();
template void sim_mob::Worker<sim_mob::Agent>::interrupt();
template void sim_mob::Worker<sim_mob::Agent>::join();

template void sim_mob::Worker<sim_mob::Agent>::addEntity(Agent* entity);
template void sim_mob::Worker<sim_mob::Agent>::remEntity(Agent* entity);
template std::vector<Agent*>& sim_mob::Worker<sim_mob::Agent>::getEntities();
template void sim_mob::Worker<sim_mob::Agent>::scheduleForRemoval(Agent* entity);

template void sim_mob::Worker<sim_mob::Agent>::perform_main(frame_t frameNumber);
template void sim_mob::Worker<sim_mob::Agent>::perform_flip();
template void sim_mob::Worker<sim_mob::Agent>::barrier_mgmt();
