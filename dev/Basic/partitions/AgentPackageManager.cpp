/*
 * AgentPackageManager.cpp
 */
#include "AgentPackageManager.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"

namespace sim_mob {

AgentPackageManager AgentPackageManager::instance_;

role_modes AgentPackageManager::getAgentRoleType(Agent const* agent) {
	const Person *person = dynamic_cast<const Person *> (agent);
	if (!person) {
		std::cout << "Error: agent is not a person in AgentPackageManager"
				<< std::endl;

		const Signal *one_signal = dynamic_cast<const Signal *> (agent);
		if (!one_signal) {
			std::cout << "Oh, it is one signal" << std::endl;
		}

		return role_modes(No_Role);
	}

	Person* p = const_cast<Person*> (person);

	const Driver *driver = dynamic_cast<const Driver *> (p->getRole());
	if (driver) {
		return role_modes(Driver_Role);
	}

	const Pedestrian *pedestrian =
			dynamic_cast<const Pedestrian *> (p->getRole());
	if (pedestrian) {
		return role_modes(Pedestrian_Role);
	}

	const Passenger *passenger = dynamic_cast<const Passenger *> (p->getRole());
	if (passenger) {
		return role_modes(Passenger_Role);
	}

	return role_modes(No_Role);
}

void sim_mob::AgentPackageManager::updateOneFeedbackAgent(Agent* new_agent,
		Agent * old_agent) {
	role_modes agent_type = getAgentRoleType(new_agent);

	if (agent_type == Driver_Role) {
		updateOneFeedbackDriver(new_agent, old_agent);
	} else if (agent_type == Pedestrian_Role) {
		updateOneFeedbackPedestrian(new_agent, old_agent);
	} else if (agent_type == Passenger_Role) {
		updateOneFeedbackPassenger(new_agent, old_agent);
	} else {
		std::cout << "Error: not found agent type" << std::endl;
	}
}

void sim_mob::AgentPackageManager::updateOneFeedbackDriver(Agent * new_agent,
		Agent * old_agent) {

	old_agent->xPos.force(new_agent->xPos.get());
	old_agent->yPos.force(new_agent->yPos.get());
	old_agent->xVel.force(new_agent->xVel.get());
	old_agent->yVel.force(new_agent->yVel.get());
	old_agent->xAcc.force(new_agent->xAcc.get());
	old_agent->yAcc.force(new_agent->yAcc.get());

	const Person *new_person = dynamic_cast<const Person *> (new_agent);
	Person* one_new_person = const_cast<Person*> (new_person);
	const Driver *new_driver =
			dynamic_cast<const Driver *> (one_new_person->getRole());
	Driver *one_new_driver =const_cast<Driver*> (new_driver);

	const Person *old_person = dynamic_cast<const Person *> (old_agent);
	Person* one_old_person = const_cast<Person*> (old_person);
	const Driver *old_driver =
			dynamic_cast<const Driver *> (one_old_person->getRole());
	Driver *one_old_driver = const_cast<Driver*> (old_driver);

	one_old_driver->currLane_.force(new_driver->currLane_.get());
	one_old_driver->currLaneOffset_.force(new_driver->currLaneOffset_.get());
	one_old_driver->currLaneLength_.force(new_driver->currLaneLength_.get());
	one_old_driver->buffer_velocity.force(new_driver->buffer_velocity.get());
	one_old_driver->buffer_accel.force(new_driver->buffer_accel.get());
	one_old_driver->inIntersection_.force(one_new_driver->inIntersection_.get());

	one_old_driver->vehicle->velocity.abs.x
			= new_driver->getVehicle()->velocity.abs.x;
	one_old_driver->vehicle->velocity.abs.y
			= new_driver->getVehicle()->velocity.abs.y;
	one_old_driver->vehicle->velocity.rel.x
			= new_driver->getVehicle()->velocity.rel.x;
	one_old_driver->vehicle->velocity.rel.y
			= new_driver->getVehicle()->velocity.rel.y;
	one_old_driver->vehicle->velocity.scaleDir.x
			= new_driver->getVehicle()->velocity.scaleDir.x;
	one_old_driver->vehicle->velocity.scaleDir.y
			= new_driver->getVehicle()->velocity.scaleDir.y;

	one_old_driver->vehicle->accel.abs.x
			= new_driver->getVehicle()->accel.abs.x;
	one_old_driver->vehicle->accel.abs.y
			= new_driver->getVehicle()->accel.abs.y;
	one_old_driver->vehicle->accel.rel.x
			= new_driver->getVehicle()->accel.rel.x;
	one_old_driver->vehicle->accel.rel.y
			= new_driver->getVehicle()->accel.rel.y;
	one_old_driver->vehicle->accel.scaleDir.x
			= new_driver->getVehicle()->accel.scaleDir.x;
	one_old_driver->vehicle->accel.scaleDir.y
			= new_driver->getVehicle()->accel.scaleDir.y;

	one_old_driver->vehicle->xPos = new_driver->getVehicle()->xPos;
	one_old_driver->vehicle->yPos = new_driver->getVehicle()->yPos;
}

void sim_mob::AgentPackageManager::updateOneFeedbackPedestrian(
		Agent * new_agent, Agent * old_agent) {

	old_agent->xPos.force(new_agent->xPos.get());
	old_agent->yPos.force(new_agent->yPos.get());
	old_agent->xVel.force(new_agent->xVel.get());
	old_agent->yVel.force(new_agent->yVel.get());
	old_agent->xAcc.force(new_agent->xAcc.get());
	old_agent->yAcc.force(new_agent->yAcc.get());

	const Person *new_person = dynamic_cast<const Person *> (new_agent);
	Person* one_new_person = const_cast<Person*> (new_person);
	const Pedestrian *new_pedestrian =
			dynamic_cast<const Pedestrian *> (one_new_person->getRole());

	const Person *old_person = dynamic_cast<const Person *> (old_agent);
	Person* one_old_person = const_cast<Person*> (old_person);
	const Pedestrian *old_pedestrian =
			dynamic_cast<const Pedestrian *> (one_old_person->getRole());
	Pedestrian *one_old_pedestrian = const_cast<Pedestrian*> (old_pedestrian);

	one_old_pedestrian->speed = new_pedestrian->speed;
	one_old_pedestrian->xVel = new_pedestrian->xVel;
	one_old_pedestrian->yVel = new_pedestrian->yVel;

	one_old_pedestrian->currentStage = new_pedestrian->currentStage;
	one_old_pedestrian->curCrossingID = new_pedestrian->curCrossingID;
	one_old_pedestrian->startToCross = new_pedestrian->startToCross;

	one_old_pedestrian->cStartX = new_pedestrian->cStartX;
	one_old_pedestrian->cStartY = new_pedestrian->cStartY;
	one_old_pedestrian->cEndX = new_pedestrian->cEndX;
	one_old_pedestrian->cEndY = new_pedestrian->cEndY;

	one_old_pedestrian->firstTimeUpdate = new_pedestrian->firstTimeUpdate;
	one_old_pedestrian->xCollisionVector = new_pedestrian->xCollisionVector;
	one_old_pedestrian->yCollisionVector = new_pedestrian->yCollisionVector;

}

void sim_mob::AgentPackageManager::updateOneFeedbackPassenger(
		Agent * new_agent, Agent * old_agent) {

}

}
