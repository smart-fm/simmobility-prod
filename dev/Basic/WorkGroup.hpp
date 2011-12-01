/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "SimpleWorkGroup.hpp"
#include "workers/Worker.hpp"
#include "entities/Entity.hpp"

#include "util/LangHelpers.hpp"


namespace sim_mob
{


/**
 * A WorkGroup is like a SimpleWorkGroup, except that it also manages all Buffered<>
 * types that each EntityType contains. This requires EntityType to be a sub-class
 * of Entity, and to implement getSubscriptionList().
 */
class WorkGroup : public sim_mob::SimpleWorkGroup<Entity> {
public:
	WorkGroup(size_t size, unsigned int endTick=0, unsigned int tickStep=1, bool auraManagerActive=false)
	: SimpleWorkGroup<Entity>(size, endTick, tickStep, auraManagerActive) {}

protected:
	//Migrates all subscribed types.
	virtual void manageData(sim_mob::BufferedDataManager* mgr, Entity* ag, bool takeControl);

	//For debugging
	static const bool DebugOn;

public:
	//This is much more automatic than its SimpleWorkGroup counterpart.
	void migrateByID(Entity* ag, int toID);
	void migrate(Entity* ag, sim_mob::Worker<Entity>* toWorker);

};


}

