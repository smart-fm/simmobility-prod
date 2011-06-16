#include "Agent.hpp"
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace sim_mob;


//TEMP
boost::mutex sim_mob::Agent::global_mutex;


sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(NULL, 0), yPos(NULL, 0) {
//	int currMode = id%4;

	//Set to pedestrian for testing only
	int currMode = 1;
	//TODO: Inheritance, inheritance, inheritance
	if (currMode==0)
		currMode = DRIVER;
	else if (currMode==1)
		currMode = PEDESTRIAN;
	else if (currMode==2)
		currMode = CYCLIST;
	else if (currMode==3)
		currMode = PASSENGER;

	//Set random seed
	srand(id);

	//Set default speed in the range of 1m/s to 1.4m/s
	speed = 1+(double(rand()%5))/10;

	xVel = 0;
	yVel = 0;

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

	//Check if the agent has reached the goal
	if(isGoalReached()){

		if(!toRemoved){
			//Output (temp)
			{
				boost::mutex::scoped_lock local_lock(global_mutex);
				std::cout <<"(" <<this->getId() <<" has reached the goal)" <<std::endl;
			}
			toRemoved = true;
		}
	}

	else{

		updateVelocity();
		updatePosition();
		//Output (temp)
		{
			boost::mutex::scoped_lock local_lock(global_mutex);
			std::cout <<"(" <<this->getId() <<"," <<frameNumber<<","<<this->xPos.get()<<"," <<this->yPos.get()<<")" <<std::endl;
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

void sim_mob::Agent::setGoal() {
	goal.xPos = this->xPos.get();
	goal.yPos = topLeftCrossing.yPos + double(rand()%5) + 1;;
}

bool sim_mob::Agent::isGoalReached() {
	return (this->yPos.get()>=goal.yPos);
}

void sim_mob::Agent::updateVelocity() {
	//Set direction (towards the goal)
	xVel = goal.xPos - this->xPos.get();
	yVel = goal.yPos - this->yPos.get();
	//Normalize
	double length = sqrt(xVel*xVel + yVel*yVel);
	xVel = xVel/length;
	yVel = yVel/length;
	//Set actual velocity
	xVel = xVel*speed;
	yVel = yVel*speed;
}

void sim_mob::Agent::updatePosition(){
	//Compute
	double newX = this->xPos.get()+xVel*1; //Time step is 1 second
	double newY = this->yPos.get()+yVel*1;
	//Set
	this->xPos.set(newX);
	this->yPos.set(newY);
}



