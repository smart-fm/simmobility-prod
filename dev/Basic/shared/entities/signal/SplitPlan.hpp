#pragma once
#include<map>
#include<vector>
//#include "geospatial/Link.hpp"
#include "defaults.hpp"
#include "Phase.hpp"
#include "Cycle.hpp"
//#include "Offset.hpp"
//#include "conf/simpleconf.hpp"

#define NUMBER_OF_VOTING_CYCLES 5

namespace sim_mob
{
//Forward dseclaration
class Signal_SCATS;

enum TrafficControlMode
{
	FIXED_TIME,
	ADAPTIVE
};

class SplitPlan
{
public:
//	typedef boost::multi_index_container<
//			sim_mob::Phase,
//			boost::multi_index::indexed_by<
//			boost::multi_index::random_access<>
//			,boost::multi_index::ordered_non_unique<boost::multi_index::member<sim_mob::Phase,const std::string, &Phase::name> >
//	  >
//	> phases;
private:
	unsigned int TMP_PlanID;//to identify "this" object(totally different from choice set related terms like currSplitPlanID,nextSplitPlanID....)
    int signalTimingMode;//Fixed plan or adaptive control
	double cycleLength,offset;
	std::size_t NOF_Plans; //NOF_Plans= number of split plans = choiceSet.size()
	std::size_t NOF_Phases; //NOF_Phases = number of phases = phases_.size()
	std::size_t currSplitPlanID;
	std::size_t nextSplitPlanID;
	std::size_t currPhaseID;//Better Name is: phaseAtGreen (according to TE terminology)The phase which is currently undergoing green, f green, amber etc..

	sim_mob::Cycle cycle_;
	phases phases_;
	sim_mob::Signal_SCATS *parentSignal;

	/*
	 * the following variable will specify the various choiceSet combinations that
	 * can be assigned to phases.
	 * therefore we can note that in this matrix the outer vector denote columns and inner vector denotes rows(reverse to common sense):
	 * 1- the size of the inner vector = the number of phases(= the size of the above phases_ vector)
	 * 2- currPlanIndex is actually one of the index values of the outer vector.
	 * everytime a voting procedure is performed, one of the sets of choiceSet are orderly assigned to phases.
	 */

	std::vector< std::vector<double> > choiceSet; //choiceSet[Plan][phase]

	/* the following variable keeps track of the votes obtained by each splitplan(I mean phase choiceSet combination)
	 * ususally a history of the last 5 votings are kept
	 */
	/*
	 * 			plan1	plan2	plan3	plan4	plan5
	 * 	iter1	1		0		0		0		0
	 * 	iter2	0		1		0		0		0
	 * 	iter3	0		1		0		0		0
	 * 	iter5	1		0		0		0		0
	 * 	iter5	0		0		0		1		0
	 */
	std::vector< std::vector<int> > votes;  //votes[cycle][plan]

public:
	typedef boost::multi_index::nth_index_iterator<phases,0>::type phases_iterator;
	typedef boost::multi_index::nth_index_iterator<phases,1>::type phases_name_iterator;
	typedef boost::multi_index::nth_index<phases,1>::type plan_phases_view;
	void get_PlanPhasesByName(plan_phases_view & v) const
	{
		v = boost::multi_index::get<1>(phases_);
	}
	/*plan methods*/
	SplitPlan(double cycleLength_ = 90,double offset_ = 0/*, int signalTimingMode_= ConfigParams::GetInstance().signalTimingMode*/, unsigned int TMP_PlanID_ = 1);
	std::size_t CurrSplitPlanID();
	std::vector< double >  CurrSplitPlan();
	void setCurrPlanIndex(std::size_t);
	std::size_t findNextPlanIndex(std::vector<double> DS);
	void updatecurrSplitPlan();
	std::size_t nofPlans();
	void setcurrSplitPlanID(std::size_t index);
	void setnextSplitPlan(std::vector<double> DS);
	void setCoiceSet(std::vector< std::vector<double> >);
	void setDefaultSplitPlan(int);
	void initialize();

	/*cycle length related methods*/
	double getCycleLength() const ;
	void setCycleLength(std::size_t);

	/*phase related methods*/
	std::size_t & CurrPhaseID();
//	const std::vector<sim_mob::Phase> & getPhases() const;
//	std::vector<sim_mob::Phase> & getPhases();//over load for database loader
	void addPhase(sim_mob::Phase);
	std::size_t nofPhases();
	std::size_t find_NOF_Phases();
	std::size_t computeCurrPhase(double currCycleTimer);
	const sim_mob::Phase & CurrPhase() const;
	int getPhaseIndex(std::string);
	const phases & getPhases(){return phases_;}

	/*offset related methods*/
	std::size_t getOffset();
	void setOffset(std::size_t);

	/*main update mehod*/
	void Update(std::vector<double> &DS);

	/*General Methods*/
	void calMaxProDS(std::vector<double>  &maxproDS,std::vector<double> DS);
	std::size_t Vote(std::vector<double> maxproDS);
	int getPlanId_w_MaxVote();
	double fmin_ID(std::vector<double> maxproDS);
	std::size_t getMaxVote();
	void fill(double defaultChoiceSet[5][10], int approaches);
	std::string createStringRepresentation(std::string newLine);
	void setParentSignal(sim_mob::Signal_SCATS * signal) { parentSignal = signal;}
	sim_mob::Signal_SCATS * getParentSignal() { return parentSignal;}
	void printColors(double printColors);
	double fmax(std::vector<double> &DS);
	std::string  outputTrafficLights(std::string newLine, int phaseId = -1)const;

	/*friends*/
	friend class Signal_SCATS;
};
}
