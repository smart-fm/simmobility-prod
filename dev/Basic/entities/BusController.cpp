/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BusController.hpp"

//using std::vector;
using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;
//BusController sim_mob::BusController::instance_;
sim_mob::BusController busctrller(ConfigParams::GetInstance().mutexStategy, 0);

sim_mob::BusController::BusController(const MutexStrategy& mtxStrat, int id) :
	Agent(mtxStrat, id),frameNumberCheck(0), tickStep(1), nextTimeTickToStage(0), firstFrameTick(true), TobeOutput(false)
{

}

sim_mob::BusController::~BusController() {
	//Clear all tracked entitites
	if (!managedBuses.empty()) {
		std::vector<Bus*>::iterator it;
		for (it = managedBuses.begin(); it != managedBuses.end(); ++it) {
			delete *it;
		}
		managedBuses.clear();
	}
	if(currWorker) {
		//Update our Entity's pointer after it migrated out
		currWorker = nullptr;
	}
}

void sim_mob::BusController::addBus(Bus* bus)
{
	managedBuses.push_back(bus);
}

void sim_mob::BusController::remBus(Bus* bus)
{
	std::vector<Bus*>::iterator it = std::find(managedBuses.begin(), managedBuses.end(), bus);
	if (it!=managedBuses.end()) {
		managedBuses.erase(it);
	}
}

void sim_mob::BusController::updateBusInformation(DPoint pt) {
	posBus = pt;
	std::cout<<"Report Given Bus postion: --->("<<posBus.x<<","<<posBus.y<<")"<<std::endl;
}

void sim_mob::BusController::DispatchInit()
{

}

void sim_mob::BusController::DispatchFrameTick(frame_t frameTick)
{
//	nextTimeTickToStage += tickStep;
//	unsigned int nextTickMS = nextTimeTickToStage*ConfigParams::GetInstance().baseGranMS;
//	while (!pending_Entity.empty() && pending_Entity.top().start <= nextTickMS) {
//		Person* ag = Person::GeneratePersonFromPending(pending_Entity.top());
//
//		//std::cout <<"Check: " <<loader->pending_source.top().manualID <<" => " <<ag->getId() <<std::endl;
//		//throw 1;
//
//		pending_Entity.pop();

//		if (sim_mob::Debug::WorkGroupSemantics) {
//			std::cout <<"Staging agent ID: " <<ag->getId() <<" in time for tick: " <<nextTimeTickToStage <<"\n";
//		}

//		//Add it to our global list.
//		agentWorkers->loader->entity_dest.push_back(ag);
// 		add a virtual Bus here  just like the real bus to be assigned to the Person
//
//		//Find a worker to assign this to and send it the Entity to manage.
//		agentWorkers->assignAWorker(ag);
}

UpdateStatus sim_mob::BusController::update(frame_t frameNumber)
{
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
		profile.logAgentUpdateBegin(*this, frameNumber);
#endif

	UpdateStatus retVal(UpdateStatus::RS_CONTINUE);
#ifndef SIMMOB_STRICT_AGENT_ERRORS
	try {
#endif
		//First, we need to retrieve an UpdateParams subclass appropriate for this Agent.
		unsigned int currTimeMS = frameNumber * ConfigParams::GetInstance().baseGranMS;

		//Has update() been called early?
		if(currTimeMS < getStartTime()) {
			//This only represents an error if dynamic dispatch is enabled. Else, we silently skip this update.
			if (!ConfigParams::GetInstance().DynamicDispatchDisabled()) {
				std::stringstream msg;
				msg << "Agent(" << getId() << ") specifies a start time of: " << getStartTime()
						<< " but it is currently: " << currTimeMS
						<< "; this indicates an error, and should be handled automatically.";
				throw std::runtime_error(msg.str().c_str());
			}
			return UpdateStatus::Continue;
		}

		if (firstFrameTick) {
			frame_init(frameNumber);
			firstFrameTick = false;
		}

		//save the output, if no buscontroller in the loadorder, no output
		if (TobeOutput) {
			frame_tick_output(frameNumber);
		}

//Respond to errors only if STRICT is off; otherwise, throw it (so we can catch it in the debugger).
#ifndef SIMMOB_STRICT_AGENT_ERRORS
} catch (std::exception& ex) {
#ifdef SIMMOB_AGENT_UPDATE_PROFILE
		profile.logAgentException(*this, frameNumber, ex);
#endif

		//Add a line to the output file.
#ifndef SIMMOB_DISABLE_OUTPUT
		std::stringstream msg;
		msg <<"Error updating BusController[" <<getId() <<"], will be removed from the simulation.";
		msg <<"Current frame is: " << frameNumber << "\n";
		msg <<ex.what();
		LogOut(msg.str() <<std::endl);
#endif
		TobeOutput = false;
}
#endif

#ifdef SIMMOB_AGENT_UPDATE_PROFILE
	profile.logAgentUpdateEnd(*this, frameNumber);
#endif
	return retVal;
}

void sim_mob::BusController::frame_init(frame_t frameNumber)
{
	frameNumberCheck = 0;
}

void sim_mob::BusController::frame_tick_output(frame_t frameNumber)
{
#ifndef SIMMOB_DISABLE_OUTPUT
	LogOut("(\"BusController\""
			<<","<<frameNumber
			<<","<<getId()
			<<",{"
			<<"\"managedBuses size\":\""<<static_cast<int>(managedBuses.size())
			<<"\",\"Bus_xPos\":\""<<static_cast<int>(posBus.x)
			<<"\",\"Bus_yPos\":\""<<static_cast<int>(posBus.y)
			<<"\"})"<<std::endl);
#endif
}

void sim_mob::BusController::buildSubscriptionList(std::vector<BufferedBase*>& subsList)
{
	Agent::buildSubscriptionList(subsList);
}
