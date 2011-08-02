/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.cpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */

#include "Signal.hpp"
#include <math.h>
using namespace sim_mob;


double DS_all;

//Private namespace
namespace {
	//parameters for calculating next cycle length
	const double  DSmax = 0.9, DSmed = 0.5, DSmin = 0.3;
	const double CLmax = 140, CLmed = 100, CLmin = 60;

	//parameters for calculating next Offset
	const double CL_low = 70, CL_up = 120;
	const double Off_low = 5, Off_up = 26;
}


const double sim_mob::Signal::SplitPlan1[] = {0.30, 0.30, 0.20, 0.20};
const double sim_mob::Signal::SplitPlan2[] = {0.20, 0.35, 0.20, 0.25};
const double sim_mob::Signal::SplitPlan3[] = {0.35, 0.35, 0.20, 0.10};
const double sim_mob::Signal::SplitPlan4[] = {0.35, 0.30, 0.10, 0.25};
const double sim_mob::Signal::SplitPlan5[] = {0.20, 0.35, 0.25, 0.20};

//Signal* sim_mob::Signal::instance_ = NULL;

/*
sim_mob :: Signal :: Signal(): sim_mob::Entity(id)
{
	setCL(60,60,60);//default initial cycle length for SCATS
	setRL(60,60);//default initial RL for SCATS
	startSplitPlan();
}
*/

void sim_mob :: Signal :: initializeSignal()
{
	setCL(60,60,60);//default initial cycle length for SCATS
	setRL(60,60);//default initial RL for SCATS
	startSplitPlan();
	currPhase = 0;
	phaseCounter = 0;
}


/*Signal* sim_mob :: Signal :: GetInstance()
{
	if(!instance_){
		instance_ = new Signal();
	}
	return instance_;
}*/

//initialize SplitPlan
void sim_mob :: Signal :: startSplitPlan()
{
	//CurrSplitPlan
	currSplitPlanID=1;
	/*for(int i = 0; i < 4; i++) {
		currSplitPlan.push_back(SplitPlan1[i]);
	}*/
	currSplitPlan.assign(SplitPlan1, SplitPlan1+4);  //This does the same thing as the for loop
	nextSplitPlan.assign(4, 0); //Initialize to the number 0, four times.

	//initialize votes for choosing SplitPlan
	vote1 = 0;
	vote2 = 0;
	vote3 = 0;
	vote4 = 0;
	vote5 = 0;
}


//Update Signal Light
void sim_mob :: Signal :: updateSignal (double DS[])
{
	if(phaseCounter == 0){
	//find the maximum DS
	DS_all = fmax(DS);

	//use DS_all for calculating next cycle length
	setnextCL(DS_all);

	//use next cycle length to calculate next Offset
	setnextOffset(getnextCL());

	updateOffset();
	updateprevCL();
	updatecurrCL();
	setnextSplitPlan(DS);
	updatecurrSplitPlan();
	}


	if(phaseCounter<nextCL*nextSplitPlan[0])
	{
		if(phaseCounter <= (nextCL*nextSplitPlan[0] - 3) )currPhase=0;
		else currPhase=10;
	}
	else if(phaseCounter<nextCL*(nextSplitPlan[0]+nextSplitPlan[1]))
	{
		if(phaseCounter <= (nextCL*(nextSplitPlan[0]+nextSplitPlan[1]) - 3) )currPhase=1;
		else currPhase=11;
	}
	else if(phaseCounter<nextCL*(nextSplitPlan[0]+nextSplitPlan[1]+nextSplitPlan[2]))
	{
		if(phaseCounter <= (nextCL*(nextSplitPlan[0]+nextSplitPlan[1]+nextSplitPlan[2]) - 3) )currPhase=2;
		else currPhase=12;
	}
	else if(phaseCounter <= (nextCL-3))currPhase=3;
	else currPhase=13;


	if(phaseCounter == floor(nextCL)){
		phaseCounter = 0;
	}else{}

	phaseCounter++;
}

int sim_mob :: Signal :: getcurrPhase()
{
	return currPhase;
}


//use SCATS to determine next cyecle length
void sim_mob :: Signal :: setnextCL (double DS)
{
	//parameters in SCATS
	double RL0;
	//double diff_CL,diff_CL0;
	double w1 = 0.45, w2 = 0.33, w3 = 0.22;


	//calculate RL0
	if (DS <= DSmed) {
		RL0 = CLmin + (DS - DSmin)*(CLmed - CLmin)/(DSmed - DSmin);
	} else { //if (DS>DSmed)
		RL0=CLmed + (DS - DSmed)*(CLmax - CLmed)/(DSmax - DSmed);
	}
	//else {}


	int sign;
	double diff_CL;
	if(RL0-currCL >= 0) {
		diff_CL = RL0 - currCL; sign = 1;
	} else {
		diff_CL = currCL - RL0; sign = -1;
	}



	//modify the diff_CL0
	double diff_CL0;
	if (diff_CL <= 4) {
		diff_CL0 = diff_CL;
	} else if (diff_CL > 4 && diff_CL <= 8) {
		diff_CL0 = 0.5*diff_CL + 2;
	} else {
		diff_CL0 = 0.25*diff_CL + 4;
	}

	double RL1 = currCL + sign*diff_CL0;

	//RL is partly determined by its previous values
	double RL = w1*RL1 + w2*prevRL1 + w3*prevRL2;

	//update previous RL
	prevRL2 = prevRL1;
	prevRL1 = RL1;


	sign = (RL >= currCL) ? 1 : -1;  //This is equivalent.
	/*if(RL >= currCL) {
		sign = 1;
	} else {
		sign = -1;
	}*/

	//set the maximum change as 6s
	if (abs(RL - currCL) <= 6) {
		nextCL = RL;
	} else {
		nextCL = currCL + sign*6;
	}

	//when the maximum changes in last two cycle are both larger than 6s, the value can be set as 9s
	if ( ((nextCL - currCL) >= 6 && (currCL - prevCL) >= 6) || ((nextCL - currCL) <= -6 && (currCL - prevCL) <= -6) ) {
		if (abs(RL-currCL) <= 9) {
			nextCL = RL;
		} else {
			nextCL = currCL + sign*9;
		}
	}
}


void sim_mob :: Signal :: updateprevCL() {
	prevCL=currCL;
}

void sim_mob :: Signal :: updatecurrCL() {
	currCL=nextCL;
}

void sim_mob :: Signal :: updateprevRL1 (double RL1){
	prevRL1=RL1;
}

void sim_mob :: Signal :: updateprevRL2 (double RL2){
	prevRL2=RL2;
}



//use DS to choose SplitPlan for next cycle
void sim_mob :: Signal :: setnextSplitPlan (double DS[])
{
	double proDS[4];// projected DS
	double maxproDS[6];// max projected DS of each SplitPlan
	//int i;

	//Calculate max proDS of SplitPlan1
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan1[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan1[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan1[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan1[3];
	maxproDS[1]=fmax(proDS);

	//Calculate max proDS of SplitPlan2
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan2[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan2[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan2[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan2[3];
	maxproDS[2]=fmax(proDS);

	//Calculate max proDS of SplitPlan3
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan3[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan3[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan3[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan3[3];
	maxproDS[3]=fmax(proDS);

	//Calculate max proDS of SplitPlan4
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan4[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan4[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan4[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan4[3];
	maxproDS[4]=fmax(proDS);

	//Calculate max proDS of SplitPlan5
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan5[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan5[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan5[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan5[3];
	maxproDS[5]=fmax(proDS);

	//find the minimum value among the max projected DS, vote for its ID;
	vote1=fmin_ID(maxproDS);

	//next SplitPlan is determined by votes in last 5 cycles
	nextSplitPlanID=calvote(vote1,vote2,vote3,vote4,vote5);

	//update votes;
	vote5=vote4;vote4=vote3;vote3=vote2;vote2=vote1;

	//Get a reference to the SplitPlan array
	const double* SplitPlan = nullptr;

	//Retrieve the pointer
	switch(nextSplitPlanID)
	{
		case 1:
			SplitPlan = SplitPlan1;
			break;
		case 2:
			SplitPlan = SplitPlan2;
			break;
		case 3:
			SplitPlan = SplitPlan3;
			break;
		case 4:
			SplitPlan = SplitPlan4;
			break;
		case 5:
			SplitPlan = SplitPlan5;
			break;
		default:
			assert(false);
	}

	//Set the next split plan
	nextSplitPlan[0] = SplitPlan[0];
	nextSplitPlan[1] = SplitPlan[1];
	nextSplitPlan[2] = SplitPlan[2];
	nextSplitPlan[3] = SplitPlan[3];
}



void sim_mob :: Signal :: updatecurrSplitPlan() {
	currSplitPlanID = nextSplitPlanID;
	for(int i = 0; i < 4; i++) {
		currSplitPlan[i] = nextSplitPlan[i];
	}
}


//use next cycle length to calculate next Offset
void sim_mob :: Signal :: setnextOffset(double nextCL)
{
	if(nextCL <= CL_low) {
		nextOffset = Off_low;
	} else if(nextCL > CL_low && nextCL <= CL_up) {
		nextOffset = Off_low + (nextCL - CL_low)*(Off_up - Off_low)/(CL_up - CL_low);
	} else {
		nextOffset = Off_up;
	}
}


void sim_mob :: Signal :: updateOffset(){
	currOffset=nextOffset;
}


//find the max projected DS in each SplitPlan
double sim_mob :: Signal :: fmax(const double proDS[])
{
	double max = proDS[0];
	for(int i = 1; i < 4; i++)
	{
		if(proDS[i] > max) {
			max = proDS[i];
		}
		//else{}
	}
	return max;
}


//find the minimum among the max projected DS
int sim_mob :: Signal :: fmin_ID(const double maxproDS[])
{
	int min=1;
	for (int i = 2; i <= 5; i++)
	{
		if(maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;
}

//determine next SplitPlan according to the votes in last 5 cycle
int sim_mob :: Signal :: calvote(unsigned int vote1,unsigned int vote2, unsigned int vote3, unsigned int vote4, unsigned int vote5)
{
	assert(vote1<6);
	assert(vote2<6);
	assert(vote3<6);
	assert(vote4<6);
	assert(vote5<6);

	int vote_num[6] = {0,0,0,0,0,0};
	int ID = 1;

	vote_num[vote1]++;
	vote_num[vote2]++;
	vote_num[vote3]++;
	vote_num[vote4]++;
	vote_num[vote5]++;
	for(int i = 1; i <= 5; i++) {
		if(vote_num[i] > vote_num[ID]) {
			ID=i;
		}
		//else{}
	}
	return ID;
}
