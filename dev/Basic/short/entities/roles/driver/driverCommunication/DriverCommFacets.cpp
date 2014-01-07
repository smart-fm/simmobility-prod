//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * DriverFacets.cpp
 *
 *  Created on: May 15th, 2013
 *      Author: Yao Jin
 */

#include "DriverCommFacets.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;
using std::endl;

namespace sim_mob {
DriverCommBehavior::DriverCommBehavior(sim_mob::Person* parentAgent):
	DriverBehavior(parentAgent),parentDriverCommRole(nullptr){}

DriverCommBehavior::~DriverCommBehavior() {}

void DriverCommBehavior::frame_init() {
	DriverBehavior::frame_init();
}

void DriverCommBehavior::frame_tick() {
	DriverBehavior::frame_tick();
}

void DriverCommBehavior::frame_tick_output() {
	DriverBehavior::frame_tick_output();
}


sim_mob::DriverCommMovement::DriverCommMovement(sim_mob::Person* parentAgent):
	DriverMovement(parentAgent), parentDriverCommRole(nullptr)
{
}

sim_mob::DriverCommMovement::~DriverCommMovement()
{
//	/kjsdhawehakjlsdajkashlk
}

void sim_mob::DriverCommMovement::frame_init() {
	DriverMovement::frame_init();
	sim_mob::Agent * agent = 0;
	//todo: solve this hardcoding
//	this->parentDriverCommRole->RegisterWithBroker("roadrunner");
//	this->parentDriverCommRole->RegisterWithBroker("stk");

	//register this agent with every broker alive!
	std::map<std::string, sim_mob::Broker*>::iterator it =  Broker::getExternalCommunicators().begin();
	while(it != Broker::getExternalCommunicators().end()){
		Broker *broker = it->second;
		broker->registerEntity(parentDriverCommRole);
		it++;
	}
}

void sim_mob::DriverCommMovement::frame_tick() {
	DriverMovement::frame_tick();
	////	Print() << "Driver Agent " << this->parent << " ticking " << p.now.frame() << std::endl;
	////	if((p.now.frame() > 4)&&(p.now.frame() <= 400))
	////	{
	////		sendModule(p.now);
	////	}
	//////	else if((p.now.frame() >= 4) && (p.now.frame() < 10) )//todo, just to test, just put else without if
	//////	{
	////		receiveModule(p.now);
	//////	}
	////	if(!registered)
	////	{
	////		return;
	////	}
	////	Print() << "Driver " << this << "  Setting agent update done" << std::endl;

//	this->parentDriverCommRole->setAgentUpdateDone(true);
}

void sim_mob::DriverCommMovement::frame_tick_output() {
	DriverMovement::frame_tick_output();
}


DriverComm* sim_mob::DriverCommMovement::getParentDriverComm() const {
	return parentDriverCommRole;
}

void sim_mob::DriverCommMovement::setParentDriverComm(DriverComm* parentDriverCommRole_) {
	this->parentDriverCommRole = parentDriverCommRole_;
}

}
