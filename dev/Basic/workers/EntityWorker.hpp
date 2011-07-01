#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Entity.hpp"

namespace sim_mob
{


/**
 * Handles groups of Entities; calls their "update" functions in the main loop.
 *
 * \note
 * This class currently does nothing; everything is done with functional pointers
 * in the Worker class.
 */
class EntityWorker : public Worker<Entity> {
public:
	EntityWorker(Worker<Entity>::actionFunction* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);
	virtual ~EntityWorker() {}

	virtual void perform_main(frame_t frameNumber);

};


}
