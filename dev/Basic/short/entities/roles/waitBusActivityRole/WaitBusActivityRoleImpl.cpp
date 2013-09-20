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

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_init(UpdateParams& p) {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_init is not implemented yet");
}

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_tick(UpdateParams& p) {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick is not implemented yet");
}

void sim_mob::WaitBusActivityRoleBehaviorImpl::frame_tick_output(const UpdateParams& p) {
	throw std::runtime_error("WaitBusActivityRoleBehavior::frame_tick_output is not implemented yet");
}


sim_mob::WaitBusActivityRoleMovementImpl::WaitBusActivityRoleMovementImpl(sim_mob::Person* parentAgent) :
		WaitBusActivityRoleMovement(parentAgent)
{

}

sim_mob::WaitBusActivityRoleMovementImpl::~WaitBusActivityRoleMovementImpl() {

}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_init(UpdateParams& p) {
	if(getParent()->destNode.type_== WayPoint::BUS_STOP) { // to here waiting(busstop)
		busStopAgent = getParent()->destNode.busStop_->generatedBusStopAgent;
	} else {
		sim_mob::BusStop* busStop_dest = setBusStopXY(getParent()->destNode.node_);// to here waiting(node)
		busStopAgent = busStop_dest->generatedBusStopAgent;// assign the BusStopAgent to WaitBusActivityRole
		getParent()->xPos.set(busStop_dest->xPos);// set xPos to WaitBusActivityRole
		getParent()->yPos.set(busStop_dest->yPos);// set yPos to WaitBusActivityRole
	}
	parentWaitBusActivityRole->TimeOfReachingBusStop = p.now.ms();
	buslineid = "7_2";// set Busline information(hardcoded now, later from Public Transit Route Choice to choose the busline)
}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_tick(UpdateParams& p) {
	if(0!=boarding_MS) {// if boarding_Frame is already set
		if(boarding_MS == p.now.ms()) {// if currFrame is equal to the boarding_Frame
			getParent()->setToBeRemoved();
			boarding_MS = 0;
			//Person* person = dynamic_cast<Person*> (parent);
			if(getParent()) {
				if(getParent()->getNextRole()) {
					Passenger* passenger = dynamic_cast<Passenger*> (getParent()->getNextRole());// check whether nextRole is passenger Role or not
					if(passenger) {
						//BusDriver* driver = dynamic_cast<BusDriver*>(busDriver);
						passenger->busdriver.set(busDriver);// assign this busdriver to Passenger
						passenger->BoardedBus.set(true);
						passenger->AlightedBus.set(false);
					}
				}
			}
		}
	}
}

void sim_mob::WaitBusActivityRoleMovementImpl::frame_tick_output(const UpdateParams& p) {
	if (ConfigManager::GetInstance().FullConfig().is_run_on_many_computers) {
		return;
	}

	//Reset our offset if it's set to zero
	if (DisplayOffset.getX()==0 && DisplayOffset.getY()==0) {
	   boost::mt19937 gen(static_cast<unsigned int>(getParent()->getId()*getParent()->getId()));
	   boost::uniform_int<> distX(0, 249);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
	   boost::uniform_int<> distY(0, 99);
	   boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
	   unsigned int value = (unsigned int)varX();
	   DisplayOffset.setX(value+1);
	   value= (unsigned int)varY();
	   DisplayOffset.setY(value+1);
	}
	//LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<parent->getId()<<","<<"{\"xPos\":\""<<(parent->xPos.get()+DisplayOffset.getX()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(parent->yPos.get()+DisplayOffset.getY()+DisplayOffset.getY())<<"\",})"<<std::endl);
	LogOut("("<<"\"passenger\","<<p.now.frame()<<","<<getParent()->getId()<<","<<"{\"xPos\":\""<<(getParent()->xPos.get()+DisplayOffset.getX())<<"\"," <<"\"yPos\":\""<<(getParent()->yPos.get()+DisplayOffset.getY())<<"\",})"<<std::endl);
}


void sim_mob::WaitBusActivityRoleMovementImpl::flowIntoNextLinkIfPossible(UpdateParams& p) {

}
