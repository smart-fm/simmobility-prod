//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SplitPlan.hpp"

#include <cstdio>
#include <sstream>

#include "Signal.hpp"

using std::vector;

namespace
{
const int NUMBER_OF_VOTING_CYCLES = 5;
}

using namespace sim_mob;

SplitPlan::SplitPlan(double cycleLen, double offset) :
cycleLength(cycleLen), offset(offset)
{
	nextSplitPlanIdx = 0;
	currSplitPlanIdx = 0;
	numOfPlans = 0;
	cycle.setCurrentCycleLen(cycleLen);
}

SplitPlan::~SplitPlan()
{
}

const std::vector<double>& SplitPlan::getCurrSplitPlan() const
{
	if (choiceSet.size() == 0)
	{
		std::ostringstream out("");
		out << "Choice Set is empty for signal " << this->parentSignal->getTrafficLightId();
		throw std::runtime_error(out.str());
	}
	return choiceSet[currSplitPlanIdx];
}

void SplitPlan::updatecurrSplitPlan()
{
	currSplitPlanIdx = nextSplitPlanIdx;
}

double SplitPlan::getCycleLength() const
{
	return cycleLength;
}

std::size_t SplitPlan::getOffset() const
{
	return offset;
}

void SplitPlan::setOffset(std::size_t value)
{
	offset = value;
}

void SplitPlan::setParentSignal(Signal_SCATS* parentSignal)
{
	this->parentSignal = parentSignal;
}

std::size_t SplitPlan::vote(std::vector<double> maxprojectedDS)
{
	std::vector<int> vote(numOfPlans, 0);
	
	//The corresponding split plan's vote is incremented by one(the rests are zero)
	vote[findIndexOfMinDS(maxprojectedDS)]++; 
	votes.push_back(vote);
	
	if (votes.size() > NUMBER_OF_VOTING_CYCLES)
	{
		//Remove the oldest vote (we keep only NUMBER_OF_VOTING_CYCLES records)
		votes.erase(votes.begin());
	}
	
	//The split plan with the highest vote in the last certain number of cycles will win the vote
	return getMaxVotedSplitPlan();
}

void SplitPlan::calcMaxProjectedDS(std::vector<double> &maxproDS, std::vector<double> &DS)
{
	vector<double> proDS(parentSignal->getNumOfPhases(), 0);
	
	//Traversing the columns of Phase::choiceSet matrix
	for (int i = 0; i < numOfPlans; i++)
	{
		double max = 0.0;
		for (int j = 0; j < parentSignal->getNumOfPhases(); j++)
		{
			//Calculate the projected DS for this plan
			proDS[j] = DS[j] * choiceSet[currSplitPlanIdx][j] / choiceSet[i][j];
			
			//Then find the Maximum Projected DS for each Split plan(used in the next step)
			if (max < proDS[j])
			{
				max = proDS[j];
			}
		}
		
		maxproDS[i] = max;
	}
}

std::size_t SplitPlan::findNextPlanIndex(std::vector<double> degOfSaturation)
{
	//Max. projected DS of each split plan
	std::vector<double> maxproDS(numOfPlans, 0);

	//1: Calculate the Max projected DS for each approach (and find the maximum projected DS)
	calcMaxProjectedDS(maxproDS, degOfSaturation);
	
	//2: The split plan with the 'lowest "Maximum Projected DS"' will get a vote
	//3: The split plan with the highest vote in the last certain number of cycles will win the vote
	nextSplitPlanIdx = vote(maxproDS);
	
	return nextSplitPlanIdx;
}

double SplitPlan::findIndexOfMinDS(std::vector<double> maxProjectedDS)
{
	int min = 0;
	for (int i = 1; i < maxProjectedDS.size(); i++)
	{
		if (maxProjectedDS[i] < maxProjectedDS[min])
		{
			min = i;
		}
	}
	
	//Note: Not the minimum value! But the 'index' of the minimum value
	return min;
}

std::size_t SplitPlan::getMaxVotedSplitPlan()
{
	int planIdxWithMaxVotes = -1;
	int planIdx;
	int maxVotes = -1;

	for (planIdx = 0; planIdx < numOfPlans; planIdx++)//column iterator(plans)
	{
		//Calculate sum of votes in each column
		int vote_sum = 0;
		
		//Row iterator(cycles)
		for (int i = 0; i < votes.size(); i++) 
		{
			vote_sum += votes[i][planIdx];
		}
		
		if (maxVotes < vote_sum)
		{
			maxVotes = vote_sum;
			
			//split plan index with max vote so far
			planIdxWithMaxVotes = planIdx;
		}
	}
	return planIdxWithMaxVotes;
}

double SplitPlan::getMaxDS(std::vector<double> &phaseDensity)
{
	double max = phaseDensity[0];
	for (int i = 0; i < phaseDensity.size(); i++)
	{
		if (phaseDensity[i] > max)
		{
			max = phaseDensity[i];
		}
	}
	return max;
}

void SplitPlan::update(std::vector<double> &phaseDensity)
{
	double DS_all = getMaxDS(phaseDensity);
	cycle.update(DS_all);
	cycleLength = cycle.getCurrentCycleLen();
	findNextPlanIndex(phaseDensity);
	updatecurrSplitPlan();
}

void SplitPlan::fill(double defaultChoiceSet[5][10], int approaches)
{
	for (int i = 0; i < numOfPlans; i++)
	{
		for (int j = 0; j < approaches; j++)
		{
			choiceSet[i][j] = defaultChoiceSet[i][j];
		}
	}
}

void SplitPlan::setDefaultSplitPlan(int approaches)
{
	double defaultChoiceSet_1[5][10] = {
		{ 100},
		{ 100},
		{ 100},
		{ 100},
		{100}
	};

	double defaultChoiceSet_2[5][10] = {
		{ 50, 50},
		{ 30, 70},
		{ 75, 25},
		{ 60, 40},
		{ 40, 60}
	};

	double defaultChoiceSet_3[5][10] = {
		{ 33, 33, 34},
		{ 40, 20, 40},
		{ 25, 50, 25},
		{ 25, 25, 50},
		{ 50, 25, 25}
	};

	double defaultChoiceSet_4[5][10] = {
		{ 25, 25, 25, 25},
		{ 20, 35, 20, 25},
		{ 35, 35, 20, 10},
		{ 35, 30, 10, 25},
		{ 20, 35, 25, 20}
	};

	double defaultChoiceSet_5[5][10] = {
		{ 20, 20, 20, 20, 20},
		{ 15, 15, 25, 25, 20},
		{ 30, 30, 20, 10, 10},
		{ 25, 25, 20, 15, 15},
		{ 10, 15, 20, 25, 30}
	};

	double defaultChoiceSet_6[5][10] = {
		{ 16, 16, 17, 17, 17, 17},
		{ 10, 15, 30, 20, 15, 10},
		{ 30, 20, 15, 15, 10, 10},
		{ 20, 30, 20, 10, 10, 10},
		{ 15, 15, 20, 20, 15, 15}
	};

	double defaultChoiceSet_7[5][10] = {
		{ 14, 14, 14, 14, 14, 15, 15},
		{ 30, 15, 15, 10, 10, 10, 10},
		{ 15, 30, 10, 15, 10, 10, 10},
		{ 15, 20, 20, 15, 10, 10, 10},
		{ 10, 10, 10, 20, 20, 15, 15}
	};

	numOfPlans = 5;
	choiceSet.resize(numOfPlans, vector<double>(approaches));

	switch (approaches)
	{
	case 1:
		fill(defaultChoiceSet_1, 1);
		break;
	case 2:
		fill(defaultChoiceSet_2, 2);
		break;
	case 3:
		fill(defaultChoiceSet_3, 3);
		break;
	case 4:
		fill(defaultChoiceSet_4, 4);
		break;
	case 5:
		fill(defaultChoiceSet_5, 5);
		break;
	case 6:
		fill(defaultChoiceSet_6, 6);
		break;
	case 7:
		fill(defaultChoiceSet_7, 7);
		break;
	default:
		fill(defaultChoiceSet_7, 7);
		break;
	}

	currSplitPlanIdx = 0;
}
