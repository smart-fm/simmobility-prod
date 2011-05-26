#include "AgentWorker.hpp"



AgentWorker::AgentWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : internal_barr(internal_barr), external_barr(external_barr), action(action)
{
}


/**
 * Update all agents that this Worker controls.
 */
AgentWorker::main_loop() {


}
