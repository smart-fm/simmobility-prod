#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "EntityWorker.hpp"
#include "../entities/Entity.hpp"
#include "../entities/Agent.hpp"

namespace sim_mob
{

/**
 * Operates directly on Agents, but instead of calling "update",
 * it performs its own function.
 *
 * \note
 * This class currently does nothing; everything is done with functional pointers
 * in the Worker class.
 */
class ShortestPathWorker : public Worker<Agent> {
public:
	ShortestPathWorker(boost::function<void(Worker<Agent>*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);
	virtual void perform_main();

};


}
