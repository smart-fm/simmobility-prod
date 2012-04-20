/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.cpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */

#include "Signal.hpp"
#include <math.h>
#include "geospatial/Lane.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/StreetDirectory.hpp"
#include "util/OutputUtil.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::map;
using std::vector;
using std::string;

typedef sim_mob::Entity::UpdateStatus UpdateStatus;

namespace sim_mob
{

std::vector<Signal*> Signal::all_signals_;

//Private namespace
namespace {
//parameters for calculating next cycle length
const double DSmax = 0.9, DSmed = 0.5, DSmin = 0.3;
const double CLmax = 140, CLmed = 100, CLmin = 60;

//parameters for calculating next Offset
const double CL_low = 70, CL_up = 120;
const double Off_low = 5, Off_up = 26;

const double fixedCL = 60;
}
/*
 * The folowing are Helper arrays for setting TC_* data (in an anonymous namespace).
 * Vahid:
 * what they are:
 * a-they are color indications(Red=1,Amber=2,Green=3)
 *   flashing green is missing.Actually many things are missing but it's ok, we are "simulating" SCATS which is almost a "closed" specification.
 * b-each template(pedestrian and Driver) has 8 rows of data.
 * c-there are 2 rows for each phase.(there are 4 phases, so 4X2=8)
 * d-why 2 rows: one row for when driver light is green and one for when driver light is amber(yellow)
 *   therefore pedestrian also gets 2 identical rows ,for each phase,to be compatible with driver data.
 * e-each row has 4 "set"s of data(one "set" for each approach of the intersection).
 *  what do I mean by "set" of data! : for pedestrian there is only one value in each "set", but driver has 3 values for left,forward,right enclosed like { 3, 3, 1 },...
 *  Note: Had the respected programmer dropped a few lines as I did above, i wouldn't be itching my brain for two days to
 *  figure out what they are :))
 *  I still don't know what TC stands for!(same unclarity goes with case 0, case 10.... what do you mean by case?)
 *
 */

namespace {


//Driver template
const int TC_for_DriverTemplate[][4][3] = {
		{ { 3, 3, 1 }, { 1, 1, 1 }, { 3, 3, 1 }, { 1, 1, 1 } }, //Case  0
		{ { 2, 2, 1 }, { 1, 1, 1 }, { 2, 2, 1 }, { 1, 1, 1 } }, //Case 10
		{ { 1, 1, 3 }, { 1, 1, 1 }, { 1, 1, 3 }, { 1, 1, 1 } }, //Case  1
		{ { 1, 1, 2 }, { 1, 1, 1 }, { 1, 1, 2 }, { 1, 1, 1 } }, //Case 11
		{ { 1, 1, 1 }, { 3, 3, 1 }, { 1, 1, 1 }, { 3, 3, 1 } }, //Case  2
		{ { 1, 1, 1 }, { 2, 2, 1 }, { 1, 1, 1 }, { 2, 2, 1 } }, //Case 12
		{ { 1, 1, 1 }, { 1, 1, 3 }, { 1, 1, 1 }, { 1, 1, 3 } }, //Case  3
		{ { 1, 1, 1 }, { 1, 1, 2 }, { 1, 1, 1 }, { 1, 1, 2 } }, //Case 13
		};

//Pedestrian template
const int TC_for_PedestrianTemplate[][4] = {
		{ 1, 3, 1, 3 }, //Case  0
		{ 1, 3, 1, 3 }, //Case 10
		{ 1, 1, 1, 1 }, //Case  1
		{ 1, 1, 1, 1 }, //Case 11
		{ 3, 1, 3, 1 }, //Case  2
		{ 3, 1, 3, 1 }, //Case 12
		{ 1, 1, 1, 1 }, //Case  3
		{ 1, 1, 1, 1 }, //Case 13
		};

} //End anon namespace

const double Signal::fixedSplitPlan[] = { 0.30, 0.30, 0.20, 0.20 };
///////////////////// Implementation ///////////////////////
Signal const &
Signal::signalAt(Node const & node, const MutexStrategy& mtxStrat) {
	Signal const * signal = StreetDirectory::instance().signalAt(node);
	if (signal)
		return *signal;

	Signal * sig = new Signal(node, mtxStrat);
	all_signals_.push_back(sig);
	StreetDirectory::instance().registerSignal(*sig);
	return *sig;
}

//todo
void Signal::addSignalSite(centimeter_t /* xpos */, centimeter_t /* ypos */,
		std::string const & /* typeCode */, double /* bearing */) {
	// Not implemented yet.
}

Signal::Signal(Node const & node, const MutexStrategy& mtxStrat, int id)
  : Agent(mtxStrat, id)
  , node_(node)
  , buffered_TC(mtxStrat, SignalStatus())
  , loopDetector_(*this, mtxStrat)
{
	ConfigParams& config = ConfigParams::GetInstance();
	signalAlgorithm = config.signalAlgorithm;
    initializeSignal();
//    setupIndexMaps();  I guess this function is Not needed any more
}

void Signal::initializeSignal() {
	Density[0]=0,Density[1]=0,Density[2]=0,Density[3]=0;
	setCL(60, 60, 60);//default initial cycle length for SCATS
	setRL(60, 60);//default initial RL for SCATS
	startSplitPlan();
	currPhase = 0;
	phaseCounter = 0;
	currOffset = 0;
	nextOffset = 0;
	updateTrafficLights();

}
//find the max projected DS in each SplitPlan
double Signal::fmax(std::vector<double> proDS) {
	double max = proDS[0];
	for (int i = 1; i < proDS.size(); i++) {
		if (proDS[i] > max) {
			max = proDS[i];
		}
		//else{}
	}
	return max;
}

//find the minimum among the max projected DS
int Signal::fmin_ID(std::vector<double> maxproDS) {
	int min = 0;
	for (int i = 1; i < maxproDS.size(); i++) {
		if (maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;
}

////determine next SplitPlan according to the votes in last certain number of cycles
//int Signal::calvote(std::vector<unsigned int >vote) {
//
//	for(int i=0;i< vote.size() ; i++)
//		assert(vote[i] < 6);
//
//	std::vector<unsigned int >vote_num;
//	int vote_num[6] = { 0, 0, 0, 0, 0, 0 };
//	int ID = 1;
//
//	vote_num[vote1]++;
//	vote_num[vote2]++;
//	vote_num[vote3]++;
//	vote_num[vote4]++;
//	vote_num[vote5]++;
//	for (int i = 1; i <= 5; i++) {
//		if (vote_num[i] > vote_num[ID]) {
//			ID = i;
//		}
//		//else{}
//	}
//	return ID;
//}

//calculate the projected DS and max Projected DS for each split plan
void Signal::calProDS_MaxProDS(std::vector<double> &proDS,std::vector<double>  &maxproDS)
{
	for(int i=0; i < SplitPlan.size(); i++)
	{
		for(int j=0; j < DS.size(); j++)
		{
			proDS[j] = DS[j] * currSplitPlan[j] / SplitPlan[i][j];
		}
		//Also find the Maximum Projected DS for each Split plan(used in the next step)
		std::vector<double>::iterator DS_it;
		double temp;
		DS_it = maxproDS.begin();
		for(temp = *DS_it; DS_it != maxproDS.end(); DS_it++)
		{
			if(temp < *DS_it) temp = *DS_it;
		}
		maxproDS[i] = temp;// maximum projected DS
		//cleanup
		proDS.clear();
		proDS.resize(DS.size(),0);
	}
}

//find te split plan Id which currently has the maximum vote
int Signal::getPlanId_w_MaxVote()
{
	int PlanId_w_MaxVote = -1 , SplitPlanID , max_vote_value = -1, vote_sum = 0;
	for(SplitPlanID = 0 ; SplitPlanID < votes.size(); SplitPlanID++)
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

//4.3 Split Plan Selection (use DS to choose SplitPlan for next cycle)(section 4.3 of the Li Qu's manual)
void Signal::setnextSplitPlan(std::vector<double> DS) {
	std::vector<int> vote(SplitPlan.size(),0);
	std::vector<double> proDS(DS.size(),0);// projected DS
	std::vector<double>  maxproDS(SplitPlan.size(),0);// max projected DS of each SplitPlan
	int i,j;

	//step 1:Calculate the projected DS for each approach (and find the maximum projected DS)
	calProDS_MaxProDS(proDS,maxproDS);
	//Step 2: The split plan with the ' lowest "Maximum Projected DS" ' will get a vote
	vote[fmin_ID(maxproDS)]++;//the corresponding split plan's vote is incremented by one(the rests are zero)
	votes.push_back(vote);
	if(votes.size() > NUMBER_OF_VOTING_CYCLES) votes.erase(0); //removing the oldest vote if necessary(wee keep only NUMBER_OF_VOTING_CYCLES records)
	/*
	 * step 3: The split plan with the highest vote in the last certain number of cycles will win the vote
	 *         no need to eat up your brain, in the following chunk of code I will get the id of the maximum
	 *         value in vote vector and that id will actually be my nextSplitPlanID
	*/
	nextSplitPlanID = getPlanId_w_MaxVote();
	//enjoy the result
	nextSplitPlan = SplitPlan[nextSplitPlanID];
}

void Signal::updatecurrSplitPlan() {
	currSplitPlanID = nextSplitPlanID;
	currSplitPlan = nextSplitPlan;
	//why didn't you just copy the vectors?(as i did above)
//	for (int i = 0; i < 4; i++) {
//		currSplitPlan[i] = nextSplitPlan[i];
//	}
}

//use next cycle length to calculate next Offset
void Signal::setnextOffset(double nextCL) {
//	std::cout<<"nextCL "<<nextCL<<std::endl;
	if (nextCL <= CL_low) {
		nextOffset = Off_low;
	} else if (nextCL > CL_low && nextCL <= CL_up) {
		nextOffset = Off_low + (nextCL - CL_low) * (Off_up - Off_low) / (CL_up - CL_low);
	} else {
		nextOffset = Off_up;
	}
}

void Signal::updateOffset() {
	currOffset = nextOffset;
}

namespace {
std::string mismatchError(char const * const func_name, Signal const & signal, RoadSegment const & road) {
	std::ostringstream stream;
	stream << func_name << ": mismatch in Signal and Lane; Details as follows" << std::endl;
	stream << "    Signal is located at (" << signal.getNode().location << std::endl;
	stream << "    Lane is part of RoadSegment going from " << road.getStart()->location << " to "
			<< road.getEnd()->location << std::endl;
	return stream.str();
}
}
/*I will try to change only the Data structure, not the algorithm-vahid
 * This function will tell you what lights a driver is gonna get when he is at the traffic signal
 * this is done based on the lan->rs->link he is in.
 * He will get three colors for three options of heading directions(left, forward,right)
 */
Signal::VehicleTrafficColors Signal::getDriverLight(Lane const & lane) const {
	RoadSegment const * road = lane.getRoadSegment();
	Link const * link = road->getLink();
	//todo check if this link is listed in the links associated with the currSplitPlan.phases.links
	std::set<sim_mob::Link *>::iterator it=currPhase.links.find(link);
	if(it == currPhase.links.end()) {
		throw std::runtime_error(mismatchError("Signal::getDriverLight(lane)", *this, *road).c_str());
	}
	std::map<sim_mob::Link *, struct VehicleTrafficColors>::iter = currPhase.links_colors.find(link);
	return iter->second;
}

Signal::TrafficColor Signal::getDriverLight(Lane const & fromLane, Lane const & toLane) const {
	RoadSegment const * fromRoad = fromLane.getRoadSegment();
	Link const * fromLink = fromRoad->getLink();
	std::map<Link const *, size_t>::const_iterator iter = links_map_.find(fromLink);
	if (iter == links_map_.end()) {
		throw std::runtime_error(mismatchError("Signal::getDriverLight(fromLane, toLane)", *this, *fromRoad).c_str());
	}
	size_t fromIndex = iter->second;

	RoadSegment const * toRoad = toLane.getRoadSegment();
	Link const * toLink = toRoad->getLink();
	iter = links_map_.find(toLink);
	if (iter == links_map_.end()) {
		throw std::runtime_error(mismatchError("Signal::getDriverLight(fromLane, toLane)", *this, *toRoad).c_str());
	}
	size_t toIndex = iter->second;

	// When links_map was populated in setupIndexMaps(), the links were numbered in anti-clockwise
	// direction.  The following switches are based on this fact.
	VehicleTrafficColors colors = getDriverLight(fromLane);
	if (fromIndex > toIndex) {
		int diff = fromIndex - toIndex;
		switch (diff) {
		case 0:
			return Red; // U-turn is not supported currently.
		case 1:
			return colors.left;
		case 2:
			return colors.forward;
		case 3:
			return colors.right;
		default:
			return Red;
		}
	} else {
		int diff = toIndex - fromIndex;
		switch (diff) {
		case 0:
			return Red; // U-turn is not supported currently.
		case 1:
			return colors.right;
		case 2:
			return colors.forward;
		case 3:
			return colors.left;
		default:
			return Red;
		}
	}

	return Red;
}
