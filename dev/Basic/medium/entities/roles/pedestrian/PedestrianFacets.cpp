/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

namespace sim_mob {

namespace medium {

PedestrianBehavior::PedestrianBehavior(sim_mob::Person* parentAgent) :
		BehaviorFacet(parentAgent), parentPedestrian(nullptr)
{

}

PedestrianBehavior::~PedestrianBehavior()
{

}

PedestrianMovement::PedestrianMovement(sim_mob::Person* parentAgent):
		MovementFacet(parentAgent), parentPedestrian(nullptr), totalTimeToCompleteMS(0), walkSpeed(200)
{

}

PedestrianMovement::~PedestrianMovement()
{

}

void PedestrianMovement::setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian){
	this->parentPedestrian = parentPedestrian;
}

void PedestrianBehavior::setParentPedestrian(sim_mob::medium::Pedestrian* parentPedestrian){
	this->parentPedestrian = parentPedestrian;
}

void PedestrianMovement::frame_init(){

	float totalTime = getParent()->currSubTrip->totalDistanceOD/walkSpeed;
	totalTimeToCompleteMS = totalTime;
}

void PedestrianMovement::frame_tick()
{
	unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();

	totalTimeToCompleteMS -= tickMS;
	if(totalTimeToCompleteMS <= 0){
		getParent()->setToBeRemoved();
	}
}

void PedestrianMovement::frame_tick_output(){

}



}

} /* namespace sim_mob */
