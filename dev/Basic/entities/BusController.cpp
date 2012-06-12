/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusController.hpp"

using std::vector;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::BusController::BusController(const MutexStrategy& mtxStrat, int id) :
		Agent(mtxStrat, id), frameNumberCheck(0)
{

}

sim_mob::BusController::~BusController()
{

}

UpdateStatus sim_mob::BusController::update(frame_t frameNumber)
{

}

void sim_mob::BusController::buildSubscriptionList(vector<BufferedBase*>& subsList)
{

}
