/**
 * Handles groups of Entities; calls their "update" functions in the main loop.
 *
 * \todo
 * The add/remEntity and related functions should be expressed through templates.
 *
 * \todo
 * Perhaps we can specialize this template when we extend it? Then we could use it exactly
 * the same way we're using it now.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Entity.hpp"

namespace sim_mob
{


class EntityWorker : public Worker {
public:
	EntityWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);
	virtual void perform_main();
	//virtual void perform_flip();
	//virtual void start();

	//void setSimulationEnd(unsigned int time);

	//These behave something like overrides.
	// TODO: Again, need a better way of doing this... (template maybe)
	void addEntity(Entity* entity);
	void remEntity(Entity* entity);
	std::vector<Entity*>& getEntities();

protected:
	std::vector<Entity*> entities;


};


}
