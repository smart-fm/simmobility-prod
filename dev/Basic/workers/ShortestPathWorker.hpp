/*
 * A "ShortestPathWorker" operates directly on Agents, but instead of calling "update",
 *     it performs its own function.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "EntityWorker.hpp"
#include "../entities/Entity.hpp"
#include "../entities/Agent.hpp"

class ShortestPathWorker : public EntityWorker {
public:
	ShortestPathWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL);
	virtual void perform_main();

};
