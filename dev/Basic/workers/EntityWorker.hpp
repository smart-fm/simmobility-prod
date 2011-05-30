/*
 * An "EntityWorker" updates any kind of Entity.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Entity.hpp"

class EntityWorker : public Worker {
public:
	EntityWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL);
	virtual void perform_main();
	virtual void perform_flip();
	virtual void start();

	void setSimulationEnd(unsigned int time);

	//These behave something like overrides.
	// TODO: Again, need a better way of doing this... (template maybe)
	void addEntity(Entity* entity);
	void remEntity(Entity* entity);
	std::vector<Entity*>& getEntities();

protected:
	unsigned int localTimestep;
	unsigned int simulationEnd;

	std::vector<Entity*> entities;


};
