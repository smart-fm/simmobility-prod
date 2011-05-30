/*
 * A "SignalStatusWorker" updates regions.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "Worker.hpp"
#include "../entities/Agent.hpp"


//////////////////////////////////////////////////////////////////////////
//TODO: It occurs to me that a general "Agent" worker is all we need; we
//      can simply subclass "Agent" (or Entity, or whatever) and then
//      just call the "update" method. There's no need to duplicate
//      the Worker subclasses.
//NOTE: Instead of inheritance, we could also use templates.
//      It shouldn't matter much in terms of performance either way.
//////////////////////////////////////////////////////////////////////////

class SignalStatusWorker : public Worker {
public:
	SignalStatusWorker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL);
	virtual void perform_main();
	virtual void perform_flip();
	virtual void start();

	void setSimulationEnd(unsigned int time);

private:
	unsigned int localTimestep;
	unsigned int simulationEnd;


};
