/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusController.hpp"

//using std::vector;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;
BusController sim_mob::BusController::instance_;

sim_mob::BusController::BusController(const MutexStrategy& mtxStrat, int id) :
	Agent(mtxStrat, id), frameNumberCheck(0)
{

}

sim_mob::BusController::~BusController() {
	//Clear all tracked entitites
	if (!managedBuses.empty()) {
		std::vector<Bus*>::iterator it;
		Bus* pBus = NULL;
		for (it = managedBuses.begin(); it != managedBuses.end(); ++it) {
			pBus = (*it);
			delete pBus;
			pBus = NULL;
			managedBuses.erase(it);
		}
	}
}
//
void sim_mob::BusController::update(DPoint pt) {
	posBus = pt;
	std::cout<<"Report Given Bus postion: --->("<<posBus.x<<","<<posBus.y<<")"<<std::endl;
}

UpdateStatus sim_mob::BusController::update(frame_t frameNumber)
{

}

//void sim_mob::BusController::buildSubscriptionList(vector<BufferedBase*>& subsList)
//{
//
//}
