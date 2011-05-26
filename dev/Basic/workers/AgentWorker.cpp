#include "AgentWorker.hpp"

using boost::function;
using boost::barrier;


AgentWorker::AgentWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : Worker(action, internal_barr, external_barr)
{
}


/**
 * Update all agents that this Worker controls.
 */
void AgentWorker::perform_main() {

}


/**
 * Flip the memory used to store each agent's properties.
 */
void AgentWorker::perform_flip() {

}
