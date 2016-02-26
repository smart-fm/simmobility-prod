/*
 * TrainStationAgent.cpp
 *
 *  Created on: Feb 24, 2016
 *      Author: zhang huai peng
 */

#include "TrainStationAgent.hpp"
#include "message/MT_Message.hpp"
#include "entities/roles/driver/TrainDriver.hpp"
#include "entities/Person_MT.hpp"
namespace sim_mob {
namespace medium
{
TrainStationAgent::TrainStationAgent():station(nullptr),Agent(MutexStrategy::MtxStrat_Buffered, -1) {
	// TODO Auto-generated constructor stub

}

TrainStationAgent::~TrainStationAgent() {
	// TODO Auto-generated destructor stub
}

void TrainStationAgent::setStation(const Station* station)
{
	this->station = station;
}

void TrainStationAgent::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case TRAIN_MOVETO_NEXT_PLATFORM:
	{
		const TrainDriverMessage& msg = MSG_CAST(TrainDriverMessage, message);
		trainDriver.push_back(msg.trainDriver);
		msg.trainDriver->setCurrentStatus(TrainDriver::MOVE_TO_PLATFROM);
		break;
	}
	}
}

Entity::UpdateStatus TrainStationAgent::frame_init(timeslice now)
{
	return UpdateStatus::Continue;
}
Entity::UpdateStatus TrainStationAgent::frame_tick(timeslice now)
{
	std::list<TrainDriver*>::iterator it=trainDriver.begin();
	while(it!=trainDriver.end())
	{
		callMovementFrameTick(now, *it);
		if((*it)->getCurrentStatus()==TrainDriver::ARRIVAL_AT_PLATFORM){
			(*it)->calculateDwellTime();
			(*it)->setCurrentStatus(TrainDriver::WAITING_LEAVING);
		} else if((*it)->getCurrentStatus()==TrainDriver::LEAVING_FROM_PLATFORM){
			it = trainDriver.erase(it);
			continue;
		}
		it++;
	}
	return UpdateStatus::Continue;
}
void TrainStationAgent::frame_output(timeslice now)
{

}

bool TrainStationAgent::isNonspatial()
{
	return false;
}

Entity::UpdateStatus TrainStationAgent::callMovementFrameTick(timeslice now, TrainDriver* driver)
{
	if(driver){
		driver->Movement()->frame_tick();
	}
	return UpdateStatus::Continue;
}
}
} /* namespace sim_mob */
