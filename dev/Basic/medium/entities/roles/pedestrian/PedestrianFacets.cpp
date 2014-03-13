/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"


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
		MovementFacet(parentAgent), parentPedestrian(nullptr)
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

}
void PedestrianMovement::frame_tick()
{

}
void PedestrianMovement::frame_tick_output(){

}
void PedestrianMovement::flowIntoNextLinkIfPossible(UpdateParams& p){

}

Vehicle* PedestrianMovement::initializePath(bool allocateVehicle){
	return nullptr;
}


}

} /* namespace sim_mob */
