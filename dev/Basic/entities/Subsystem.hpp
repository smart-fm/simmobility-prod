/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Subsystem.hpp
 *
 *  Created on: 2011-7-25
 *      Author: xrm
 */

#pragma once

#include <vector>

#include "../constants.h"
#include "Signal.hpp"


namespace sim_mob
{

/**
 * Basic  Subsystem Signal Control class.
 *
 * NOTES:
 *1.Each subsystem has only one critical intersection. The cycle length of
 *	this subsystem is controlled by the critical intersection.
 *
 *2.Each intersection in the subsystem has its own Split Plan, which determine
 *	the length of each phase.
 *
 *3.In order to set offset for each intersection in the subsystem, we need to
 *	know some parameters such as Direction Bias for each offset plan of each link.
 *	Since I have no data about that now, these parameters are set as default or
 *	random numbers.
 *
 */

class Subsystem : public sim_mob::Signal {

public:
	Subsystem(unsigned int id);

	//update Subsystem
	void updateSubsystem(double DS_C[],double DS_S1[], double DS_S2[], int flow1[], int flow2[]);

	//update cycle length, which is controlled by critical intersection
	void updateCycleLength(double DS_C[]);

	//update split plan for each intersection
	void updateSplitPlan(double DS_S1[], double DS_S2[]);

	//update offset for each intersection
	void updateOffset(int flow1[], int flow2[]);

	virtual void update(frame_t frameNumber) = 0;
	virtual void buildSubscriptionList() = 0;

	static int findmax(std::vector<int>);


private:
	//signal of critical intersection, slaved intersection1 and slaved intersection2
	sim_mob::Signal* Critical;
	sim_mob::Signal* S1;
	sim_mob::Signal* S2;

	// all intersections in one subsystem share a common cycle length
	int currCycleLength;

	// SplitPlan of critical intersection, slaved intersection1 and slaved intersection2
	std::vector<double>currSplitPlan_C;
	std::vector<double>currSplitPlan_S1;
	std::vector<double>currSplitPlan_S2;

	// SplitPlan ofslaved intersection1 and slaved intersection2
	int Offset_S1;
	int Offset_S2;

	//Direction Bias for each offset plan of each link
	static const int DB_Link1[], DB_Link2[], DB_Link3[], DB_Link4[];

};

}
