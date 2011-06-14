/**
 * Operates directly on Agents, but instead of calling "update",
 * it performs its own function.
 *
 * \todo
 * This should extend Worker, not EntityWorker.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "EntityWorker.hpp"
#include "../entities/Entity.hpp"
#include "../entities/Agent.hpp"

namespace sim_mob
{

class ShortestPathWorker : public EntityWorker {
public:
	ShortestPathWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);
	virtual void perform_main();

};


}
