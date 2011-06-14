/**
 * Handles groups of Entities; calls their "update" functions in the main loop.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Entity.hpp"

namespace sim_mob
{


class EntityWorker : public Worker<Entity> {
public:
	EntityWorker(boost::function<void(Worker<Entity>*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);
	virtual void perform_main();

};


}
