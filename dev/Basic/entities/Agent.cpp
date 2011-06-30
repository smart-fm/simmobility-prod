#include "Agent.hpp"
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace sim_mob;

using std::vector;


vector<Agent*> sim_mob::Agent::all_agents;
double Agent::collisionForce = 20;
double Agent::agentRadius = 10;

//TEMP
boost::mutex sim_mob::Agent::global_mutex;


sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(NULL, 0), yPos(NULL, 0) {
//	int currMode = id%4;

	//Set to pedestrian for testing only
	int currMode = 1;
	//TODO: Inheritance, inheritance, inheritance
	if (currMode==0)
		currMode = 0;//DRIVER;
	else if (currMode==1)
		currMode = 0;//PEDESTRIAN;
	else if (currMode==2)
		currMode = 0;//CYCLIST;
	else if (currMode==3)
		currMode = 0;//PASSENGER;

	currPhase = 0; //Green phase by default
	phaseCounter = 0;

	//Set random seed
	srand(id);

	//Set default speed in the range of 1m/s to 1.4m/s
	speed = 1+(double(rand()%5))/10;

	//TEMP: Needed to speed things up a little:
	speed *= 5;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;

	isGoalSet = false;
	toRemoved = false;

}


void sim_mob::Agent::update(frame_t frameNumber) {
	//TODO: Migrate this into the agent's behavior using inheritance.
	/*if (currMode==DRIVER) {
		updateDriverBehavior(*this);
	} else if (currMode==PEDESTRIAN || currMode==CYCLIST) {
		updatePedestrianBehavior(*this);
	} else if (currMode==PASSENGER) {
		updatePassengerBehavior(*this);
	}*/

	//Set the goal of agent
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}

	updatePedestrianSignal();

	checkForCollisions();

	//Check if the agent has reached the goal
	if(isGoalReached()){

		if(!toRemoved){
			//Output (temp)
			{
				boost::mutex::scoped_lock local_lock(global_mutex);
				std::cout <<"(Agent " <<this->getId() <<" has reached the goal)" <<std::endl;
			}
			toRemoved = true;
		}
	}

	else{

		if(reachStartOfCrossing()){
			if(currPhase == 0){ //Green phase
				updateVelocity();
				updatePosition();
			}
			else if (currPhase == 1) { //Red phase
				//Waiting, do nothing now
				//Output (temp)
				{
					boost::mutex::scoped_lock local_lock(global_mutex);
					std::cout <<"(Agent " <<this->getId() <<" is waiting at crossing at frame "<<frameNumber<<")" <<std::endl;
				}
			}
		}
		else {
			updateVelocity();
			updatePosition();
		}

		//Output (temp)
		{
			boost::mutex::scoped_lock local_lock(global_mutex);
			std::cout <<"(" <<this->getId() <<"," <<frameNumber<<","<<this->xPos.get()<<"," <<this->yPos.get()<<","<<currPhase<<")" <<std::endl;
		}
	}
}


void sim_mob::Agent::subscribe(BufferedDataManager* mgr, bool isNew)
{
	if (isNew) {
		xPos.migrate(mgr);
		yPos.migrate(mgr);
	} else {
		xPos.migrate(NULL);
		yPos.migrate(NULL);
	}

}


void sim_mob::Agent::updateShortestPath() {
	trivial(getId());
}



///////////////////////////////
// Temporary location
///////////////////////////////
void sim_mob::Agent::pathChoice(Agent& a) {
	trivial(a.getId()); //Trivial. Will update path choice later.
}
void sim_mob::Agent::updateDriverBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update driver behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void sim_mob::Agent::updatePedestrianBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update pedestrian behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void sim_mob::Agent::updatePassengerBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update passenger behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);  //NOTE: Do passengers need to do this?
	}
}

//Simple implementations for testing

void sim_mob::Agent::checkForCollisions() {
	//For now, just check all agents and get the first positive collision. Very basic.
	Agent* other = NULL;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==this->getId()) {
			other = NULL;
			continue;
		}

		//Check.
		double dx = other->xPos.get() - this->xPos.get();
		double dy = other->yPos.get() - this->yPos.get();
		double distance = sqrt(dx*dx + dy*dy);
		if (distance < 2*agentRadius) {
			break; //Collision
		}
		other = NULL;
	}

	//Set collision vector. Overrides previous setting, if any.
	if (other!=NULL) {
		//Get a heading.
		double dx = other->xPos.get() - this->xPos.get();
		double dy = other->yPos.get() - this->yPos.get();

		//If the two agents are directly on top of each other, set
		//  their distances to something non-crashable.
		if (dx==0 && dy==0) {
			dx = other->getId() - this->getId();
			dy = this->getId() - other->getId();
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

void sim_mob::Agent::setGoal() {
	//goal.xPos = this->xPos.get();
	//goal.yPos = topLeftCrossing.yPos + double(rand()%5) + 1;;

	//Give every agent the same goal.
	goal.xPos = 1100;
	goal.yPos = 200;
}

bool sim_mob::Agent::isGoalReached() {
	return (this->yPos.get()>=goal.yPos);
}

void sim_mob::Agent::updateVelocity() {
	//Set direction (towards the goal)
	double xDirection = goal.xPos - this->xPos.get();
	double yDirection = goal.yPos - this->yPos.get();

	//Normalize
	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
	xDirection = xDirection/magnitude;
	yDirection = yDirection/magnitude;

	//Set actual velocity
	xVel = xDirection*speed;
	yVel = yDirection*speed;
}

void sim_mob::Agent::updatePosition(){
	//Factor in collisions
	double xVelCombined = xVel + xCollisionVector;
	double yVelCombined = yVel + yCollisionVector;

	//Compute
	double newX = this->xPos.get()+xVelCombined*1; //Time step is 1 second
	double newY = this->yPos.get()+yVelCombined*1;

	//Decrement collision velocity
	if (xCollisionVector != 0) {
		xCollisionVector -= ((0.1*collisionForce) / (xCollisionVector/abs(xCollisionVector)) );
	}
	if (yCollisionVector != 0) {
		yCollisionVector -= ((0.1*collisionForce) / (yCollisionVector/abs(yCollisionVector)) );
	}

	//Set
	this->xPos.set(newX);
	this->yPos.set(newY);
}

void sim_mob::Agent::updatePedestrianSignal(){

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

bool sim_mob::Agent::reachStartOfCrossing(){

	if(this->yPos.get()<=lowerRightCrossing.yPos){
		double dist = lowerRightCrossing.yPos - this->yPos.get();
		if(dist<speed*1)
			return true;
		else
			return false;
	}
	else
		return false;
}



