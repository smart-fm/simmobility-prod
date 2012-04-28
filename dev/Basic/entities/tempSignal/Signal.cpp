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

Signal::Signal(Node const & node, const MutexStrategy& mtxStrat, int id)
  : Agent(mtxStrat, id)
  , node_(node)
  , loopDetector_(*this, mtxStrat)
{

	findIncomingLanes();
	ConfigParams& config = ConfigParams::GetInstance();
	signalAlgorithm = config.signalAlgorithm;
	Density.resize(IncomingLanes_.size(), 0);
    initializeSignal();
//    setupIndexMaps();  I guess this function is Not needed any more
}
/* Set Split plan and Initialize its Indicators in the signal class*/
void Signal::setSplitPlan(sim_mob::SplitPlan plan)
{
	plan_ = plan;
}
/* Set the cycle length and Initialize its Indicators in the signal class*/
void Signal::setCycleLength(sim_mob::Cycle cycle)
{
	cycle_ = cycle;
}
void Signal::initializeSignal() {

//	setCL(0, 60, 0);//default initial cycle length for SCATS
//	setRL(60, 60);//default initial RL for SCATS
	startSplitPlan();
	currPhase = 0;
	phaseCounter = 0;
	currOffset = 0;
	nextOffset = 0;
	updateTrafficLights();

}

void Signal::addSignalSite(centimeter_t /* xpos */, centimeter_t /* ypos */,
		std::string const & /* typeCode */, double /* bearing */) {
	// Not implemented yet.
}
/*
 * this class needs to access lanes coming to it, mostly to calculate DS
 * It is not feasible to extract the lanes from every traffic signal every time
 * we need to calculate DS. Rather, we book-keep  the lane information.
 * It is a trade-off between process and memory.
 * In order to save memory, we only keep the record of Lane pointers-vahid
 */
int Signal::findIncomingLanes()
{
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_);
	if(! mNode) retunrn -1;
	const std::set<sim_mob::RoadSegment*>& rs = mNode->getRoadSegments();
	for (std::set<sim_mob::RoadSegment*>::const_iterator it = rs.begin(); it!= rs.end(); it++) {
		if ((*it)->getEnd() != &node_)//consider only the segments that end here
			continue;
		IncomingLanes_.reserve(IncomingLanes_.size() + (*it)->getLanes().size());
		IncomingLanes_.insert(IncomingLanes_.end(), (*it)->getLanes().begin(), (*it)->getLanes().end());
	}
}

//initialize SplitPlan
void Signal::startSplitPlan() {
	//CurrSplitPlan
	currSplitPlanID = plan_.CurrSplitPlanID();//1; -vahid
	currSplitPlan = plan_.CurrSplitPlan();
	nextSplitPlan.assign(plan_.nofPhases(), 0); //Initialize to the number 0, four times.
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

std::vector<double> Signal::getNextSplitPlan() {return nextSplitPlan;}
std::vector<double> Signal::getCurrSplitPlan() {return currSplitPlan;}
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
	std::vector<double>  maxproDS(plan_.getPhases().size(),0);// max projected DS of each SplitPlan
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

/*
 * This function calculates the Degree of Saturation based on the "lanes"(so a part of the effort will be devoted to finding lanes)
 * The return Value of this function is, however,the max DS among all -corresponding- lanes.
 * Previous implementation had a mechanism to filter out unnecessary lanes but since it was based on the default
 * 4-junction intersection scenario only, I had to replace it.
 */
double Signal::computeDS(double total_g)
{
	double maxDS = 0;
	std::set<sim_mob::links_map>::iterator it_LM = plan_.CurrPhase().LinksMap().begin();
	for(;it_LM!= plan_.CurrPhase().LinksMap().end(); it_LM++)
	{
		sim_mob::Link *link = (*it_LM).LinkFrom;
		std::set<sim_mob::RoadSegment*>::iterator it = (*link).uniqueSegments.begin();
		for(; it != (*link).uniqueSegments.end(); it++)
		{
			//discard the segments that don't end here(coz those who don't end here, don't cross the intersection neither)
			if((*it)->getEnd()!=&node_)//sim_mob::Link is bi-directionl so we use RoadSegment's start and end to imply direction
				continue;
		}
		const std::vector<sim_mob::Lane*>& lanes = (*it)->getLanes();
		for(size_t i=0;i<lanes.size();i++)
		{
			const Lane* lane = lanes.at(i);
			if(lane->is_pedestrian_lane())
				continue;
			const LoopDetectorEntity::CountAndTimePair& ctPair = loopDetector_.getCountAndTimePair(*lane);
			double lane_DS = LaneDS(ctPair,total_g);
			if(lane_DS > maxDS) maxDS = lane_DS;
		}
	}
//	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_); is history :)
	return maxDS;
}

/*
 * The actual DS computation formula is here!
 * It calculates the DS on a specific Lane
 */
double Signal::LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g)
{
//	CountAndTimePair would give you T and n of the formula 2 in section 3.2 of the memurandum (page 3)
	size_t vehicleCount = ctPair.vehicleCount;
	unsigned int spaceTime = ctPair.spaceTimeInMilliSeconds;
	double standard_space_time = 1.04*1000;//1.04 seconds
	/*this is formula 2 in section 3.2 of the memurandum (page 3)*/
	double used_g = (vehicleCount==0)?0:total_g - (spaceTime - standard_space_time*vehicleCount);
	return used_g/total_g;//And this is formula 1 in section 3.2 of the memurandum (page 3)
}

//Update Signal Light
void Signal::updateSignal(double DS[]) {
	if (phaseCounter == 0) {
		// 0 is fixed phase, 1 is scats
		if(signalAlgorithm == 1)
		{
			//find the maximum DS
			DS_all = fmax(DS);

			//use DS_all for calculating next cycle length
			cycle_.setnextCL(DS_all);

			//use next cycle length to calculate next Offset
			offset_.setnextOffset(getnextCL());
			offset_.updateOffset();

			cycle_.updateprevCL();
			cycle_.updatecurrCL();
			plan_.findNextPlanIndex();
			plan_.updatecurrSplitPlan();
			loopDetector_.reset();
		}
		else
		{
			nextCL = fixedCL;
			nextSplitPlan.assign(fixedSplitPlan, fixedSplitPlan + 4);
		}

		currPhase = 0;
		phaseCounter += currOffset;
	}

	// 0 is fixed phase, 1 is scats

	int prePhase = currPhase;
	if (phaseCounter < nextCL * nextSplitPlan[0]) {
		if (phaseCounter <= (nextCL * nextSplitPlan[0] - 3))
			currPhase = 0;
		else
			currPhase = 10;
	} else if (phaseCounter < nextCL * (nextSplitPlan[0] + nextSplitPlan[1])) {
		if (phaseCounter <= (nextCL * (nextSplitPlan[0] + nextSplitPlan[1]) - 3))
			currPhase = 1;
		else
			currPhase = 11;
	} else if (phaseCounter < nextCL * (nextSplitPlan[0] + nextSplitPlan[1] + nextSplitPlan[2])) {
		if (phaseCounter <= (nextCL * (nextSplitPlan[0] + nextSplitPlan[1] + nextSplitPlan[2]) - 3))
			currPhase = 2;
		else
			currPhase = 12;
	} else if (phaseCounter <= (nextCL - 3))
		currPhase = 3;
	else
		currPhase = 13;

	phaseCounter++;

	if (phaseCounter == floor(nextCL)) {
		phaseCounter = 0;
	}

	// 0 is fixed phase, 1 is scats
	if(signalAlgorithm == 1)
	{
		if(currPhase%10!=prePhase%10||phaseCounter==0)
		{
			double total_g = (nextCL * nextSplitPlan[prePhase%10])*1000;

			double currPhaseDS = computeDS(total_g);
			//		if(getNode().location.getX()==37250760 && getNode().location.getY()==14355120)
			//			std::cout<<"currDS "<<currPhaseDS<<std::endl;
			DS[prePhase%10] = currPhaseDS;
			loopDetector_.reset();
		}
	}
	updateTrafficLights();
}

double updateCurrCycleTimer(frame_t frameNumber) {

}
UpdateStatus Signal::update(frame_t frameNumber) {
	//todo (= or some range )
	newCycle = updateCurrCycleTimer(frameNumber);

	if(newCycle) DS_all = computeDS();
	if(newCycle) cycle_.Update(currCycleTimer,DS_all);
	plan_.Update(newCycle,currCycleTimer,DS_all);
		offset_.update();

	updateIndicators();//i guess except currCycleTimer which was updated first to serv the other functions.
//	updateSignal(Density);
//	outputToVisualizer(frameNumber);
//	if (ConfigParams::GetInstance().is_run_on_many_computers == false)
//		frame_output(frameNumber);
//
//	return UpdateStatus::Continue;
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
