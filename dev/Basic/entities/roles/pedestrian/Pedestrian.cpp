/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Pedestrian.cpp
 *
 *  Created on: 2011-6-20
 *      Author: Linbo
 */

#include "Pedestrian.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "geospatial/Node.hpp"
#include "util/OutputUtil.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "entities/Signal.hpp"

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
	sigColor = 2; //Green by default
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
			parent->xPos.set(parent->originNode->location->getX());
			parent->yPos.set(parent->originNode->location->getY());
//			cStartX=372507.60;
//			cStartY=143551.20;
//			cEndX=((double)parent->destNode->location->getX())/100;
//			cEndX=((double)parent->destNode->location->getY())/100;

			//TEMP: for testing on self-created network only
//			cStartX=500;
//			cStartY=300;
//			cEndX=500;
//			cEndY=600;

		}
		else{
		//update signal information
//		updatePedestrianSignal();

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
				LogOut("Pedestrian " <<parent->getId() <<" is walking on sidewalk" <<std::endl);
//				//Output (temp)
//				LogOut("("<<"Pedestrian,"<<frameNumber<<","<<parent->getId()<<","<<"{xPos:"<<parent->xPos.get()<<"," <<"yPos:"<<this->parent->yPos.get()<<","<<"pedSig:"<<currPhase<<",})"<<std::endl);
//				LogOut("("<<"\"pedestrian\","<<frameNumber<<","<<parent->getId()<<","<<"{\"xPos\":\""<<parent->xPos.get()<<"\"," <<"\"yPos\":\""<<this->parent->yPos.get()<<"\",})"<<std::endl);
			}
			else if(currentStage==1){

				//Check whether to start to cross or not
				updatePedestrianSignal();
				if(!startToCross){
					if(sigColor == 2)  //Green phase
						startToCross = true;
					else if(sigColor == 0){ //Red phase
						if(checkGapAcceptance()==true)
							startToCross=true;
					}
				}

				if(startToCross){
					if(sigColor==2) //Green phase
						updateVelocity(1);
					else if (sigColor ==0) //Red phase
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
		goal = Point2D(37250760,14355120); //Hard-code now, need to be replaced once route choicer is done

		//TEMP: for testing on self-created network only
//		goal = Point2D(50000,30000);
//		destPos = Point2D(50000,60000);

	}
	else if(stage==1){

		//???? How to get position of crossings
//		RoadNetwork& network = ConfigParams::GetInstance().getNetwork();

		//Set the agent's position at the start of crossing and set the goal to the end of crossing
		setCrossingParas();

	}
	else if(stage==2){

		parent->xPos.set(37250760);  //Hard-code now, to be changed
		parent->yPos.set(14355120);
		goal = Point2D(parent->destNode->location->getX(),parent->destNode->location->getY());
//		goal = Point2D(destPos.getX(),destPos.getY());
	}

}

bool sim_mob::Pedestrian::isDestReached()
{

	double dX = ((double)abs(parent->destNode->location->getX() - parent->xPos.get()))/100;
	double dY = ((double)abs(parent->destNode->location->getY() - parent->yPos.get()))/100;
//	double dX = abs(((double)destPos.getX())/100 - parent->xPos.get());
//	double dY = abs(((double)destPos.getY())/100 - parent->yPos.get());
	double dis = sqrt(dX*dX+dY*dY);
	return dis < agentRadius*4;

	//return (parent->yPos.get()>=goal.yPos);
}

bool sim_mob::Pedestrian::isGoalReached()
{
	double dX = ((double)abs(goal.getX() - parent->xPos.get()))/100;
	double dY = ((double)abs(goal.getY() - parent->yPos.get()))/100;
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

void sim_mob::Pedestrian::updatePedestrianSignal()
{
	if(!trafficSignal)
		std::cout<<"Traffic signal not found!"<<std::endl;
	else{
		if(currCrossing){
			sigColor = trafficSignal->getPedestrianLight(*currCrossing);
//			std::cout<<"Debug: signal color "<<sigColor<<std::endl;
		}
		else
			std::cout<<"Current crossing not found!"<<std::endl;
	}


//	Agent* a = nullptr;
//	Signal* s = nullptr;
//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
//		//Skip self
//		a = Agent::all_agents[i];
//		if (a->getId()==parent->getId()) {
//			a = nullptr;
//			continue;
//		}
//
//		if (dynamic_cast<Signal*>(a)) {
//		   s = dynamic_cast<Signal*>(a);
//		   currPhase=1;//s->get_Pedestrian_Light(0);
//			//It's a signal
//		}
//		s = nullptr;
//		a = nullptr;
//	}
//	s = nullptr;
//	a = nullptr;

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

//	//Search for the nearest driver on the current link
//	Agent* a = nullptr;
//	Person* p = nullptr;
//	double pedRelX, pedRelY, drvRelX, drvRelY;
//	double minDist=1000000;
//	for (size_t i=0; i<Agent::all_agents.size(); i++) {
//		//Skip self
//		a = Agent::all_agents[i];
//		if (a->getId()==parent->getId()) {
//			a = nullptr;
//			continue;
//		}
//
//		if(dynamic_cast<Person*>(a)){
//			p=dynamic_cast<Person*>(a);
//			if(dynamic_cast<Driver*>(p->getRole())){
//				if(p->currentLink.get()==0){
//					//std::cout<<"dsahf"<<std::endl;
//					absToRel(p->xPos.get(),p->yPos.get(),drvRelX,drvRelY);
//					absToRel(parent->xPos.get(),parent->xPos.get(),pedRelX,pedRelY);
//					if(minDist>abs(drvRelY-pedRelY))
//						minDist = abs(drvRelY-pedRelY);
//				}
//			}
//		}
//		p = nullptr;
//		a = nullptr;
//	}
//	p = nullptr;
//	a = nullptr;
//
//	if((minDist/5-30/speed)>1)
//	{
//
//		return true;
//	}
//	else
//		return false;

	return false;
}

/*---------------------Action-related functions--------------------------*/

void sim_mob::Pedestrian::updateVelocity(int flag) //0-on sidewalk, 1-on crossing green, 2-on crossing red
{
	//Set direction (towards the goal)
	double scale;
	xVel = ((double)(goal.getX() - parent->xPos.get()))/100;
	yVel = ((double)(goal.getY() - parent->yPos.get()))/100;
	//Normalize
	double length = sqrt(xVel*xVel + yVel*yVel);
	xVel = xVel/length;
	yVel = yVel/length;
	//Set actual velocity
	if(flag==0)
		scale = 20;
	else if(flag==1)
		scale = 1.5;
	else if (flag==2)
		scale = 3;
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
	int newX = (int)(parent->xPos.get()+ xVel*100*(((double)ConfigParams::GetInstance().agentTimeStepInMilliSeconds())/1000));
	int newY = (int)(parent->yPos.get()+ yVel*100*(((double)ConfigParams::GetInstance().agentTimeStepInMilliSeconds())/1000));

//	int newX = (int)(parent->xPos.get()+ xVel*100*1);
//	int newY = (int)(parent->yPos.get()+ yVel*100*1);

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

void sim_mob::Pedestrian::setCrossingParas(){

	//TEMP: for testing on self-created network only

	//Set agents' start crossing locations
//	double xRel = -30;
////	double yRel = 30+(double)(rand()%6);
//	double yRel = 30+(double)(rand()%6)+(double)(rand()%6);
//	double xAbs=0;
//	double yAbs=0;
//	double width, length, tmp;
//	relToAbs(xRel,yRel,xAbs,yAbs);
//	parent->xPos.set(xAbs);
//	parent->yPos.set(yAbs);
//	//Set agents' end crossing locations
//	xRel=30;
//	relToAbs(xRel,yRel,xAbs,yAbs);
//	goal = Point2D(int(xAbs*100),int(yAbs*100));


	double xRel, yRel;
	double xAbs, yAbs;
	double width, length, tmp;

	//Get traffic signal
	const Node* node = ConfigParams::GetInstance().getNetwork().locateNode(goal, true);
	if(node)
		trafficSignal = StreetDirectory::instance().signalAt(*node);
	else
		trafficSignal = nullptr;

	const RoadSegment* segToCross;
//	const Crossing* crossing;
	std::set<sim_mob::RoadSegment*>::const_iterator i;
	const MultiNode* currNode=dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(goal, true));
//	const MultiNode* end=dynamic_cast<const MultiNode*>(ConfigParams::GetInstance().getNetwork().locateNode(Point2D(37270984,14378959), true));
	const std::set<sim_mob::RoadSegment*>& roadsegments=currNode->getRoadSegments();

//	for(i=roadsegments.begin();i!=roadsegments.end();i++){
//		if((*i)->getLink()->getStart()==end){
//			segToCross = (*i);
//			break;
//		}
//		else if((*i)->getLink()->getEnd()==end){
//			segToCross = (*i);
//			break;
//		}
//	}

	for(i=roadsegments.begin();i!=roadsegments.end();i++){
		if((*i)->getLink()->getStart()!=parent->originNode&&(*i)->getLink()->getEnd()!=parent->originNode&&(*i)->getLink()->getStart()!=parent->destNode&&(*i)->getLink()->getEnd()!=parent->destNode){
			cStartX=(double)goal.getX();
			cStartY=(double)goal.getY();
			cEndX=(double)parent->destNode->location->getX();
			cEndY=(double)parent->destNode->location->getY();
			if((*i)->getStart()==currNode){
				absToRel((*i)->getEnd()->location->getX(),(*i)->getEnd()->location->getY(),xRel,yRel);
				if(yRel<0){
					segToCross=(*i);
					break;
				}
			}
			else{
				absToRel((*i)->getStart()->location->getX(),(*i)->getStart()->location->getY(),xRel,yRel);
				if(yRel<0){
					segToCross=(*i);
					break;
				}
			}
//			segToCross=(*i);
//			break;
		}
	}

	if(segToCross->getStart()==currNode){
		 currCrossing=dynamic_cast<const Crossing*>(segToCross->nextObstacle(0,true).item);
	}
	else{
		 currCrossing=dynamic_cast<const Crossing*>(segToCross->nextObstacle(segToCross->length,true).item);
	}

	if(currCrossing)
	{
		Point2D far1 = currCrossing->farLine.first;
		Point2D far2 = currCrossing->farLine.second;
		Point2D near1 = currCrossing->nearLine.first;
		Point2D near2 = currCrossing->nearLine.second;

		//Determine the direction of two points
		if((near1.getY()>near2.getY()&&goal.getY()>parent->originNode->location->getY())||(near1.getY()<near2.getY()&&goal.getY()<parent->originNode->location->getY())){
			cStartX=(double)near2.getX();
			cStartY=(double)near2.getY();
			cEndX=(double)near1.getX();
			cEndY=(double)near1.getY();
			absToRel(cEndX,cEndY,length,tmp);
			absToRel((double)far2.getX(),(double)far2.getY(),tmp,width);
		}
		else{
			cStartX=(double)near1.getX();
			cStartY=(double)near1.getY();
			cEndX=(double)near2.getX();
			cEndY=(double)near2.getY();
			absToRel(cEndX,cEndY,length,tmp);
			absToRel((double)far1.getX(),(double)far1.getY(),tmp,width);
		}

		xRel = 0;
		if(width<0)
			yRel = -((double)(rand()%(int(abs(width)/2+1)))+(double)(rand()%(int(abs(width)/2+1))));
		else
			yRel = (double)(rand()%(int(width/2+1)))+(double)(rand()%(int(width/2+1)));
		xRel = (yRel*tmp)/width;
		relToAbs(xRel,yRel,xAbs,yAbs);
		parent->xPos.set((int)xAbs);
		parent->yPos.set((int)yAbs);
		xRel = xRel+length;
		relToAbs(xRel,yRel,xAbs,yAbs);
		goal = Point2D((int)xAbs,(int)yAbs);

//		double slope1, slope2;
//		slope1 = (double)(far2.getY()-far1.getY())/(far2.getX()-far1.getX());

	}
	else
		std::cout<<"Crossing not found!"<<std::endl;


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

bool sim_mob::Pedestrian::isOnCrossing(){
	if(currentStage==1&&startToCross==true)
		return true;
	else
		return false;
}








