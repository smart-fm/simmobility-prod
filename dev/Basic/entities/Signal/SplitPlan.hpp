//dont worry guys, i will create the cpp file(s) later.
#include<map>
#include<vector>
#include "geospatial/Link.hpp"
#include "defaults.hpp"
#include "Phase.hpp"
//#include "Offset.hpp"

#define NUMBER_OF_VOTING_CYCLES 5
using namespace std;

namespace sim_mob
{
//Forward dseclaration
class Signal;

enum TrafficControlMode
{
	FIXED_TIME,
	ADAPTIVE
};

class SplitPlan
{
private:
	unsigned int TMP_PlanID;//to identify "this" object(totally different from choice set related terms like currSplitPlanID,nextSplitPlanID....)
    int signalAlgorithm;//Fixed plan or adaptive control
	double cycleLength,offset;
	std::size_t NOF_Plans; //NOF_Plans= number of split plans = percentages.size()
	std::size_t NOF_Phases; //NOF_Phases = number of phases = phases_.size()
	std::size_t currSplitPlanID;
	std::size_t nextSplitPlanID;
	std::size_t currPhaseID;//Better Name is: phaseAtGreen (according to TE terminology)The phase which is currently undergoing green, f green, amber etc..


	std::vector<sim_mob::Phase> phases_;
	/*
	 * the following variable will specify the various percentage combinations that
	 * can be assigned to phases.
	 * therefore we can note that in this matrix the outer vector denote columns and inner vector denotes rows(reverse to common sense):
	 * 1- the size of the inner vector = the number of phases(= the size of the above phases_ vector)
	 * 2- currPlanIndex is actually one of the index values of the outer vector.
	 * everytime a voting procedure is performed, one of the sets of percentaged are orderly assigned to phases.
	 */

	std::vector< vector<double> > percentages; //percentages[Plan][phase]

	/* the following variable keeps track of the votes obtained by each splitplan(I mean phase percentage combination)
	 * ususally a history of the last 5 votings are kept
	 */
	std::vector< std::vector<int> > votes;  //votes[cycle][vote]



public:
	\
	/*plan methods*/
	SplitPlan(){}
	std::size_t CurrSplitPlanID();
	std::vector< double >  CurrSplitPlan();
	void setCurrPlanIndex(std::size_t);
	std::size_t findNextPlanIndex(std::vector<double> DS);
	void updatecurrSplitPlan();
	std::size_t nofPlans();
	void setcurrSplitPlanID(std::size_t index);
	void setnextSplitPlan(std::vector<double> DS);
	std::vector< vector<double> > getPercentages();

	/*cycle length related methods*/
	std::size_t getCycleLength();
	void setCycleLength(std::size_t);

	/*phase related methods*/
	std::size_t & CurrPhaseID();
	const std::vector<sim_mob::Phase> & getPhases() const;
	void addPhase(sim_mob::Phase);
	std::size_t nofPhases();
	std::size_t computeCurrPhase(double currCycleTimer);
	const sim_mob::Phase & CurrPhase() const;

	/*offset related methods*/
	std::size_t getOffset();
	void setOffset(std::size_t);

	/*main update mehod*/
	void Update(std::vector<double> DS);

	/*General Methods*/
	void calMaxProDS(std::vector<double>  &maxproDS,std::vector<double> DS);
	std::size_t Vote(std::vector<double> maxproDS);
	int getPlanId_w_MaxVote();
	double fmin_ID(std::vector<double> maxproDS);
	std::size_t getMaxVote();

	/*friends*/
	friend class Signal;
};
}
