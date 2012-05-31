/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.cpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */
#include "./Signal.hpp" //just a precaution
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

//std::vector<Signal*> Signal::all_signals_;
Signal::all_signals Signal::all_signals_;
const double Signal::updateInterval = ConfigParams::GetInstance().granSignalsTicks * ConfigParams::GetInstance().baseGranMS / 1000;


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
	{
		std::cout << "Node : " << node.getID() <<" Already has a Signal !" << std::endl;
		return *signal;
	}
	std::cout << "Node : " << node.getID() <<" creating a Signal !" << std::endl;
	Signal * sig = new Signal(node, mtxStrat);
	all_signals_.push_back(sig);
	std::cout << "Signal Created" << std::endl;
	StreetDirectory::instance().registerSignal(*sig);
	return *sig;
}

Signal::Signal(Node const & node, const MutexStrategy& mtxStrat, int id)
  : Agent(mtxStrat, id)
	, loopDetector_(*this, mtxStrat)
	, node_(node)
{
	//some inits
	currSplitPlanID = 0;
	phaseCounter = 0;
	currCycleTimer = 0;
	DS_all = 0;
	currCL = 0;
	currPhaseID = 0;
	isNewCycle = false;
	currOffset = 0;
	TMP_SignalID = node.getID();//the best id for a signal is the node id itself
	findIncomingLanes();//what was it used for? dont remember
	findSignalLinksAndCrossings();
	ConfigParams& config = ConfigParams::GetInstance();
	signalAlgorithm = config.signalAlgorithm;
	Density.resize(IncomingLanes_.size(), 0);//todo wrong ! Density has changed to contain phase DS
    std::cout << "after initializeSignal" << std::endl;
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
	currPhaseID = 0;
	phaseCounter = 0;
	currOffset = 0;
//	updateTrafficLights(); //todo what is that?

}
sim_mob::SplitPlan & Signal::getPlan()
{
	return plan_;
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
void Signal::findIncomingLanes()
{
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_);
	if(! mNode) return ;
	const std::set<sim_mob::RoadSegment*>& rs = mNode->getRoadSegments();
	for (std::set<sim_mob::RoadSegment*>::const_iterator it = rs.begin(); it!= rs.end(); it++) {
		if ((*it)->getEnd() != &node_)//consider only the segments that end here
			continue;
		IncomingLanes_.reserve(IncomingLanes_.size() + (*it)->getLanes().size());
		IncomingLanes_.insert(IncomingLanes_.end(), (*it)->getLanes().begin(), (*it)->getLanes().end());
	}
}


// This functor calculates the angle between a link and a reference link, which have a node
// in common.
struct AngleCalculator {
	// <node> is the node in common and <refLink> is the reference link.  Therefore <node>
	// must be one of the ends of <refLink>.
	AngleCalculator(Node const & node, Link const * refLink) :
		center_(node) {
		assert(refLink->getStart() == &center_ || refLink->getEnd() == &center_);
		// <refAngle_> is the angle that the <refLink> makes with the X-axis.
		refAngle_ = angle(refLink);
	}

	// Calculates the angle between <link> and <refLink>, which have <node> in common.
	// Therefore <node> must be one of the ends of <link>.
	double operator()(Link const * link) const {
		assert(link->getStart() == &center_ || link->getEnd() == &center_);
		return angle(link) - refAngle_;
	}

	Node const & center_;
	double refAngle_;

	// Caculates the angle that <link> with respect to the X-axis.
	double angle(Link const * link) const {
		Point2D point;
		if (link->getStart() == &center_)
			point = link->getEnd()->location;
		else
			point = link->getStart()->location;
		double xDiff = point.getX() - center_.location.getX();
		double yDiff = point.getY() - center_.location.getY();
		return atan2(yDiff, xDiff);
	}
};



// Return the Crossing object, if any, in the specified road segment.  If there are more
// than one Crossing objects, return the one that has the least offset.
Crossing const *
getCrossing(RoadSegment const * road) {
	//Crossing const * result = 0;
	//double offset = std::numeric_limits<double>::max();
	int currOffset = 0;
	for (;;) {
		//Get the next item, if any.
		RoadItemAndOffsetPair res = road->nextObstacle(currOffset, true);
		if (!res.item) {
			break;
		}

		//Check if it's a Crossing.
		if (Crossing const * crossing = dynamic_cast<Crossing const *>(res.item)) {
			//Success
			return crossing;
		}

		//Increment
		currOffset += res.offset;
	}
	std::cout <<"]";

	//Failure.
	return nullptr;
}
/*
 * this class needs to access lanes coming to it, mostly to calculate DS
 * It is not feasible to extract the lanes from every traffic signal every time
 * we need to calculate DS. Rather, we book-keep  the lane information.
 * It is a trade-off between process and memory.
 * In order to save memory, we only keep the record of Lane pointers-vahid
 */
void Signal::findSignalLinksAndCrossings()
{
	LinkAndCrossingByLink & inserter = get<2>(LinkAndCrossings_);//2 means that duplicate links will not be allowed
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_);
	if(! mNode) return ;
	const std::set<sim_mob::RoadSegment*>& roads = mNode->getRoadSegments();
	std::set<RoadSegment*>::const_iterator iter = roads.begin();
	sim_mob::RoadSegment const * road = *iter;
	sim_mob::Crossing const * crossing = getCrossing(road);
	sim_mob::Link const * link = road->getLink();
	inserter.insert(LinkAndCrossing(0,link,crossing,0));
	++iter;

	//Prepare output(perhapse this was the best place to accommodate this unrelated part :)
		std::ostringstream output;
		output << "(\"Signal-location\", 0, " <<  TMP_SignalID << ", {";
		output << "\"node\":\"" << node_.getID() << "\"";

	AngleCalculator angle(node_, link);
	double angleAngle = 0;
	size_t id = 1;
	angleAngle = 0;
	crossing = nullptr;
	link = nullptr;
	for (; iter != roads.end(); ++iter, ++id) {//id=1 coz we have already made an insertion with id=0 above
		road = *iter;
		crossing = getCrossing(road);
		link = road->getLink();
		angleAngle = angle.angle(link);
		inserter.insert(LinkAndCrossing(id,link,crossing,angleAngle));
		//Append to output, again sorry for the inconvenience :)
		char letter = static_cast<char> ('a' + id);
		output << ",\"v" << letter << "\":\"" << link << "\",\"a" << letter << "\":\"" << 180 * (angleAngle/ M_PI) << "\",\"p" << letter << "\":\"" << crossing << "\"";
		crossing = nullptr;
		link = nullptr;
	}
	//Close off and save the string representation.
	output << "})";
	strRepr = output.str();//all the aim of the unrelated part

//	for (std::set<sim_mob::RoadSegment*>::const_iterator it = rs.begin(); it!= rs.end(); it++) {
//		SignalLinks.push_back((*it)->getLink());
////		SignalLinks.reserve(SignalLinks.size() + (*it)->getLink);
////		SignalLinks.insert(SignalLinks.end(), (*it)->getLanes().begin(), (*it)->getLanes().end());
//	}
}
//deprecated
void Signal::findSignalLinks()
{
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_);
	if(! mNode) return ;
	const std::set<sim_mob::RoadSegment*>& rs = mNode->getRoadSegments();
	for (std::set<sim_mob::RoadSegment*>::const_iterator it = rs.begin(); it!= rs.end(); it++) {
		SignalLinks.push_back((*it)->getLink());
//		SignalLinks.reserve(SignalLinks.size() + (*it)->getLink);
//		SignalLinks.insert(SignalLinks.end(), (*it)->getLanes().begin(), (*it)->getLanes().end());
	}
}
//const std::vector<sim_mob::Link const *> & Signal::getSignalLinks() const
//{
//	return SignalLinks;
//}

//initialize SplitPlan
void Signal::startSplitPlan() {
	//CurrSplitPlan
	currSplitPlanID = plan_.CurrSplitPlanID();//todo check what .CurrSplitPlanID() is giving? where is it initializing?
	currSplitPlan = plan_.CurrSplitPlan();//todo check if we really need this
}

//find the minimum among the max projected DS
int Signal::fmin_ID(const std::vector<double> maxproDS) {
	int min = 0;
	for (int i = 1; i < maxproDS.size(); i++) {
		if (maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;
}

//std::vector<double> Signal::getNextSplitPlan() {return nextSplitPlan;}
//std::vector<double> Signal::getCurrSplitPlan() {return currSplitPlan;}





///*
// * This function calculates the Degree of Saturation of the entire intersection
// * based on the "lanes"(so a part of the effort will be devoted to finding lanes)
// * The return Value of this function is the max DS among all -corresponding- lanes.
// * Previous implementation had a mechanism to filter out unnecessary lanes but since it was based on the default
// * 4-junction intersection scenario only, I had to replace it.
// */
//double Signal::computeMaxDS(double total_g)
//{
//	double maxDS = 0;
//	std::set<sim_mob::links_map>::iterator it_LM = plan_.CurrPhase().LinksMap().begin();
//	for(;it_LM!= plan_.CurrPhase().LinksMap().end(); it_LM++)
//	{
//		sim_mob::Link *link = (*it_LM).LinkFrom;
//		std::set<sim_mob::RoadSegment*>::iterator it = (*link).uniqueSegments.begin();
//		for(; it != (*link).uniqueSegments.end(); it++)
//		{
//			//discard the segments that don't end here(coz those who don't end here, don't cross the intersection neither)
//			if((*it)->getEnd()!=&node_)//sim_mob::Link is bi-directionl so we use RoadSegment's start and end to imply direction
//				continue;
//		}
//		const std::vector<sim_mob::Lane*>& lanes = (*it)->getLanes();
//		for(std::size_t i=0;i<lanes.size();i++)
//		{
//			const Lane* lane = lanes.at(i);
//			if(lane->is_pedestrian_lane())
//				continue;
//			const LoopDetectorEntity::CountAndTimePair& ctPair = loopDetector_.getCountAndTimePair(*lane);
//			double lane_DS = LaneDS(ctPair,total_g);
//			if(lane_DS > maxDS) maxDS = lane_DS;
//		}
//	}
////	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&node_); is history :)
//	return maxDS;
//}
/*
 * This function calculates the Degree of Saturation based on the "lanes" of phases.
 * There are two types of outputs produced by this funtion:
 * 1-maximum DS of each phase, which will be stored in Signal::Density member variable
 * 2-the maximum Density observed at this junction which will be returned by is function.
 * The DS of phases is stored in Density vector. Note that DS of each phase is the max DS observed in its lanes.
 * The return Value of this function is the max DS among all -corresponding- lanes.
 * why does a phase DS represented by max DS of all lanes in that phase? coz I say so! jungle rule!
 * Previous implementation had a mechanism to filter out unnecessary lanes but since it was
 * based on the default 4-junction intersection scenario only. I had to replace it.
 * For code readers,the for loops are nested as:
 * for(phases)
 * 		for(Links)
 * 				for(Road Segments)
 * 							for(Lanes)
 * 						 		 getlaneDS()
 * 								 getMaxDS
 * 								 getMaxPhaseDS
 */
double Signal::computeDS() {
	double lane_DS, maxPhaseDS, maxDS;
	sim_mob::SplitPlan::phases_iterator p_it = plan_.phases_.begin();
	for(int i = 0 ;p_it != plan_.phases_.end(); p_it++)//Loop1===>phase
	{
		maxPhaseDS = 0;
		const std::string name = (*p_it).name;
		double total_g = (*p_it).computeTotalG();//todo: I guess we can avoid calling this function EVERY time by adding an extra container at split plan level.(mapped to choiceSet container)
		sim_mob::Phase::links_map_iterator link_it = (*p_it).LinkFrom_begin();
		for (; link_it != (*p_it).LinkFrom_end(); link_it++) {//Loop2===>link
			std::set<sim_mob::RoadSegment*> segments = (*link_it).first->getUniqueSegments();//optimization: use either fwd or bed segments
			std::set<sim_mob::RoadSegment*>::iterator seg_it =	segments.begin();
			for (; seg_it != segments.end(); seg_it++) {//Loop3===>road segment
				//discard the segments that don't end here(coz those who don't end here, don't cross the intersection neither)
				//sim_mob::Link is bi-directionl so we use RoadSegment's start and end to imply direction
				if ((*seg_it)->getEnd() != &node_)	continue;
				const std::vector<sim_mob::Lane*> lanes = (*seg_it)->getLanes();
				for (std::size_t i = 0; i < lanes.size(); i++) {//Loop4===>lane
					const Lane* lane = lanes.at(i);
					if (lane->is_pedestrian_lane())	continue;
					const LoopDetectorEntity::CountAndTimePair& ctPair = loopDetector_.getCountAndTimePair(*lane);
					lane_DS = LaneDS(ctPair, total_g);
					if (lane_DS > maxPhaseDS)	maxDS = lane_DS;
					if (lane_DS > maxDS)		maxDS = lane_DS;
				}
			}

		}
		Density[i++] = maxPhaseDS;
	}
	DS_all = maxDS;
	return (DS_all);
}

/*
 * The actual DS computation formula is here!
 * It calculates the DS on a specific Lane
 * at the moment total_g amounts to total_g at each phase,
 * However this function doesn't care total_g comes from which scop(phase level, cycle level....)
 */
double Signal::LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g)
{
//	CountAndTimePair would give you T and n of the formula 2 in section 3.2 of the memurandum (page 3)
	std::size_t vehicleCount = ctPair.vehicleCount;
	unsigned int spaceTime = ctPair.spaceTimeInMilliSeconds;
	double standard_space_time = 1.04*1000;//1.04 seconds
	/*this is formula 2 in section 3.2 of the memurandum (page 3)*/
	double used_g = (vehicleCount==0)?0:total_g - (spaceTime - standard_space_time*vehicleCount);
	return used_g/total_g;//And this is formula 1 in section 3.2 of the memurandum (page 3)
}

////Update Signal Light
//void Signal::updateSignal(double DS[]) {
//	if (phaseCounter == 0) {
//		// 0 is fixed phase, 1 is scats
//		if(signalAlgorithm == 1)
//		{
//			//find the maximum DS
//			DS_all = fmax(DS);
//
//			//use DS_all for calculating next cycle length
//			cycle_.setnextCL(DS_all);
//
//			//use next cycle length to calculate next Offset
//			offset_.setnextOffset(getnextCL());
//			offset_.updateOffset();
//
//			cycle_.updateprevCL();
//			cycle_.updatecurrCL();
//			plan_.findNextPlanIndex();
//			plan_.updatecurrSplitPlan();
//			loopDetector_.reset();
//		}
//		else
//		{
//			nextCL = fixedCL;
//			nextSplitPlan.assign(fixedSplitPlan, fixedSplitPlan + 4);
//		}
//
//		currPhase = 0;
//		phaseCounter += currOffset;
//	}
//
//	// 0 is fixed phase, 1 is scats
//
//	int prePhase = currPhase;
//	if (phaseCounter < nextCL * nextSplitPlan[0]) {
//		if (phaseCounter <= (nextCL * nextSplitPlan[0] - 3))
//			currPhase = 0;
//		else
//			currPhase = 10;
//	} else if (phaseCounter < nextCL * (nextSplitPlan[0] + nextSplitPlan[1])) {
//		if (phaseCounter <= (nextCL * (nextSplitPlan[0] + nextSplitPlan[1]) - 3))
//			currPhase = 1;
//		else
//			currPhase = 11;
//	} else if (phaseCounter < nextCL * (nextSplitPlan[0] + nextSplitPlan[1] + nextSplitPlan[2])) {
//		if (phaseCounter <= (nextCL * (nextSplitPlan[0] + nextSplitPlan[1] + nextSplitPlan[2]) - 3))
//			currPhase = 2;
//		else
//			currPhase = 12;
//	} else if (phaseCounter <= (nextCL - 3))
//		currPhase = 3;
//	else
//		currPhase = 13;
//
//	phaseCounter++;
//
//	if (phaseCounter == floor(nextCL)) {
//		phaseCounter = 0;
//	}
//
//	// 0 is fixed phase, 1 is scats
//	if(signalAlgorithm == 1)
//	{
//		if(currPhase%10!=prePhase%10||phaseCounter==0)
//		{
//			double total_g = (nextCL * nextSplitPlan[prePhase%10])*1000;
//
//			double currPhaseDS = computeDS(total_g);
//			//		if(getNode().location.getX()==37250760 && getNode().location.getY()==14355120)
//			//			std::cout<<"currDS "<<currPhaseDS<<std::endl;
//			DS[prePhase%10] = currPhaseDS;
//			loopDetector_.reset();
//		}
//	}
//	updateTrafficLights();
//}
//This is a part of signal::update function that is executed only if a new cycle has reached
void Signal::newCycleUpdate()
{
	//	4-Compute DS for cycle length, split plan and offset selection
		DS_all = computeDS();
	//	5-update cycle length
		cycle_.Update(DS_all);

	//	6-update split plan
		plan_.Update(Density);
	//	7-update offset
		offset_.update(cycle_.getnextCL());

		updateIndicators();//i guess except currCycleTimer which was updated first to serv the other functions.
	//	updateSignal(Density);
		loopDetector_.reset();
}

bool Signal::updateCurrCycleTimer(frame_t frameNumber) {
	bool is_NewCycle = false;
	if(currCycleTimer + updateInterval > plan_.getCycleLength()) is_NewCycle = true;
	//even if it is a new cycle(and a ew cycle length, the currCycleTimer will hold the amount of time system has proceeded to the new cycle
	currCycleTimer =fmod((currCycleTimer + updateInterval) , plan_.getCycleLength());
	return is_NewCycle;
}
/*
 * 1- update current cycle timer
 * 2- update current phase color
 * 3- update current phase
 * if current cycle timer indicates end of cycle:
 * 4-compute DS
 * 5-update cycle length
 * 6-update split plan
 * 7-update offset
 * end of if
 * 8-reset the loop detector to make it ready for the next cycle
 * 8-start
 */
UpdateStatus Signal::update(frame_t frameNumber) {
	//todo (= or some range )
//	1- update current cycle timer( Signal::currCycleTimer)
	isNewCycle = updateCurrCycleTimer(frameNumber);
//	2- update current phase color
//	std::cout << "in Signal(" << this->TMP_SignalID << ") Updating  plan_.phases_[" << currPhaseID << "]\n"; getchar();
	plan_.phases_[currPhaseID].update(currCycleTimer);
//	std::cout << "in Signal::update-plan_.phases_[currPhaseID].update done\n"; getchar();
//	3-Update Current Phase
	currPhaseID = plan_.computeCurrPhase(currCycleTimer);

	if(isNewCycle)	newCycleUpdate();//major update!
//	outputToVisualizer(frameNumber);
//	if (ConfigParams::GetInstance().is_run_on_many_computers == false)
//		frame_output(frameNumber);
//
	return UpdateStatus::Continue;
}
void Signal::updateIndicators()
{
	currCL = cycle_.getcurrCL();
	currPhaseID = plan_.CurrPhaseID();
	currOffset = offset_.getcurrOffset();
	currSplitPlanID = plan_.CurrSplitPlanID();
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
//TrafficColor Signal::getDriverLight(Lane const & lane) const {
//	RoadSegment const * road = lane.getRoadSegment();
//	Link const * link = road->getLink();
//	//todo check if this link is listed in the links associated with the currSplitPlan.phases.links
//	std::set<sim_mob::Link *>::iterator it=currPhase.links.find(link);
//	if(it == currPhase.links.end()) {
//		throw std::runtime_error(mismatchError("Signal::getDriverLight(lane)", *this, *road).c_str());
//	}
//	std::map<sim_mob::Link *, struct VehicleTrafficColors>::iter = currPhase.links_colors.find(link);
//	return iter->second;
//}


TrafficColor Signal::getDriverLight(Lane const & fromLane, Lane const & toLane)const  {
	RoadSegment const * fromRoad = fromLane.getRoadSegment();
	Link * const fromLink = fromRoad->getLink();

	RoadSegment const * toRoad = toLane.getRoadSegment();
	Link const * toLink = toRoad->getLink();

	const sim_mob::Phase &currPhase = plan_.CurrPhase();
	sim_mob::Phase::links_map_equal_range range = currPhase.getLinkTos(fromLink);
	sim_mob::Phase::links_map_const_iterator iter;
	for(iter = range.first; iter != range.second ; iter++ )
	{
		if((*iter).second.LinkTo == toLink)
			break;
	}

	//if the link is not listed in the current phase, just return red(actually it should throw an error)
	if(iter == range.second)
		return sim_mob::Red;
	//		throw std::runtime_error(mismatchError("Signal::getDriverLight(fromLane, toLane)", *this, *fromRoad).c_str());
	else
		return (*iter).second.currColor;
}

TrafficColor Signal::getPedestrianLight(Crossing const & crossing) const
{


}
//todo talk to xuyan or seth on who should decide the format
void Signal::outputToVisualizer(frame_t frameNumber) {
	std::stringstream logout;
	logout << "(\"Signal\"," << frameNumber << "," << this << ",{\"";
//	std::vector<sim_mob::Phase>::const_iterator ph_iter;
	for(SplitPlan::phases_iterator ph_iter = plan_.phases_.begin(), it_end(plan_.phases_.end()); ph_iter != it_end ; ph_iter++)//upper bound
	{

	}

//#ifndef SIMMOB_DISABLE_OUTPUT
//	std::stringstream logout;
//	logout << "(\"Signal\"," << frameNumber << "," << this << ",{\"va\":\"";
//	for (int i = 0; i < 3; i++) {
//		logout << TC_for_Driver[0][i];
//		if (i == 2) {
//			logout << "\",";
//		} else {
//			logout << ",";
//		}
//	}
//	logout << "\"vb\":\"";
//	for (int i = 0; i < 3; i++) {
//		logout << TC_for_Driver[1][i];
//		if (i == 2) {
//			logout << "\",";
//		} else {
//			logout << ",";
//		}
//	}
//	logout << "\"vc\":\"";
//	for (int i = 0; i < 3; i++) {
//		logout << TC_for_Driver[2][i];
//		if (i == 2) {
//			logout << "\",";
//		} else {
//			logout << ",";
//		}
//	}
//	logout << "\"vd\":\"";
//	for (int i = 0; i < 3; i++) {
//		logout << TC_for_Driver[3][i];
//		if (i == 2) {
//			logout << "\",";
//		} else {
//			logout << ",";
//		}
//	}
//
//	logout << "\"pa\":\"" << TC_for_Pedestrian[0] << "\",";
//	logout << "\"pb\":\"" << TC_for_Pedestrian[1] << "\",";
//	logout << "\"pc\":\"" << TC_for_Pedestrian[2] << "\",";
//	logout << "\"pd\":\"" << TC_for_Pedestrian[3] << "\"})" << std::endl;
//	LogOut(logout.str());
//#endif
}
}//namespace
