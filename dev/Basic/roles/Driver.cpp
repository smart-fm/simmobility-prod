/*
 * Driver.cpp
 *
 *  Created on: 2011-7-5
 *      Author: wangxy
 */


#include "Driver.hpp"

using namespace sim_mob;
using std::numeric_limits;



//Some static properties require initialization in the CPP file. ~Seth
const double sim_mob::Driver::maxLaneSpeed[] = {120,140,180};
const double sim_mob::Driver::lane[] = {300,320,340};
const double sim_mob::Driver::MAX_NUM = numeric_limits<double>::max();
const double sim_mob::Driver::laneWidth = 20;


//initiate
sim_mob::Driver::Driver(Agent* parent) : Role(parent)
{
	//Set random seed
	srand(parent->getId());

	//Set default speed in the range of 1m/s to 1.4m/s
	speed = 1+((double)(rand()%10))/10;
	//speed up
	speed *= 80;

	//Set default data for acceleration
	acc = 0;
	maxAcceleration = 40;
	normalDeceleration = -maxAcceleration*0.6;
	maxDeceleration = -maxAcceleration;

	//Other basic parameters
	xPos = 0;
	yPos = 0;
	xVel = 0;
	yVel = 0;
	xAcc = 0;
	yAcc = 0;

	//assume that all the car has the same size
	length=20;
	width=8;

	targetSpeed=speed*2;

	timeStep=0.1;			//assume that time step is constant
	isGoalSet = false;
	isOriginSet = false;
	LF=NULL;LB=NULL;RF=NULL;RB=NULL;

	ischanging=false;
	isback=false;
	fromLane=getLane();
	toLane=getLane();
}


//Main update functionality
void sim_mob::Driver::update(frame_t frameNumber)
{
	//Set the goal of agent
	if(!isGoalSet){
		setGoal();
		isGoalSet = true;
	}
	//Set the origin of agent
	if(!isOriginSet){
		setOrigin();
		isOriginSet=true;
	}
	//To check if the vehicle reaches the lane it wants to move to
	if(getLane()==toLane){
		ischanging=false;
		isback=false;
		fromLane=toLane=getLane();
		changeDecision=0;
	}

	//update information
	updateLeadingDriver();
	updateAcceleration();
	updateVelocity();
	updatePosition();

	//lane changing part
	excuteLaneChanging();
}

void sim_mob::Driver::setOrigin()
{
	origin.xPos = -100;
	origin.yPos=parent->yPos.get();
}

void sim_mob::Driver::setGoal()
{
	goal.xPos = 1000;			//all the cars move in x dirction to reach the goal
}

bool sim_mob::Driver::isGoalReached()
{
	if( (goal.xPos - parent->xPos.get())<0) return true;
	else return false;
}


void sim_mob::Driver::updateAcceleration()
{
	makeAcceleratingDecision();
	//Set direction (towards the goal)
	double xDirection = goal.xPos - parent->xPos.get();
	double yDirection = 0;

	//Normalize
	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
	xDirection = xDirection/magnitude;
	yDirection = yDirection/magnitude;

	//Set actual acceleration
	xAcc = xDirection*acc;
	yAcc = yDirection*acc;
	parent->xAcc.set(xAcc);
	parent->yAcc.set(yAcc);
}


void sim_mob::Driver::updateVelocity()
{
	//Set direction (towards the goal)
	double xDirection = goal.xPos - parent->xPos.get();
	double yDirection = 0;

	//Normalize
	double magnitude = sqrt(xDirection*xDirection + yDirection*yDirection);
	xDirection = xDirection/magnitude;
	yDirection = yDirection/magnitude;

	//when vehicle just gets back to the origin, help them to speed up
	if(parent->xPos.get()<0){
		xVel=(0.2+((double)(rand()%10))/30)*getTargetSpeed();
		yVel=0;
	}
	else{
	xVel = xDirection*speed+xAcc*timeStep;
	yVel = yDirection*speed+yAcc*timeStep;
	if(xVel<0)xVel=0;
	if(yVel<0)yVel=0;
	}
	//if(!ischanging){
		double foward;
		if(parent->leader==NULL)foward=MAX_NUM;
		else foward=parent->leader->xPos.get()-parent->xPos.get()-length;
		if(foward<0)xVel=0;
		yVel=0;
	//}
	//Set actual velocity
	parent->xVel.set(xVel);
	parent->yVel.set(yVel);
	speed=sqrt(xVel*xVel+yVel*yVel);
}


void sim_mob::Driver::updatePosition()
{
	//Compute
	if(xVel==0)xPos=parent->xPos.get();			//when speed is zero, stop in the same position
	else
		xPos = parent->xPos.get()+xVel*timeStep+0.5*xAcc*timeStep*timeStep;
	yPos=parent->yPos.get();

	//Set
	parent->xPos.set(xPos);
	//if reach the goal, get back to the origin and avoid crash
	if(isGoalReached()){
		double fallback=0;//(double)(rand()%4);//when crash is about to happen, get back 50 units
		parent->xPos.set(origin.xPos-fallback*50);
		parent->yPos.set(origin.yPos);
		xPos=origin.xPos-fallback*50;;
		yPos=origin.yPos;
		//parent->xVel.set(parent->xVel.get());
		//parent->xAcc.set(parent->xAcc.get());
	}
}


void sim_mob::Driver::updateLeadingDriver()
{
	Agent* other = NULL;
	// In fact the so-called big brother can return the leading driver.
	// Since now there is no such big brother, so I assume that each driver can know information of other vehicles
	// It will find it's leading vehicle itself.
	double leadingDistance=MAX_NUM;
	size_t leadingID=Agent::all_agents.size();
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==parent->getId()) {
			other = NULL;
			continue;
		}
		//Check.
		//When the vehicle is changing lane, it will search both the lane it is from and towards
		//When the vehicle is on a lane, it will search all the vehicle it may get crashed.
		if(other->yPos.get()<parent->yPos.get()+width && other->yPos.get()>parent->yPos.get()-width){
			double tmpLeadingDistance=other->xPos.get()-parent->xPos.get();
			if(tmpLeadingDistance<leadingDistance && tmpLeadingDistance >0)	{
				leadingDistance=tmpLeadingDistance;
				leadingID=i;
			}
		}
	}
	if(leadingID == Agent::all_agents.size())leadingDriver=NULL;
	else leadingDriver=Agent::all_agents[leadingID];
	parent->leader=leadingDriver;
}

int sim_mob::Driver::getLane()
{
	for (int i=0;i<3;i++){
		if(parent->yPos.get()==lane[i])return i;
	}
	return -1;
}


double sim_mob::Driver::getDistance()
{
	if(parent->leader == NULL) return MAX_NUM;
	else {
		double temp=parent->leader->xPos.get()-parent->xPos.get()-length;
		return (temp<0)?0:temp;
	}
}


void sim_mob::Driver::makeAcceleratingDecision()
{
	space = getDistance();
	if (speed == 0)headway = 2 * space * 100000;
	else headway = space / speed;
	if(parent->leader == NULL){
		v_lead		=	MAX_NUM;
		a_lead		=	MAX_NUM;
		space_star	=	MAX_NUM;
	}
	else{
		v_lead 		=	parent->leader->xVel.get();
		a_lead		=	parent->leader->xAcc.get();
		double dt	=	timeStep;
		space_star	=	space + v_lead * dt + 0.5 * a_lead * dt * dt;
	}
	if(headway < hBufferLower) acc = accOfEmergencyDecelerating();
	if(headway > hBufferUpper) acc = accOfMixOfCFandFF();
	if(headway <= hBufferUpper && headway >= hBufferLower) acc = accOfCarFollowing();
}

double sim_mob::Driver::breakToTargetSpeed()
{
	double v 			=	parent->xVel.get();
	double dt			=	timeStep;
	if( space_star > FLT_EPSILON)
		return (( v_lead + a_lead * dt ) * ( v_lead + a_lead * dt) - v * v) / 2 / space_star;
	else if ( dt <= 0 )
		return MAX_ACCELERATION;
	else return ( v_lead + a_lead * dt - v ) / dt;
}

double sim_mob::Driver::accOfEmergencyDecelerating()
{
	double v 			=	parent->xVel.get();
	double dv			=	v-v_lead;
	double epsilon_v	=	0.001;
	double aNormalDec	=	-getNormalDeceleration();

	if( dv < epsilon_v ) return a_lead + 0.25*aNormalDec;
	else if ( space > 0.01 ) return a_lead - dv * dv / 2 / space;
	else return breakToTargetSpeed();
}

double sim_mob::Driver::accOfCarFollowing()
{
	double alpha[2] 	=	{1,1};		//[0] for positive   [1] for negative
	double beta[2] 		=	{1,1};
	double gama[2]		=	{1,1};
	double v			=	parent->xVel.get();
	int i = (v > v_lead) ? 1 : 0;
	double acc_ = alpha[i] * pow(v , beta[i]) /pow(space , gama[i]) * (v_lead - v);
	return acc_;
}

double sim_mob::Driver::accOfFreeFlowing()
{
	double vn			=	speed;
	double acc_;
	if ( vn < getTargetSpeed()){
		if( vn < maxLaneSpeed[getLane()]) acc_=getMaxAcceleration();
		else acc_ = getNormalDeceleration();
	}
	if ( vn > getTargetSpeed())acc_ = getNormalDeceleration();
	if ( vn == getTargetSpeed()){
		if( vn < maxLaneSpeed[getLane()]) acc_=getMaxAcceleration();
		else acc_ = 0;
	}
	return acc_;
}

double sim_mob::Driver::accOfMixOfCFandFF()
{
	distanceToNormalStop = speed * speed / 2 /getNormalDeceleration();
	if( space > distanceToNormalStop ) return accOfFreeFlowing();
	else return breakToTargetSpeed();
}


Agent* sim_mob::Driver::getNextForBDriver(bool isLeft,bool isFront)
{
	int border;
	double offset;

	if(isLeft) {
		border = 0;
		offset = Driver::laneWidth;
	} else{
		border = 2;
		offset = -Driver::laneWidth;
	}

	double NFBDistance;
	if(isFront)NFBDistance=MAX_NUM;
	else NFBDistance=-MAX_NUM;
	size_t NFBID=Agent::all_agents.size();
	if(getLane()==border)return NULL;		//has no left side or right side
	else {
		Agent* other = NULL;
		for (size_t i=0; i<Agent::all_agents.size(); i++) {
			//Skip self
			other = Agent::all_agents[i];
			if (other->getId()==parent->getId()) {
				other = NULL;
				continue;
			}
			//Check.
			if(other->yPos.get() == parent->yPos.get()-offset) {	//now it just searches vehicles on the lane
				double forward=other->xPos.get()-parent->xPos.get();
				if(
						(isFront && forward>0 && forward < NFBDistance)||
						((!isFront) && forward<0 && forward > NFBDistance)
						){
					NFBDistance=forward;NFBID=i;
				}
			}
		}
	}
	if(NFBID == Agent::all_agents.size())return NULL;
	else return Agent::all_agents[NFBID];
}

int sim_mob::Driver::gapAcceptance()
{
	int border[2]={0,2};				//[0] for left, [1] for right
	LF=getNextForBDriver(true,true);
	LB=getNextForBDriver(true,false);
	RF=getNextForBDriver(false,true);
	RB=getNextForBDriver(false,false);
	Agent* F;
	Agent* B;
	bool flagF[2]={false,false},flagB[2]={false,false};
	for(int i=0;i<2;i++){
		if(i==0){F=LF;B=LB;}
		else{F=RF;B=RB;}
		if(getLane()!=border[i]){		//if it has left lane or right lane
			if(F!=NULL){
				double gna=F->xPos.get()-parent->xPos.get()-length;
				if(gna > getTimeStep()*(parent->xVel.get()-F->xVel.get())) flagF[i]=true;
				else flagF[i]=false;
			}
			else flagF[i]=true;

			if(B!=NULL){
				double gnb=parent->xPos.get()-B->xPos.get()-length;
				if(gnb > getTimeStep()*(B->xVel.get()-parent->xVel.get())) flagB[i]=true;
				else flagB[i]=false;
			}
			else flagB[i]=true;
		}
	}
	bool canL=false,canR=false;
	if(flagF[0]&&flagB[0])canL=true;
	if(flagF[1]&&flagB[1])canR=true;
	if(canL && canR)return 2;
	if(canL && !canR)return -1;
	if(!canL && canR)return 1;
	return 0;
}

double sim_mob::Driver::makeLaneChangingDecision()
{
	// for available gaps(including current gap between leading vehicle and itself), vehicle will choose the longest
	int i=gapAcceptance();
	bool left,right;
	double s=getDistance();
	double sl,sr;
	if(RF!=NULL)sr=RF->xPos.get()-parent->xPos.get()-length;
	else sr=MAX_NUM;
	if(s<sr)right=true;
	else right=false;
	if(LF!=NULL)sl=LF->xPos.get()-parent->xPos.get()-length;
	else sl=MAX_NUM;
	if(s<sl)left=true;
	else left=false;
	if(i==1 && right)return 1;
	if(i==-1 && left)return -1;
	if(i==2){
		if(right && left){
			if(sr>sl)return 1;
			else if(sr<sl)return -1;
			else return 2*rand()%2-1;
			//else return 1;
		}
		if(right && !left)return 1;
		if(!right && left)return -1;
		if(!right && !left)return 0;
	}
	return 0;
}

void sim_mob::Driver::excuteLaneChanging()
{
	if(!ischanging){
		double change=makeLaneChangingDecision();
		changeDecision=change;
		fromLane=getLane();
		toLane=getLane()+change;
	}
	if(changeDecision==0)yPos=parent->yPos.get();
	else{
		// when crash will happen, exchange the leaving lane and target lane
		if(checkForCrash() && !isback){
			int tmp;
			tmp=fromLane;
			fromLane=toLane;
			toLane=tmp;
			changeDecision=-changeDecision;
			isback=true;
		}
		ischanging=true;
		parent->yPos.set(yPos+changeDecision*VelOfLaneChanging);
	}
}


bool sim_mob::Driver::checkForCrash()
{
	if(!ischanging)return false;
	Agent* other = NULL;
	for (size_t i=0; i<Agent::all_agents.size(); i++) {
		//Skip self
		other = Agent::all_agents[i];
		if (other->getId()==parent->getId()) {
			other = NULL;
			continue;
		}
		//Check. when other vehicle is too close to subject vehicle, crash will happen
		if(
				(other->yPos.get() < parent->yPos.get()+width*1.1) &&
				(other->yPos.get() > parent->yPos.get()-width*1.1) &&
				(other->xPos.get() < parent->xPos.get()+length*1.1) &&
				(other->xPos.get() > parent->xPos.get()-length*1.1))
			return true;
	}
	return false;
}
