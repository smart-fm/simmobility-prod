/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Subsystem.cpp
 *
 *  Created on: 2011-7-25
 *      Author: xrm
 */

#include "Subsystem.hpp"
#include <math.h>
using namespace sim_mob;

namespace {

	//Paremeters for offset plan 1
	const int CL_low_1 = 70, CL_up_1 = 120;
	const int Off_low_1 = 5, Off_up_1 = 26;

	//Paremeters for offset plan 2
	const int CL_low_2 = 70, CL_up_2 = 120;
	const int Off_low_2 = 15, Off_up_2 = 36;

	//Paremeters for offset plan 3
	const int CL_low_3 = 70, CL_up_3 = 120;
	const int Off_low_3 = -5, Off_up_3 = -26;

	//Paremeters for offset plan 4
	const int CL_low_4 = 70, CL_up_4 = 120;
	const int Off_low_4 = -15, Off_up_4 = -36;
}

//Directional Bias for each offset plan of each link
const int sim_mob::Subsystem::DB_Link1[] = {1, 0, 0, 0};
const int sim_mob::Subsystem::DB_Link2[] = {0, 1, 0, 0};
const int sim_mob::Subsystem::DB_Link3[] = {0, 0, 1, 0};
const int sim_mob::Subsystem::DB_Link4[] = {0, 0, 0, 1};



sim_mob :: Subsystem :: Subsystem(Agent* parent) : sim_mob::Signal(parent)
{
	Critical->setCL(60,60,60);//default initial cycle length for SCATS
	Critical->setRL(60,60);//default initial RL for SCATS
	S1->setCL(60,60,60);
	S2->setCL(60,60,60);

	Critical->startSplitPlan();
	S1->startSplitPlan();
	S2->startSplitPlan();
}


//update cycle length, split plan, offset in the subsystem
void sim_mob :: Subsystem :: updateSubsystem(double DS_C[],double DS_S1[], double DS_S2[], int flow1[], int flow2[])
{
	Critical->updateSignal(DS_C);
	//Cycle Length of a subsystem is controlled by the critical intersection
	currCycleLength = (int) Critical->getnextCL();
	currSplitPlan_C.assign(Critical->getnextSplitPlan(),Critical->getnextSplitPlan()+3);

	//update SplitPlan of two slaved intersection
	updateSplitPlan(DS_S1, DS_S2);

	//update offset of two slaved intersection
	updateOffset(flow1, flow2);
}

void sim_mob :: Subsystem ::updateSplitPlan(double DS_S1[], double DS_S2[])
{
	//update SplitPlan of slaved intersection1
	S1->setnextSplitPlan(DS_S1);
	currSplitPlan_S1.assign(S1->getnextSplitPlan(),S1->getnextSplitPlan()+3);

	//update SplitPlan of slaved intersection2
	S2->setnextSplitPlan(DS_S2);
	currSplitPlan_S2.assign(S2->getnextSplitPlan(),S2->getnextSplitPlan()+3);

}


//update offset settings for slaved intersections in the subsystem
void sim_mob :: Subsystem :: updateOffset(int flow1[], int flow2[])
{
	//parameters for calculating offset
	int Offset1, Offset2;
	int CL_low , CL_up ;
	int Off_low , Off_up ;
	int planID1, planID2;

	//vote for offset plans
	std::vector<int>planvote1;
	std::vector<int>planvote2;

	//initialize votes
	planvote1.assign(4, 0);
	planvote2.assign(4, 0);

	//calculate the votes for each plan
	for(int i = 0; i < 4; i++){
		planvote1[i] += DB_Link1[i]*flow1[1] + DB_Link2[i]*flow1[2] + DB_Link2[i]*flow1[3] + DB_Link4[i]*flow1[4];
		planvote2[i] += DB_Link1[i]*flow2[1] + DB_Link2[i]*flow2[2] + DB_Link2[i]*flow2[3] + DB_Link4[i]*flow2[4];
	}

	//find the offset plan with maximum votes
	planID1 = findmax(planvote1);
	planID2 = findmax(planvote2);

	//set parameters for slaved intersection1
	switch(planID1 + 1)
	{
		case 1:
			CL_low = CL_low_1;
			CL_up = CL_up_1;
			Off_low = Off_low_1;
			Off_up = Off_up_1;
			break;
		case 2:
			CL_low = CL_low_2;
			CL_up = CL_up_2;
			Off_low = Off_low_2;
			Off_up = Off_up_2;
			break;
		case 3:
			CL_low = CL_low_3;
			CL_up = CL_up_3;
			Off_low = Off_low_3;
			Off_up = Off_up_3;
			break;
		case 4:
			CL_low = CL_low_4;
			CL_up = CL_up_4;
			Off_low = Off_low_4;
			Off_up = Off_up_4;
			break;
		default:
			assert(false);
	}

	//calculate offset for slaved intersection1
	if(currCycleLength <= CL_low) {
		Offset1 = Off_low;
	} else if(currCycleLength > CL_low && currCycleLength <= CL_up) {
		Offset1 = Off_low + (currCycleLength - CL_low)*(Off_up - Off_low)/(CL_up - CL_low);
	} else {
		Offset1 = Off_up;
	}

	//set parameters for slaved intersection2
	switch(planID2 + 1)
	{
		case 1:
			CL_low = CL_low_1;
			CL_up = CL_up_1;
			Off_low = Off_low_1;
			Off_up = Off_up_1;
			break;
		case 2:
			CL_low = CL_low_2;
			CL_up = CL_up_2;
			Off_low = Off_low_2;
			Off_up = Off_up_2;
			break;
		case 3:
			CL_low = CL_low_3;
			CL_up = CL_up_3;
			Off_low = Off_low_3;
			Off_up = Off_up_3;
			break;
		case 4:
			CL_low = CL_low_4;
			CL_up = CL_up_4;
			Off_low = Off_low_4;
			Off_up = Off_up_4;
			break;
		default:
			assert(false);
	}

	//calculate offset for slaved intersection2
	if(currCycleLength <= CL_low) {
		Offset2 = Off_low;
	} else if(currCycleLength > CL_low && currCycleLength <= CL_up) {
		Offset2 = Off_low + (currCycleLength - CL_low)*(Off_up - Off_low)/(CL_up - CL_low);
	} else {
		Offset2 = Off_up;
	}


	//modify offsets for slaved intersection1 & intersection2
	Offset_S1 = currCycleLength*(currSplitPlan_C[0] - currSplitPlan_S1[0]) + Offset1;
	Offset_S2 = currCycleLength*(currSplitPlan_C[0] - currSplitPlan_S2[0]) + Offset2;

}

//find the planID with maximum votes
int sim_mob :: Subsystem :: findmax(std::vector<int>vote)
{
	int maxID = 0;
	for(int i = 1; i < 4; i++){
		if(vote[i] > maxID) {
			maxID = i;
		}
		else{}
	}
	return maxID;
}
