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
	ShortestPathWorker(Worker<Agent>::actionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, unsigned int endTick=0);
	virtual ~ShortestPathWorker() {}

	virtual void perform_main();

};


}
