#include "IncidentManager.hpp"


#include "entities/conflux/Conflux.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/Person_MT.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "path/PathSetManager.hpp"
#include "message/MessageBus.hpp"
#include "conf/ConfigManager.hpp"
#include <boost/tokenizer.hpp>
#include "event/SystemEvents.hpp"
#include "event/args/ReRouteEventArgs.hpp"
using namespace sim_mob::event;
namespace sim_mob {
namespace medium {

IncidentManager * IncidentManager::instance = 0;
IncidentManager::IncidentManager() : Agent(ConfigManager::GetInstance().FullConfig().mutexStategy())
{
}

void IncidentManager::setDisruptions(std::vector<DisruptionParams>& disruptions)
{
	this->disruptions = disruptions;
}

void IncidentManager::publishDisruption(timeslice now)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	DailyTime& simStart = cfg.simulation.simStartTime;
	DailyTime current = DailyTime(now.ms() + simStart.getValue());
	std::vector<DisruptionParams>::iterator it = disruptions.begin();
	while (it != disruptions.end()) {
		if ((*it).startTime.getValue() < current.getValue()) {
			messaging::MessageBus::PublishEvent(event::EVT_CORE_MRT_DISRUPTION, this,
					messaging::MessageBus::EventArgsPtr(new event::DisruptionEventArgs(*it)));
			it = disruptions.erase(it);
		} else {
			it++;
		}
	}
}


Entity::UpdateStatus IncidentManager::frame_init(timeslice now){
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus IncidentManager::frame_tick(timeslice now){
	publishDisruption(now);
	return UpdateStatus::Continue;
}

bool IncidentManager::isNonspatial(){
	return true;
}


IncidentManager * IncidentManager::getInstance()
{
	if(!instance){
		instance = new IncidentManager();
	}
	return instance;
}
}

}
