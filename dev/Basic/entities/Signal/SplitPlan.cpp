#include "SplitPlan.hpp"

namespace sim_mob
{
std::size_t SplitPlan::getCycleLength() {return cycleLength;}

void SplitPlan::setCycleLength(std::size_t c) {cycleLength = c;}

std::size_t SplitPlan::CurrSplitPlanID() { return currSplitPlanID; }

void SplitPlan::setcurrSplitPlanID(std::size_t index) { currSplitPlanID = index; }

std::size_t SplitPlan::getOffset() {return offset;}

void SplitPlan::setOffset(std::size_t o) {offset = o;}

//const std::vector<sim_mob::Phase> & SplitPlan::getPhases() const { return phases_; }
//std::vector<sim_mob::Phase> & SplitPlan::getPhases() { return phases_; }

void SplitPlan::addPhase(sim_mob::Phase phase) { phases_.push_back(phase); }

std::size_t & SplitPlan::CurrPhaseID() { return currPhaseID; }

const  sim_mob::Phase & SplitPlan::CurrPhase() const { return phases_[currPhaseID]; }

/*
 * This function has 2 duties
 * 1- Update the Votes data structure
 * 2- Return the index having the highst value(vote) with the help of another function(getMaxVote())
 */
std::size_t SplitPlan::Vote(std::vector<double> maxproDS) {
	std::vector<int> vote(NOF_Plans,0);//choiceSet.size()=no of plans
	vote[fmin_ID(maxproDS)]++;//the corresponding split plan's vote is incremented by one(the rests are zero)
	votes.push_back(vote);
	if(votes.size() > NUMBER_OF_VOTING_CYCLES) votes.erase(votes.begin()); //removing the oldest vote if necessary(wee keep only NUMBER_OF_VOTING_CYCLES records)
	/*
	 * step 3: The split plan with the highest vote in the last certain number of cycles will win the vote
	 *         no need to eat up your brain, in the following chunk of code I will get the id of the maximum
	 *         value in vote vector and that id will actually be my nextSplitPlanID
	*/
	return getMaxVote();
}
//calculate the projected DS and max Projected DS for each split plan (for internal use only, refer to section 4.3 table-4)
void SplitPlan::calMaxProDS(std::vector<double>  &maxproDS,std::vector<double> DS)
{
	double maax=0;
	vector<double> proDS(NOF_Phases, 0);
	for(int i=0; i < NOF_Plans; i++)//traversing the columns of Phase::choiceSet matrix
	{
		for(int j=0, maax = 0; j < NOF_Phases; j++)
		{
			//calculate the projected DS for this plan
			proDS[j] = DS[j] * choiceSet[currSplitPlanID][j]/choiceSet[i][j];
			//then find the Maximum Projected DS for each Split plan(used in the next step)
			if(maax < proDS[j]) maax = proDS[j];
		}
		maxproDS[i] = maax;
	}
}


//4.3 Split Plan Selection (use DS to choose SplitPlan for next cycle)(section 4.3 of the Li Qu's manual)
std::size_t SplitPlan::findNextPlanIndex(std::vector<double> DS) {

	std::vector<double>  maxproDS(NOF_Plans,0);// max projected DS of each SplitPlan

	int i,j;
	//step 1:Calculate the Max projected DS for each approach (and find the maximum projected DS)
	calMaxProDS(maxproDS,DS);
	//step2 2 & 3 in one function to save your brain.
	//Step 2: The split plan with the ' lowest "Maximum Projected DS" ' will get a vote
	//step 3: The split plan with the highest vote in the last certain number of cycles will win the vote
	nextSplitPlanID = Vote(maxproDS);
	return nextSplitPlanID;
}

void SplitPlan::updatecurrSplitPlan() {
	currSplitPlanID = nextSplitPlanID;
}

///////////////////////////////Not so Important //////////////////////////////////////////////////////////////////////

//find the minimum among the max projected DS
double SplitPlan::fmin_ID(std::vector<double> maxproDS) {
	int min = 0;
	for (int i = 1; i < maxproDS.size(); i++) {
		if (maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;//(Note: not minimum value ! but minimum value's "index")
}

//find the split plan Id which currently has the maximum vote
std::size_t SplitPlan::getMaxVote()
{
	int PlanId_w_MaxVote = -1 , SplitPlanID , max_vote_value = -1, vote_sum = 0;
	for(SplitPlanID = 0 ; SplitPlanID < NOF_Plans; SplitPlanID++)
	{
		for(int i=0, vote_sum = 0; i < NUMBER_OF_VOTING_CYCLES ; vote_sum += votes[SplitPlanID][i++]);//calculating sum of votes in each column
		if(max_vote_value < vote_sum)
		{
			max_vote_value = vote_sum;
			PlanId_w_MaxVote = SplitPlanID;// SplitPlanID with max vote so far
		}
	}
	return PlanId_w_MaxVote;
}

std::vector< double >  SplitPlan::CurrSplitPlan()
{
	if(choiceSet.size() == 0)
		std::cout << "Choice Set is empty the progrma can crash without it" << std::endl;
	return choiceSet[currSplitPlanID];
}

std::size_t SplitPlan::nofPhases()
{
	return NOF_Phases;
}
std::size_t SplitPlan::nofPlans()
{
	return NOF_Plans;
}

void SplitPlan::Update(std::vector<double> DS)
{
		findNextPlanIndex(DS);
		updatecurrSplitPlan();
}
/*
 * find out which phase we are in the current plan
 * based on the currCycleTimer(= cycle-time lapse so far)
 */
std::size_t SplitPlan::computeCurrPhase(double currCycleTimer)
{
	std::vector< double > currSplitPlan = CurrSplitPlan();

	double sum = 0;
	int i;
	for(i = 0; i < NOF_Phases; )
	{
		//expanded the single line for loop for better understanding of future readers
		sum += cycleLength * currSplitPlan[i++] / 100;
		if(sum > currCycleTimer) break;
	}

	return (std::size_t)(i-1);
}

SplitPlan::SplitPlan()
{
	currPhaseID = 0;
	nextSplitPlanID = 0;
	currSplitPlanID = 0;
	NOF_Phases = 0;
	NOF_Plans = 0;
	cycleLength = 0;
	offset = 0;
	/*default choice set(percentage)*/
	double defaultChoiceSet[5][4] = {
			{30,30,20,20},
			{20,35,20,25},
			{35,35,20,10},
			{35,30,10,25},
			{20,35,25,20}
	};
	choiceSet.resize(5, vector<double>(4));
	for(int i = 0; i < 5; i++)
		for(int j = 0; j < 4; j++)
			choiceSet[i][j] = defaultChoiceSet[i][j];
}

};//namespace

////find te split plan Id which currently has the maximum vote
//int SplitPlan::getPlanId_w_MaxVote()
//{
//	int PlanId_w_MaxVote = -1 , SplitPlanID , max_vote_value = -1, vote_sum = 0;
//	for(SplitPlanID = 0 ; SplitPlanID < votes.size(); SplitPlanID++)
//	{
//		for(int i=0, vote_sum = 0; i < NUMBER_OF_VOTING_CYCLES ; vote_sum += votes[SplitPlanID][i++]);//calculating sum of votes in each column
//		if(max_vote_value < vote_sum)
//		{
//			max_vote_value = vote_sum;
//			PlanId_w_MaxVote = SplitPlanID;// SplitPlanID with max vote so far
//		}
//	}
//	return PlanId_w_MaxVote;
//}
//
////4.3 Split Plan Selection (use DS to choose SplitPlan for next cycle)(section 4.3 of the Li Qu's manual)
//void SplitPlan::setnextSplitPlan(std::vector<double> DS) {
//	std::vector<int> vote(SplitPlan.size(),0);
//	std::vector<double> proDS(DS.size(),0);// projected DS
//	std::vector<double>  maxproDS(plan_.getPhases().size(),0);// max projected DS of each SplitPlan
//	int i,j;
//
//	//step 1:Calculate the projected DS for each approach (and find the maximum projected DS)
//	calProDS_MaxProDS(proDS,maxproDS);
//	//Step 2: The split plan with the ' lowest "Maximum Projected DS" ' will get a vote
//	vote[fmin_ID(maxproDS)]++;//the corresponding split plan's vote is incremented by one(the rests are zero)
//	votes.push_back(vote);
//	if(votes.size() > NUMBER_OF_VOTING_CYCLES) votes.erase(0); //removing the oldest vote if necessary(wee keep only NUMBER_OF_VOTING_CYCLES records)
//	/*
//	 * step 3: The split plan with the highest vote in the last certain number of cycles will win the vote
//	 *         no need to eat up your brain, in the following chunk of code I will get the id of the maximum
//	 *         value in vote vector and that id will actually be my nextSplitPlanID
//	*/
//	nextSplitPlanID = getPlanId_w_MaxVote();
//	//enjoy the result
//	nextSplitPlan = SplitPlan[nextSplitPlanID];
//}
