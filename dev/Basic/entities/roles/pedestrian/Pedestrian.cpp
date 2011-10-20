/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Pedestrian.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"

using namespace sim_mob;


double Pedestrian::collisionForce = 20;
double Pedestrian::agentRadius = 0.5; //Shoulder width of a person is about 0.5 meter


sim_mob::Pedestrian::Pedestrian(Agent* parent) : Role(parent)
{
	//Check non-null parent. Perhaps references may be of use here?
	if (!parent) {
		std::cout <<"Role constructed with no parent Agent." <<std::endl;
		throw 1;
	}

	//Init
	currPhase = 3; //Green phase by default: 1-red phase, 3-green phase
	currentStage=0;
	setGoal(currentStage);
	startToCross=false;
	startPosSet=false;

	//Set random seed
	srand(parent->getId());

	//Set default speed in the range of 0.9m/s to 1.3m/s
	speed = 0.9+(double(rand()%5))/10;

	xVel = 0;
	yVel = 0;

	xCollisionVector = 0;
	yCollisionVector = 0;

}

//Main update functionality
void sim_mob::Pedestrian::update(frame_t frameNumber)
{

	if(frameNumber>=parent->startTime){

		//Set the initial position of agent
		if(!isStartPosSet()){
//			parent->xPos.set(((double)parent->originNode->location->getX())/100);
//			parent->yPos.set(((double)parent->originNode->location->getY())/100);
//			cStartX=372183.51;
//			cStartY=143352.55;
//			cEndX=((double)parent->destNode->location->getX())/100;
//			cEndX=((double)parent->destNode->location->getY())/100;

			//TEMP: for testing on self-created network only
			cStartX=500;
			cStartY=300;
			cEndX=500;
			cEndY=600;

		}
		else{
		//update signal information
		updatePedestrianSignal();

	    //checkForCollisions();

		//Check if the agent has reached the destination
		if(isDestReached()){

			if(!parent->isToBeRemoved()){
				//Output (temp)
                                LogOut("Pedestrian " <<parent->getId() <<" has reached the destination" <<std::endl);
				parent->setToBeRemoved(true);
			}
			return;
		}

		if(isGoalReached()){
			currentStage++;
			setGoal(currentStage); //Set next goal
//			{
//				boost::mutex::scoped_lock local_lock(Logger::global_mutex);
//				LogOutNotSync("Pedestrian " <<parent->getId() <<" has reached goal " <<currentStage<<std::endl);
//				LogOutNotSync("("<<"Pedestrian,"<<frameNumber<<","<<parent->getId()<<","<<"{xPos:"<<parent->xPos.get()<<"," <<"yPos:"<<this->parent->yPos.get()<<","<<"pedSig:"<<currPhase<<",})"<<std::endl);
//			}
		}
		else{

			if(currentStage==0||currentStage==2){
				updateVelocity(0);
				updatePosition();
//				//Output (temp)
//				LogOut("("<<"Pedestrian,"<<frameNumber<<","<<parent->getId()<<","<<"{xPos:"<<parent->xPos.get()<<"," <<"yPos:"<<this->parent->yPos.get()<<","<<"pedSig:"<<currPhase<<",})"<<std::endl);
			}
			else if(currentStage==1){

				//Check whether to start to cross or not
				if(!startToCross){
					if(currPhase == 3)  //Green phase
						startToCross = true;
					else if(currPhase == 1){ //Red phase
						if(checkGapAcceptance()==true)
							startToCross=true;
					}
				}

				if(startToCross){
					if(currPhase==3) //Green phase
						updateVelocity(1);
					else if (currPhase ==1) //Red phase
						updateVelocity(2);
					updatePosition();
				}
				else{
					//Output (temp)
                                        LogOut("Pedestrian " <<parent->getId() <<" is waiting at the crossing" <<std::endl);
				}
				//Output (temp)
                                LogOut("("<<"\"pedestrian\","<<frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
			}

		}


	//	//Continue checking if the goal has not been reached.
	//	if(reachStartOfCrossing()) {
	//		if(currPhase == 3){ //Green phase
	//			updateVelocity(1);
	//			updatePosition();
	//		} else if (currPhase == 1) { //Red phase
	//			//Waiting, do nothing now
	//			//Output (temp)
	//			checkGapAcceptance();
        //			LogOut("(Agent " <<parent->getId() <<" is waiting at crossing at frame "<<frameNumber<<")" <<std::endl);
	//		}
	//	} else {
	//		if(currPhase==1&&onCrossing())
	//			updateVelocity(2);
	//		else
	//			updateVelocity(1);
	//		updatePosition();
	//	}
		}
		parent->currentCrossing.set(getCurrentCrossing());

	}

}

/*---------------------Perception-related functions----------------------*/

void sim_mob::Pedestrian::setGoal(int stage) //0-to the next intersection, 1-to the crossing end, 2-to the destination
{
	//goal.xPos = this->xPos.get();
	//goal.yPos = topLeftCrossing.yPos + double(rand()%5) + 1;;

	//Give every agent the same goal.
	//goal.xPos = 1100;
	if(stage==0){
//		goal = Point2D(37218351,14335255);

		//TEMP: for testing on self-created network only
		goal = Point2D(50000,30000);
		destPos = Point2D(50000,60000);

	}
	else if(stage==1){

		//???? How to get position of crossings
//		RoadNetwork& network = ConfigParams::GetInstance().getNetwork();

		//Set the agent's position at the start of crossing and set the goal to the end of crossing
		setCrossingPos();

	}
	else if(stage==2){

		//TEMP: for testing on self-created network only
		parent->xPos.set(500);
		parent->yPos.set(300);
//		goal = Point2D(parent->destNode->location->getX(),parent->destNode->location->getY());
		goal = Point2D(destPos.getX(),destPos.getY());
	}

}

bool sim_mob::Pedestrian::isDestReached()
{

//	double dX = abs(((double)parent->destNode->location->getX())/100 - parent->xPos.get());
//	double dY = abs(((double)parent->destNode->location->getY())/100 - parent->yPos.get());
	double dX = abs(((double)destPos.getX())/100 - parent->xPos.get());
	double dY = abs(((double)destPos.getY())/100 - parent->yPos.get());
	double dis = sqrt(dX*dX+dY*dY);
	return dis < agentRadius*4;

	//return (parent->yPos.get()>=goal.yPos);
}

bool sim_mob::Pedestrian::isGoalReached()
{
	double dX = abs(((double)goal.getX())/100 - parent->xPos.get());
	double dY = abs(((double)goal.getY())/100 - parent->yPos.get());
	double dis = sqrt(dX*dX+dY*dY);
	return dis < agentRadius*4;

	//return (parent->yPos.get()>=goal.yPos);
}

//bool sim_mob::Pedestrian::reachStartOfCrossing()
//{
//
//
//	return false;
//
////	int lowerRightCrossingY = ConfigParams::GetInstance().crossings["lowerright"].getY();
////
////	if(parent->yPos.get()<=lowerRightCrossingY){
////		double dist = lowerRightCrossingY - parent->yPos.get();
////		if(dist<speed*1)
////			return true;
////		else
////			return false;
////	}
////	else
////		return false;
//}

bool sim_mob::Pedestrian::onCrossing()
{
	if(currentStage==1&&startToCross)
		return true;
	else
		return false;
}

int sim_mob::Pedestrian::getCurrentCrossing()
{

	if(onCrossing())
		return 0;
	else
		return -1;
}

void sim_mob::Pedestrian::updatePedestrianSignal()
{

	Agent* a = nullptr;
	Signal* s = nullptr;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		a = Agent::all_agents[i];
		if (a->getId()==parent->getId()) {
			a = nullptr;
			continue;
		}

		if (dynamic_cast<Signal*>(a)) {
		   s = dynamic_cast<Signal*>(a);
		   currPhase=1;//s->get_Pedestrian_Light(0);
			//It's a signal
		}
		s = nullptr;
		a = nullptr;
	}
	s = nullptr;
	a = nullptr;


//	currPhase = sig.get_Pedestrian_Light(0);
//	if(phaseCounter==60){ //1 minute period for switching phases (testing only)
//		phaseCounter=0;
//		if(currPhase==0)
//			currPhase = 1;
//		else
//			currPhase = 0;
//	}
//	else
//		phaseCounter++;
//	currPhase=1 ;
}

/*---------------------Decision-related functions------------------------*/

bool sim_mob::Pedestrian::checkGapAcceptance(){

	//Search for the nearest driver on the current link
	Agent* a = nullptr;
	Person* p = nullptr;
	double pedRelX, pedRelY, drvRelX, drvRelY;
	double minDist=1000000;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		a = Agent::all_agents[i];
		if (a->getId()==parent->getId()) {
			a = nullptr;
			continue;
		}

		if(dynamic_cast<Person*>(a)){
			p=dynamic_cast<Person*>(a);
			if(dynamic_cast<Driver*>(p->getRole())){
				if(p->currentLink.get()==0){
					//std::cout<<"dsahf"<<std::endl;
					absToRel(p->xPos.get(),p->yPos.get(),drvRelX,drvRelY);
					absToRel(parent->xPos.get(),parent->xPos.get(),pedRelX,pedRelY);
					if(minDist>abs(drvRelY-pedRelY))
						minDist = abs(drvRelY-pedRelY);
				}
			}
		}
		p = nullptr;
		a = nullptr;
	}
	p = nullptr;
	a = nullptr;

	if((minDist/5-30/speed)>1)
	{

		return true;
	}
	else
		return false;

	//return false;
}

/*---------------------Action-related functions--------------------------*/

void sim_mob::Pedestrian::updateVelocity(int flag) //0-on sidewalk, 1-on crossing green, 2-on crossing red
{
	//Set direction (towards the goal)
	double scale;
	xVel = ((double)goal.getX())/100 - parent->xPos.get();
	yVel = ((double)goal.getY())/100 - parent->yPos.get();
	//Normalize
	double length = sqrt(xVel*xVel + yVel*yVel);
	xVel = xVel/length;
	yVel = yVel/length;
	//Set actual velocity
	if(flag==0)
		scale = 1.5;
	else if(flag==1)
		scale = 1;
	else if (flag==2)
		scale = 1.8;
	xVel = xVel*speed*scale;
	yVel = yVel*speed*scale;

//	//Set direction (towards the goal)
//	double xDirection = goal.getX() - parent->xPos.get();
//	double yDirection = goal.getY() - parent->yPos.get();
//
//	//Normalize
//	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
//	xDirection = xDirection/magnitude;
//	yDirection = yDirection/magnitude;
//
//	//Set actual velocity
//	xVel = xDirection*speed;
//	yVel = yDirection*speed;
}

void sim_mob::Pedestrian::updatePosition()
{
//	//Factor in collisions
//	double xVelCombined = xVel + xCollisionVector;
//	double yVelCombined = yVel + yCollisionVector;

	//Compute
//	double newX = parent->xPos.get()+xVelCombined*1; //Time step is 1 second
//	double newY = parent->yPos.get()+yVelCombined*1;
	double newX = parent->xPos.get()+xVel*1; //Time step is 1 second
	double newY = parent->yPos.get()+yVel*1;

	//Decrement collision velocity
//	if (xCollisionVector != 0) {
//		xCollisionVector -= ((0.1*collisionForce) / (xCollisionVector/abs(xCollisionVector)) );
//	}
//	if (yCollisionVector != 0) {
//		yCollisionVector -= ((0.1*collisionForce) / (yCollisionVector/abs(yCollisionVector)) );
//	}

	//Set
	parent->xPos.set(newX);
	parent->yPos.set(newY);
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
	if (other) {
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

/*---------------------Other helper functions----------------------------*/

void sim_mob::Pedestrian::setCrossingPos(){

	//TEMP: for testing on self-created network only

	//Set agents' start crossing locations
	double xRel = -30;
//	double yRel = 30+(double)(rand()%6);
	double yRel = 30+(double)(rand()%6)+(double)(rand()%6);
	double xAbs=0;
	double yAbs=0;
	relToAbs(xRel,yRel,xAbs,yAbs);
	parent->xPos.set(xAbs);
	parent->yPos.set(yAbs);
	//Set agents' end crossing locations
	xRel=30;
	relToAbs(xRel,yRel,xAbs,yAbs);
	goal = Point2D(int(xAbs*100),int(yAbs*100));
}

bool sim_mob::Pedestrian::isStartPosSet(){
//	if(parent->xPos.get()==0 && parent->yPos.get()==0)
//		return false;
//	else
//		return true;
	if(!startPosSet){
		startPosSet=true;
		return false;
	}
	else
		return true;
}

void sim_mob::Pedestrian::absToRel(double xAbs, double yAbs, double & xRel, double & yRel){

	double xDir=cEndX-cStartX;
	double yDir=cEndY-cStartY;
	double xOffset=xAbs-cStartX;
	double yOffset=yAbs-cStartY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	double xDirection=xDir/magnitude;
	double yDirection=yDir/magnitude;
	xRel = xOffset*xDirection+yOffset*yDirection;
	yRel =-xOffset*yDirection+yOffset*xDirection;
}

void sim_mob::Pedestrian::relToAbs(double xRel, double yRel, double & xAbs, double & yAbs){
	double xDir=cEndX-cStartX;
	double yDir=cEndY-cStartY;
	double magnitude=sqrt(xDir*xDir+yDir*yDir);
	double xDirection=xDir/magnitude;
	double yDirection=yDir/magnitude;
	xAbs=xRel*xDirection-yRel*yDirection+cStartX;
	yAbs=xRel*yDirection+yRel*xDirection+cStartY;
}








