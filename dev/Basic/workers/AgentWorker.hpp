/*
 * An "AgentWorker" updates agents.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Agent.hpp"

class AgentWorker : public Worker {
public:
	AgentWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL);
	virtual void perform_main();
	virtual void perform_flip();
	virtual void start();

	void setSimulationEnd(unsigned int time);

private:
	unsigned int localTimestep;
	unsigned int simulationEnd;


};
