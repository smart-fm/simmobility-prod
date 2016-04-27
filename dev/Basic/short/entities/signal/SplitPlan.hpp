//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include "Cycle.hpp"
#include "Phase.hpp"

namespace sim_mob
{

class Signal_SCATS;

enum TrafficControlMode
{
	FIXED_TIME,
	ADAPTIVE
};

class SplitPlan
{
private:
	/**Indicates whether the control mode is fixed-time or adaptive*/
	TrafficControlMode controlMode;
	
	/**Length of the cycle*/
	double cycleLength;
	
	/**Offset*/
	double offset;
	
	/**Number of split plans. This is equal to the size of the choice set*/
	std::size_t numOfPlans;

	/**The index of the current split plan*/
	std::size_t currSplitPlanIdx;
	
	/**The index of the next split plan*/
	std::size_t nextSplitPlanIdx;

	/**The cycle*/
	Cycle cycle;
	
	/**The parent signal to which the split plan belongs*/
	Signal_SCATS *parentSignal;

	/*
	 * This specifies the various choice set combinations that can be assigned to phases.
	 * Note that in this matrix the outer vector denote columns and inner vector denotes rows (reverse to common sense):
	 * 1- The size of the inner vector = the number of phases
	 * 2- currPlanIndex is actually one of the index values of the outer vector.
	 * This can be accessed as choiceSet[Plan][phase]
	 * Every-time a voting procedure is performed, one of the sets of choiceSet are orderly assigned to phases.
	 */
	std::vector< std::vector<double> > choiceSet;

	/* Keeps track of the votes obtained by each split-plan (i.e. phase - choice set combination)
	 * It can be accessed as: votes[cycle][plan]
	 * Usually a history of the last 5 votes is kept. 
	 * 
	 * 			plan1	plan2	plan3	plan4	plan5
	 * 	iter1	1		0		0		0		0
	 * 	iter2	0		1		0		0		0
	 * 	iter3	0		1		0		0		0
	 * 	iter5	1		0		0		0		0
	 * 	iter5	0		0		0		1		0
	 */
	std::vector< std::vector<int> > votes;
	
	/**Updates the current split plan index to the next split plan index*/
	void updatecurrSplitPlan();
	
	/**
	 * Finds the appropriate split plan for the next cycle based on the DS
	 * (section 4.3 of the Li Qu's manual)
	 * @param DS degree of saturation
	 * @return index of the next split plan
	 */
	std::size_t findNextPlanIndex(std::vector<double> DS);
	
	/**
	 * Updates the votes data structure
	 * @param maxprojectedDS maximum projected DS
	 * @return index having the highest vote
	 */
	std::size_t vote(std::vector<double> maxprojectedDS);
	
	/**
	 * Fills the choice set with default ones based on the number of intersection's approaches
	 * @param defaultChoiceSet
	 * @param approaches
	 */
	void fill(double defaultChoiceSet[5][10], int approaches);
	
	/**
	 * Finds the max. degree of saturation
	 * @param phaseDensity the vector holding the degree of saturation values
	 * @return 
	 */
	double getMaxDS(std::vector<double> &phaseDensity);
	
	/**
	 * Finds the split plan index which currently has the maximum vote
	 * Note: In votes, columns represent split plan vote
	 * @return the split plan index with the maximum votes
	 */
	std::size_t getMaxVotedSplitPlan();
	
	/**
	 * Finds the index of the minimum DS among the max projected DS
	 * @param maxproDS
	 * @return 
	 */
	double findIndexOfMinDS(std::vector<double> maxproDS);
	
	/**
	 * Calculates the projected DS and max projected DS for each split plan 
	 * (refer to section 4.3 table-4)
	 * @param maxProjectedDS
	 * @param DS
	 */
	void calcMaxProjectedDS(std::vector<double> &maxProjectedDS, std::vector<double> &DS);

public:
	SplitPlan(double cycleLength_ = 360, double offset_ = 0);
	~SplitPlan();
	
	const std::vector< double >& getCurrSplitPlan() const;
	double getCycleLength() const;
	
	std::size_t getOffset() const;
	void setOffset(std::size_t);
	
	void setParentSignal(Signal_SCATS* parentSignal);
	
	/**
	 * Sets the default split plan
	 * @param 
	 */
	void setDefaultSplitPlan(int);

	/**
	 * Updates the split plan based on the phase densities
	 * @param phaseDensity
	 */
	void update(std::vector<double> &phaseDensity);

	friend class Signal_SCATS;
} ;
}
