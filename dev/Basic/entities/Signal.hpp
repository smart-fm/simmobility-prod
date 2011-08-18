/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */

#pragma once

#include <vector>

#include "../constants.h"
#include "Entity.hpp"


namespace sim_mob
{

/**
 * Basic Signal class.
 *
 * NOTES:
 *1.The signal light uses adaptive signal control strategy (SCATS).
 *
 *2.The input value for SCATS is the degree of saturation(DS),this value
 *  can be recorded by loop detector, for now the loop section is
 *  nothing.We can define the value of DS as default or random.
 *
 *3.I am not so sure about how to add class Signal into the Basic Project,
 *  such as which folder to put it in, entities or roles? It needs to be
 *  updated and every agent should be able to get its information.
 *
 */


class Signal  : public Entity {



public:
//	Signal();

	void initializeSignal();

	//static Signal* GetInstance();


	//DS:degree of saturation
	void updateSignal(double DS[]);

	//set for the parameters in SCATS
	void updateprevCL();
	void updatecurrCL();
	void updateprevRL1 (double RL1);
	void updateprevRL2 (double RL2);
	void setnextCL (double DS);

	void setCL (double prevCL1, double currCL1, double nextCL1) {
		prevCL = prevCL1;
		currCL = currCL1;
		nextCL = nextCL1;
	}
	void setRL (double RL1, double RL2) {
		prevRL1 = RL1;
		prevRL2 = RL2;
	}

	//initialize the SplitPlan for SCATS
	void startSplitPlan();
	void setnextSplitPlan(double DS[]);
	void updatecurrSplitPlan();

	//Offset
	void setnextOffset(double nextCL);
	void updateOffset();


	//get the parameters in SCATS
	double getprevCL() {return prevCL;}
	double getcurrCL() {return currCL;}
	double getnextCL() {return nextCL;}
	double getpreRL1() {return prevRL1;}
	double getpreRL2() {return prevRL2;}
	int getcurrSplitPlanID() {return currSplitPlanID;}
	int getnextSplitPlanID() {return nextSplitPlanID;}
	double * getnextSplitPlan() {return &nextSplitPlan[0];}
	double getcurrOffset() {return currOffset;}
	double getnextOffset() {return nextOffset;}

	//Abstract methods. You will have to implement these eventually.
	virtual void update(frame_t frameNumber) {}
	virtual void buildSubscriptionList() {}


	static double fmax(const double proDS[]);
	static int fmin_ID(const double maxproDS[]);
	static int calvote(unsigned int vote1, unsigned int vote2, unsigned int vote3, unsigned int vote4, unsigned int vote5);

public:
	Signal(unsigned int id) : Entity(id) {}
	int getcurrPhase();
	int getphaseCounter(){return phaseCounter;}
	void updateTrafficLights();
	int get_Driver_Light(int LinkID, int LaneID);
	int get_Pedestrian_Light(int CrossingID);

private:
	//static Signal* instance_ ;
	//previous,current and next cycle length
	double prevCL,currCL,nextCL;

	//two previous RL for calculating the current RL0
	double prevRL1,prevRL2;

	//SplitPlan that can be chosen to use
	static const double SplitPlan1[], SplitPlan2[], SplitPlan3[], SplitPlan4[], SplitPlan5[];

	//current and next SplitPlan
	std::vector<double>currSplitPlan;
	std::vector<double>nextSplitPlan;
	//double currSplitPlan[4],nextSplitPlan[4];


	int currSplitPlanID,nextSplitPlanID;

	//votes for determining next SplitPlan
	int vote1, vote2, vote3, vote4, vote5;

	//current and next Offset
	double currOffset,nextOffset;

	int phaseCounter;
	int currPhase;

	int TC_for_Driver[4][3];
	int TC_for_Pedestrian[4];



};

}

