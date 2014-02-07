//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "WaitBusActivityRoleImpl.hpp"

#include "conf/ConfigParams.hpp"
#include "conf/ConfigManager.hpp"

#include "entities/Person.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/roles/passenger/Passenger.hpp"

using std::vector;
using namespace sim_mob;

sim_mob::WaitBusActivityRoleImpl::WaitBusActivityRoleImpl(Agent* parent, sim_mob::WaitBusActivityRoleBehavior* behavior, sim_mob::WaitBusActivityRoleMovement* movement) :
		WaitBusActivityRole(parent, behavior, movement)
{
}

sim_mob::WaitBusActivityRoleImpl::~WaitBusActivityRoleImpl() {

}

Role* sim_mob::WaitBusActivityRoleImpl::clone(Person* parent) const
{
	WaitBusActivityRoleBehavior* behavior = new WaitBusActivityRoleBehaviorImpl(parent);
	WaitBusActivityRoleMovement* movement = new WaitBusActivityRoleMovementImpl(parent);
	WaitBusActivityRole* waitbusactivityrole = new WaitBusActivityRoleImpl(parent, behavior, movement);
	behavior->setParentWaitBusActivityRole(waitbusactivityrole);
	movement->setParentWaitBusActivityRole(waitbusactivityrole);
	return waitbusactivityrole;
}

sim_mob::WaitBusActivityRoleBehaviorImpl::WaitBusActivityRoleBehaviorImpl(sim_mob::Person* parentAgent) :
		WaitBusActivityRoleBehavior(parentAgent)
{
}

sim_mob::WaitBusActivityRoleBehaviorImpl::~WaitBusActivityRoleBehaviorImpl() {

}

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_init() {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_init is not implemented yet");
}

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_tick() {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick is not implemented yet");
}

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_tick_output() {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick_output is not implemented yet");
}


sim_mob::WaitBusActivityRoleMovementImpl::WaitBusActivityRoleMovementImpl(sim_mob::Person* parentAgent) :
		WaitBusActivityRoleMovement(parentAgent)
{

}

sim_mob::WaitBusActivityRoleMovementImpl::~WaitBusActivityRoleMovementImpl() {

}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_init() {
	// special case: fnode, tnode are all stop ids, for scenarios
	isTagged = false;
	isBoarded = false;
	if(getParent()->originNode.type_== WayPoint::BUS_STOP && getParent()->destNode.type_== WayPoint::BUS_STOP) {
		busStopAgent = getParent()->originNode.busStop_->generatedBusStopAgent;
		getParent()->xPos.force(busStopAgent->getBusStop().xPos);// set xPos to WaitBusActivityRole
		getParent()->yPos.force(busStopAgent->getBusStop().yPos);// set yPos to WaitBusActivityRole
		parentWaitBusActivityRole->TimeOfReachingBusStop = parentWaitBusActivityRole->getParams().now.ms();
		buslineId = "857_1";// set Busline information(hardcoded now, later from Public Transit Route Choice to choose the busline)
		return;
	}
	if(getParent()->destNode.type_== WayPoint::BUS_STOP) { // to here waiting(busstop)
		busStopAgent = getParent()->destNode.busStop_->generatedBusStopAgent;
		getParent()->xPos.set(busStopAgent->getBusStop().xPos);// set xPos to WaitBusActivityRole
		getParent()->yPos.set(busStopAgent->getBusStop().yPos);// set yPos to WaitBusActivityRole
	} else {
		sim_mob::BusStop* busStop_dest = setBusStopXY(getParent()->destNode.node_);// to here waiting(node)
		busStopAgent = busStop_dest->generatedBusStopAgent;// assign the BusStopAgent to WaitBusActivityRole
		getParent()->xPos.set(busStop_dest->xPos);// set xPos to WaitBusActivityRole
		getParent()->yPos.set(busStop_dest->yPos);// set yPos to WaitBusActivityRole
	}
//	parentWaitBusActivityRole->TimeOfReachingBusStop = parentWaitBusActivityRole->getParams().now.ms();
	parentWaitBusActivityRole->TimeOfReachingBusStop = getParent()->currTick.ms();
	buslineId = "857_1";// set Busline information(hardcoded now, later from Public Transit Route Choice to choose the busline)
}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_tick() {
	WaitBusActivityRoleUpdateParams &p = parentWaitBusActivityRole->getParams();
	if(0!=boardingMS) {// if boarding_Frame is already set
		if(boardingMS == p.now.ms()) {// if currFrame is equal to the boarding_Frame, boarding finished
			getParent()->setToBeRemoved();
			isBoarded = true;
			if(getParent()) {
				if(getParent()->getNextRole()) {
					Passenger* passenger = dynamic_cast<Passenger*> (getParent()->getNextRole());// check whether nextRole is passenger Role or not
					if(passenger) {
						// startBoardingMS = boardingMS(finished time) - boardingSEC(for this person)
						// waitingTime = startBoardingMS - TimeOfReachingBusStop
						parentWaitBusActivityRole->waitingTimeAtBusStop = p.now.ms() - getParent()->getBoardingCharacteristics() * 1000 - parentWaitBusActivityRole->TimeOfReachingBusStop;
						passenger->setWaitingTimeAtStop(parentWaitBusActivityRole->waitingTimeAtBusStop);
						passenger->busdriver.set(busDriver);// assign this busdriver to Passenger
						passenger->BoardedBus.set(true);
						passenger->AlightedBus.set(false);
					}
				}
			}
		}
	}
}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_tick_output() {
	WaitBusActivityRoleUpdateParams &p = parentWaitBusActivityRole->getParams();
	if (ConfigManager::GetInstance().FullConfig().is_run_on_many_computers) {
		return;
	}

	//Reset our offset if it's set to zero
	if (displayOffset.getX()==0 && displayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(getParent()->getId()*getParent()->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   displayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   displayOffset.setY(value+1);
	}
	LogOut("("<<"\"passenger\","<<getParent()->currTick.frame()<<","<<getParent()->getId()<<","<<"{\"xPos\":\""<<(getParent()->xPos.get()+displayOffset.getX())<<"\"," <<"\"yPos\":\""<<(getParent()->yPos.get()+displayOffset.getY())<<"\",})"<<std::endl);
}
