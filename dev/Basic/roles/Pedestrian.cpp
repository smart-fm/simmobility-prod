#include "Pedestrian.hpp"

using namespace sim_mob;


double Pedestrian::collisionForce = 20;
double Pedestrian::agentRadius = 10;


sim_mob::Pedestrian::Pedestrian(Agent* parent) : Role(parent)
{
	//Check non-null parent. Perhaps references may be of use here?
	if (parent==nullptr) {
		std::cout <<"Role constructed with no parent Agent." <<std::endl;
		throw 1;
	}

	//Defaults
	currPhase = 0; //Green phase by default
	phaseCounter = 0;

	//Set random seed
	srand(parent->getId());

	//Set default speed in the range of 1m/s to 1.4m/s
	speed = 1+(double(rand()%5))/10;

	//TEMP: Needed to speed things up a little:
	speed *= 5;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;

	isGoalSet = false;
}



//Main update functionality
void sim_mob::Pedestrian::update(frame_t frameNumber)
{
	//Set the goal of agent
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}

	updatePedestrianSignal();

	checkForCollisions();

	//Check if the agent has reached the goal
	if(isGoalReached()){

		if(!parent->isToBeRemoved()){
			//Output (temp)
			{
				boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
				std::cout <<"(Agent " <<parent->getId() <<" has reached the goal)" <<std::endl;
			}
			parent->setToBeRemoved(true);
		}
		return;
	}

	//Continue checking if the goal has not been reached.
	if(reachStartOfCrossing()) {
		if(currPhase == 0){ //Green phase
			updateVelocity();
			updatePosition();
		} else if (currPhase == 1) { //Red phase
			//Waiting, do nothing now
			//Output (temp)
			{
				boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
				std::cout <<"(Agent " <<parent->getId() <<" is waiting at crossing at frame "<<frameNumber<<")" <<std::endl;
			}
		}
	} else {
		updateVelocity();
		updatePosition();
	}
}




//Simple implementations for testing

void sim_mob::Pedestrian::checkForCollisions()
{
	//For now, just check all agents and get the first positive collision. Very basic.
	Agent* other = nullptr;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==parent->getId()) {
			other = nullptr;
			continue;
		}

		//Check.
		double dx = other->xPos.get() - parent->xPos.get();
		double dy = other->yPos.get() - parent->yPos.get();
		double distance = sqrt(dx*dx + dy*dy);
		if (distance < 2*agentRadius) {
			break; //Collision
		}
		other = nullptr;
	}

	//Set collision vector. Overrides previous setting, if any.
	if (other!=nullptr) {
		//Get a heading.
		double dx = other->xPos.get() - parent->xPos.get();
		double dy = other->yPos.get() - parent->yPos.get();

		//If the two agents are directly on top of each other, set
		//  their distances to something non-crashable.
		if (dx==0 && dy==0) {
			dx = other->getId() - parent->getId();
			dy = parent->getId() - other->getId();
		}

		//Normalize
		double magnitude = sqrt(dx*dx + dy*dy);
		if (magnitude==0) {
			dx = dy;
			dy = dx;
		}
		dx = dx/magnitude;
		dy = dy/magnitude;

		//Set collision vector to the inverse
		xCollisionVector = -dx * collisionForce;
		yCollisionVector = -dy * collisionForce;
	}
}

void sim_mob::Pedestrian::setGoal()
{
	//goal.xPos = this->xPos.get();
	//goal.yPos = topLeftCrossing.yPos + double(rand()%5) + 1;;

	//Give every agent the same goal.
	//goal.xPos = 1100;
	goal.xPos = ConfigParams::GetInstance().boundaries["topright"].xPos + 100;
	//goal.yPos = 200;
	goal.yPos = ConfigParams::GetInstance().boundaries["topright"].yPos / 2;
}

bool sim_mob::Pedestrian::isGoalReached()
{
	//Simple manhatten distance check
	int dX = abs(goal.xPos - parent->xPos.get());
	int dY = abs(goal.yPos - parent->yPos.get());
	return dX+dY < agentRadius;

	//return (parent->yPos.get()>=goal.yPos);
}

void sim_mob::Pedestrian::updateVelocity()
{
	//Set direction (towards the goal)
	double xDirection = goal.xPos - parent->xPos.get();
	double yDirection = goal.yPos - parent->yPos.get();

	//Normalize
	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
	xDirection = xDirection/magnitude;
	yDirection = yDirection/magnitude;

	//Set actual velocity
	xVel = xDirection*speed;
	yVel = yDirection*speed;
}

void sim_mob::Pedestrian::updatePosition()
{
	//Factor in collisions
	double xVelCombined = xVel + xCollisionVector;
	double yVelCombined = yVel + yCollisionVector;

	//Compute
	double newX = parent->xPos.get()+xVelCombined*1; //Time step is 1 second
	double newY = parent->yPos.get()+yVelCombined*1;

	//Decrement collision velocity
	if (xCollisionVector != 0) {
		xCollisionVector -= ((0.1*collisionForce) / (xCollisionVector/abs(xCollisionVector)) );
	}
	if (yCollisionVector != 0) {
		yCollisionVector -= ((0.1*collisionForce) / (yCollisionVector/abs(yCollisionVector)) );
	}

	//Set
	parent->xPos.set(newX);
	parent->yPos.set(newY);
}

void sim_mob::Pedestrian::updatePedestrianSignal()
{

	if(phaseCounter==60){ //1 minute period for switching phases (testing only)
		phaseCounter=0;
		if(currPhase==0)
			currPhase = 1;
		else
			currPhase = 0;
	}
	else
		phaseCounter++;
}

bool sim_mob::Pedestrian::reachStartOfCrossing()
{
	int lowerRightCrossingY = ConfigParams::GetInstance().crossings["lowerright"].yPos;

	if(parent->yPos.get()<=lowerRightCrossingY){
		double dist = lowerRightCrossingY - parent->yPos.get();
		if(dist<speed*1)
			return true;
		else
			return false;
	}
	else
		return false;
}
