//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Signal.cpp
 *
 *  Created on: 2011-7-18
 *      Author: Vahid Saber
 */

#include "Signal.hpp"

#include "GenConfig.h"

#include <cmath>
#include "geospatial/Lane.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/conflux/Conflux.hpp"
#include "logging/Log.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::map;
using std::vector;
using std::string;
using std::set;

typedef sim_mob::Entity::UpdateStatus UpdateStatus;

sim_mob::Signal::All_Signals sim_mob::Signal::all_signals_;
using namespace sim_mob;

sim_mob::Signal::Signal(Node const & node, const MutexStrategy& mtxStrat, int id, signalType)
: Agent(mtxStrat, id), node_(node){

}
signalType sim_mob::Signal::getSignalType() const {
	return signalType_;
}

void sim_mob::Signal::setSignalType(signalType sigType)
{
	signalType_ = sigType;
}

void sim_mob::Signal::setLinkAndCrossing(LinkAndCrossingC & LinkAndCrossings) {
	LinkAndCrossings_ = LinkAndCrossings;
}

LinkAndCrossingC const& sim_mob::Signal::getLinkAndCrossing()const {
	return LinkAndCrossings_;
}

LinkAndCrossingC & sim_mob::Signal::getLinkAndCrossing() {
	return LinkAndCrossings_;
}

TrafficColor sim_mob::Signal::getDriverLight(Lane const & fromLane, Lane const & toLane) const {
	throw std::runtime_error("getDriverLight Not implemented");
}


TrafficColor sim_mob::Signal::getPedestrianLight (Crossing const & crossing)const {
	throw std::runtime_error("getPedestrianLight Not implemented");
}


std::string sim_mob::Signal::toString() const{
	throw std::runtime_error("toString Not implemented");
}
Node  const & sim_mob::Signal::getNode() const {
	return node_;
}
void sim_mob::Signal::outputTrafficLights(timeslice now,std::string newLine)const{

}
unsigned int sim_mob::Signal::getSignalId() const{
	return -1;
}

bool sim_mob::Signal::isNonspatial() {
	return true;
}

void sim_mob::Signal::createStringRepresentation(std::string str){
};

sim_mob::Signal::~Signal(){

}

void sim_mob::Signal::load(const std::map<std::string, std::string>& value) {

}

sim_mob::Signal::phases &sim_mob::Signal::getPhases(){
	return phases_;
}

const sim_mob::Signal::phases &sim_mob::Signal::getPhases() const{
	return phases_;
}
void sim_mob::Signal::addPhase(sim_mob::Phase &phase)
{
	phases_.push_back(phase);
}

bool sim_mob::Signal::frame_init(timeslice now){
	return false;
}


void sim_mob::Signal::frame_output(timeslice now){

}


sim_mob::Entity::UpdateStatus sim_mob::Signal::frame_tick(timeslice now){
	return UpdateStatus::Continue;
}


Signal_SCATS const &
sim_mob::Signal_SCATS::signalAt(Node const & node, const MutexStrategy& mtxStrat, bool *isNew ) {
	if (isNew) { *isNew = false; }
	Signal_SCATS const * signal = dynamic_cast<Signal_SCATS const *>(StreetDirectory::instance().signalAt(node));
	if (signal)
	{
		return *signal;
	}

	Signal_SCATS * sig = new Signal_SCATS(node, mtxStrat);
	all_signals_.push_back(sig);
	if (isNew) { *isNew = true; }
	StreetDirectory::instance().registerSignal(*sig);
	return *sig;
}
std::string sim_mob::Signal_SCATS::toString() const { return strRepr; }
unsigned int sim_mob::Signal_SCATS::getSignalId()   {return signalID;}
unsigned int sim_mob::Signal_SCATS::getSignalId() const  {return signalID;}
bool sim_mob::Signal_SCATS::isIntersection() { return isIntersection_;}

void sim_mob::Signal_SCATS::createStringRepresentation(std::string newLine)
{
	std::ostringstream output;
			output << "{" << newLine << "\"TrafficSignal\":" << "{" << newLine;
			output << "\"hex_id\":\""<< this << "\"," << newLine;
			output << "\"frame\": " << -1 << "," << newLine; //this is added to indicate that
			output << "\"simmob_id\":\"" <<  signalID << "\"," << newLine;
			output << "\"node\": \"" << &getNode() << "\"," << newLine;
//			output << splitPlan.createStringRepresentation(newLine); 2nd time
			output << "\"phases\":" << newLine << "[";
			for(int i = 0; i < getPhases().size(); i++)
			{
				output << ( getPhases()[i]).createStringRepresentation(newLine);
				if((i + 1) < getPhases().size()) output << ",";
			}
//				while(it !=getPhases().end())
//				{
//				}
				output << newLine << "]";
			output  << newLine << "}"  << newLine << "}";
			strRepr = output.str();//all the aim of the unrelated part
}

void
sim_mob::Signal_SCATS::printColors(double currCycleTimer)
{
	phases_iterator it = getPhases().begin();
	while(it !=getPhases().end())
	{
		(*it).printPhaseColors(currCycleTimer);
		it++;
	}
}

/*Signal Constructor*/
sim_mob::Signal_SCATS::Signal_SCATS(Node const & node, const MutexStrategy& mtxStrat, int id, signalType type_)
  :  Signal(node,mtxStrat,id), loopDetector_(nullptr)
	/*, node_(node)*/
{
	setSignalType(type_);
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&getNode());
	if(! mNode) isIntersection_ = false ;
	else isIntersection_ = true;
	//some inits
	currCycleTimer = 0;
	currPhaseID = 0;
	isNewCycle = false;
	currOffset = 0;
	//the best id for a signal is the node id itself
	signalID = node.getID();

//	findSignalLinksAndCrossings(); todo:eoved temporarily

	//for future use when user needs to switch between fixed and adaptive control
	//NOTE: This wasn't being used, so I'm hard-coding it. ~Seth
	//signalTimingMode = ConfigParams::GetInstance().signalTimingMode();
	//signalTimingMode = 1;

//	findIncomingLanes();//what was it used for? only Density?
	//it would be better to declare it as static const
	updateInterval = sim_mob::ConfigManager::GetInstance().FullConfig().granSignalsTicks * sim_mob::ConfigManager::GetInstance().FullConfig().baseGranMS() / 1000;
	currCycleTimer = 0;

	//TODO: Why all the ifdefs? Why does this depend on whether we're loading from XML or not? ~Seth
#ifndef SIMMOB_XML_WRITER
	if (ConfigManager::GetInstance().FullConfig().networkSource()==SystemParams::NETSRC_DATABASE) {
		findSignalLinksAndCrossings();
	}
#else
	findSignalLinksAndCrossings();
#endif
}

// Return the Crossing object, if any, in the specified road segment.  If there are more
// than one Crossing objects, return the one that has the least offset.
Crossing const *
sim_mob::Signal_SCATS::getCrossing(RoadSegment const * road) {
	//Crossing const * result = 0;
	//double offset = std::numeric_limits<double>::max();
	int currOffset = 0;
	int minus = 1;
	bool isFwd;
	//find the crossings which belong to this signal not the signal located at the other end of the segment(if any)
	int searchLength = road->length / 2;
	if(road->getStart() == &(this->getNode()))
	{
		currOffset = 0;
		minus = 1;//increment offset
		isFwd = true;
	}
	else
	{
		currOffset = road->length;
		isFwd = false;
		minus = -1;//decrement offset
	}
	for (;;) {
		//Get the next item, if any.
		RoadItemAndOffsetPair res = road->nextObstacle(currOffset, isFwd);
		//break if you didn't find anything
		if (!res.item) {
			break;
		}
		//discard the findings beyond the half length of the segment:

		//break if you started from BEGINNING of the segment and you are half way through the segment length without finding anything
		if(isFwd == true)//starting from the beginning of the segment
			if(currOffset > searchLength)
				break;
		//break if you started from END of the segment and you are half way through the segment length without finding anything
		if(isFwd == false)//starting from end of the segment
			if(currOffset < searchLength)
				break;

		//Check if it's a Crossing.
		if (Crossing const * crossing = dynamic_cast<Crossing const *>(res.item)) {
			//Success
			return crossing;
		}
		else
		{
			if(res.offset == 0)
			{
				res.offset = 1;
			}
		}

		//Increment OR Decrement
		currOffset += (minus) * (res.offset);
	}

	//Failure.
	return nullptr;
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


/*
 * this class needs to access lanes coming to it, mostly to calculate DS
 * It is not feasible to extract the lanes from every traffic signal every time
 * we need to calculate DS. Rather, we book-keep  the lane information.
 * It is a trade-off between process and memory.
 * In order to save memory, we only keep the record of Lane pointers-vahid
 */
void sim_mob::Signal_SCATS::findSignalLinksAndCrossings() {
	LinkAndCrossingC & inserter = getLinkAndCrossing();
	const MultiNode* mNode = dynamic_cast<const MultiNode*>(&getNode());
	if (!mNode){
		return;
	}
	const std::set<sim_mob::RoadSegment*>& roads = mNode->getRoadSegments();
	std::set<RoadSegment*>::const_iterator iter = roads.begin();
	sim_mob::RoadSegment const * road = *iter;
	sim_mob::Crossing const * crossing = getCrossing(road);
	sim_mob::Link const * link = road->getLink();
	inserter.insert(LinkAndCrossing(0, link, crossing, 0));
	++iter;


	AngleCalculator angle(getNode(), link);
	double angleAngle = 0;
	size_t id = 1;
	angleAngle = 0;
	crossing = nullptr;
	link = nullptr;
	for (; iter != roads.end(); ++iter, ++id) { //id=1 coz we have already made an insertion with id=0 above
		road = *iter;
		crossing = getCrossing(road);
		link = road->getLink();
		angleAngle = angle.angle(link);

		inserter.insert(LinkAndCrossing(id, link, crossing, angleAngle));
		crossing = nullptr;
		link = nullptr;
	}
}

//find the minimum among the max projected DS
int sim_mob::Signal_SCATS::fmin_ID(const std::vector<double> maxproDS) {
	int min = 0;
	for (int i = 1; i < maxproDS.size(); i++) {
		if (maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;
}

//This function will calculate the DS at the end of each phase considering only the max DS of lane in the LinkFrom(s)
//LinkFrom(s) are the links from which vehicles enter the intersection during the corresponding phase
double sim_mob::Signal_SCATS::computePhaseDS(int phaseId) {
	double lane_DS = 0, maxPhaseDS = 0, maxDS = 0;
	sim_mob::Phase p_it = getPhases()[phaseId];

	double total_g = p_it.computeTotalG(); //todo: I guess we can avoid calling this function EVERY time by adding an extra container at split plan level.(mapped to choiceSet container)
	sim_mob::Phase::links_map_iterator linkIterator = (p_it).LinkFrom_begin();
	for (; linkIterator != (p_it).LinkFrom_end(); linkIterator++) { //Loop2===>link
		std::set<sim_mob::RoadSegment*> segments =
				(*linkIterator).first->getUniqueSegments(); //optimization: use either fwd or bed segments
		std::set<sim_mob::RoadSegment*>::iterator seg_it = segments.begin();
		for (; seg_it != segments.end(); seg_it++) { //Loop3===>road segment
			//discard the segments that don't end here(coz those who don't end here, don't cross the intersection neither)
			//sim_mob::Link is bi-directionl so we use RoadSegment's start and end to imply direction
			if ((*seg_it)->getEnd() != &getNode())
				continue;
			const std::vector<sim_mob::Lane*> lanes = (*seg_it)->getLanes();
			for (std::size_t i = 0; i < lanes.size(); i++) { //Loop4===>lane
				const Lane* lane = nullptr;
				lane = lanes.at(i);
				if (lane->is_pedestrian_lane())
					continue;
				const Sensor::CountAndTimePair& ctPair =
						loopDetector_->getCountAndTimePair(*lane);
				lane_DS = LaneDS(ctPair, total_g);
				if (lane_DS > maxPhaseDS)
					maxPhaseDS = lane_DS;
			}
		}
	}

	Phase_Density[phaseId] = maxPhaseDS;
	loopDetector_->reset();
	return Phase_Density[phaseId];
}

/**
 * The actual DS computation formula is here!
 * It calculates the DS on a specific Lane
 * at the moment total_g amounts to total_g at each phase,
 * However this function doesn't care total_g comes from which scop(phase level, cycle level....)
 */
double sim_mob::Signal_SCATS::LaneDS(const Sensor::CountAndTimePair& ctPair,double total_g)
{
//	CountAndTimePair would give you T and n of the formula 2 in section 3.2 of the memurandum (page 3)
	std::size_t vehicleCount = ctPair.vehicleCount;
	unsigned int spaceTime = ctPair.spaceTimeInMilliSeconds;
	double standard_space_time = 1.04*1000;//1.04 seconds
	/*this is formula 2 in section 3.2 of the memurandum (page 3)*/
	double used_g = (vehicleCount==0)?0:total_g - (spaceTime - standard_space_time*vehicleCount);
	return used_g/total_g;//And this is formula 1 in section 3.2 of the memurandum (page 3)
}
void sim_mob::Signal_SCATS::cycle_reset()
{
	loopDetector_->reset();//extra
	isNewCycle = false;
	for(int i = 0; i < Phase_Density.size(); Phase_Density[i++] = 0);
}

//This is a part of Signal_SCATS::update function that is executed only if a new cycle has reached
void sim_mob::Signal_SCATS::newCycleUpdate()
{
	//	6-update split plan
		splitPlan.Update(Phase_Density);
	//	7-update offset
//		offset_.update(cycle_.getnextCL());
		cycle_reset();
		loopDetector_->reset();//extra?
		isNewCycle = false;
}

bool sim_mob::Signal_SCATS::updateCurrCycleTimer() {
	bool is_NewCycle = false;
	if((currCycleTimer + updateInterval) >= splitPlan.getCycleLength())
		{
			is_NewCycle = true;
		}
	//even if it is a new cycle(and a ew cycle length, the currCycleTimer will hold the amount of time system has proceeded to the new cycle
	currCycleTimer =  std::fmod((currCycleTimer + updateInterval) , splitPlan.getCycleLength());
	return is_NewCycle;
}

//Output To Visualizer
void sim_mob::Signal_SCATS::frame_output(timeslice now)
{
	LogOut(buffOut.str());
	buffOut.str("");
}



void sim_mob::Signal_SCATS::buffer_output(timeslice now, std::string newLine)
{
	//Reset again, just in case:
	buffOut.str("");
	std::stringstream output;
	output << newLine << "{" << newLine << "\"TrafficSignalUpdate\":" << newLine <<"{" << newLine ;
	output << "\"hex_id\":\""<< this << "\"," << newLine;
	output << "\"frame\": " << now.frame() << "," << newLine;
	//phase.......
	if(getNOF_Phases() == 0)
		{
			buffOut <<output.str() << newLine << "}" << newLine << "}" << std::endl;
			return;
		}
	output << "\"currPhase\": \"" << getPhases()[currPhaseID].getName() << "\"," << newLine;
	output << "\"phases\":" << newLine << "[";

	for(int i =0; i < getPhases().size(); i++)
	{
		output << getPhases()[i].outputPhaseTrafficLight(newLine);
		if((i + 1) < getPhases().size()) output << ",";
	}
	output << newLine << "]";
	buffOut <<output.str() << newLine << "}" << newLine << "}" << std::endl;
}

std::size_t sim_mob::Signal_SCATS::computeCurrPhase(double currCycleTimer)
{
	std::vector< double > currSplitPlan = splitPlan.CurrSplitPlan();

	double sum = 0;
	int i;
	for(i = 0; i < getNOF_Phases(); i++)
	{
		//expanded the single line loop, for better understanding of future readers
		sum += splitPlan.getCycleLength() * currSplitPlan[i] / 100; //in each iteration sum will represent the time (with respect to cycle length) each phase would end
		if(sum > currCycleTimer)
			{
				break;
			}
	}

	if(i >= getNOF_Phases())
		{
			std::stringstream str;
			str << "Signal " << this->getId() << " CouldNot computeCurrPhase for the given currCycleTimer(" << currCycleTimer <<  "/" << splitPlan.getCycleLength() << ") getNOF_Phases()(" <<  getNOF_Phases() << ") , sum of cycleLength chunks(" << sum << ")";
			throw std::runtime_error(str.str());
		}
	currPhaseID = (std::size_t)(i);
	return currPhaseID;
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
Entity::UpdateStatus sim_mob::Signal_SCATS::frame_tick(timeslice now)
{
//UpdateStatus Signal_SCATS::update(timeslice now) {
	if(!isIntersection_) return UpdateStatus::Continue;
	isNewCycle = false;
//	1- update current cycle timer( Signal_SCATS::currCycleTimer)
	isNewCycle = updateCurrCycleTimer();
	//if the phase has changed, here we dont update currPhaseID to a new value coz we still need some info(like DS) obtained during the last phase
	//	3-Update Current Phase
	int temp_PhaseId = computeCurrPhase(currCycleTimer);

//	2- update current phase color
	if(temp_PhaseId < getNOF_Phases())
		{
			getPhases()[temp_PhaseId].update(currCycleTimer);
		}
	else {
		throw std::runtime_error("currPhaseID out of range");
	}

	//Temporarily set to the old value; use an enum if you actually want different timing modes.
	const bool signalTimingMode = true;

	if((currPhaseID != temp_PhaseId) && signalTimingMode)//separated coz we may need to transfer computeDS here
		{
			computePhaseDS(currPhaseID);
			currPhaseID  = temp_PhaseId;
//			updateLaneState(currPhaseID);
		}
	if(isNewCycle && signalTimingMode)
	{
		newCycleUpdate();
		initializePhases();
	}
	//and now deliver your efforts to the destination:
	buffer_output(now, "");
	return UpdateStatus::Continue;
}

/**
 * I will try to change only the Data structure, not the algorithm-vahid
 * This function will tell you what lights a driver is gonna get when he is at the traffic signal
 * this is done based on the lan->rs->link he is in.
 * He will get three colors for three options of heading directions(left, forward,right)
 */

TrafficColor sim_mob::Signal_SCATS::getDriverLight(Lane const & fromLane, Lane const & toLane) const {
	RoadSegment const * fromRoad = fromLane.getRoadSegment();
	Link * const fromLink = fromRoad->getLink();

	RoadSegment const * toRoad = toLane.getRoadSegment();
	Link const * toLink = toRoad->getLink();

	const sim_mob::Phase &currPhase = getCurrPhase();
	sim_mob::links_map &linkMap = currPhase.getlinksMap();
	sim_mob::Phase::links_map_equal_range range = currPhase.getLinkTos(fromLink);
	sim_mob::Phase::links_map_const_iterator iter;
//	if(range.first == range.second){
//		Print() << "From Link [" << fromLink << "] is not in the linksMap(range is empty)\nAvailable maps:" << std::endl;
//		for(iter = linkMap.begin(); iter != linkMap.end() ; iter++ ){
//			Print() << "Link-Link [" << iter->first << " , " << iter->second.LinkTo << "]" << std::endl;
//		}
//	}
	for(iter = range.first; iter != range.second ; iter++ )
	{
		if((*iter).second.LinkTo == toLink){
			break;
		}
	}

	//if the link is not listed in the current phase throw an error (alternatively, just return red)
	if(iter == range.second)
	{
//		std::ostringstream out ("");
//		Print() << "Unknown link map[" <<  fromLink << "," << toLink << "] out of [" <<linkMap.size() << "] recoreds" << std::endl;
//		throw std::runtime_error(out.str());
		return sim_mob::Red;
	}
//	Print() << "Retrurning the Color" << std::endl;
	return (*iter).second.currColor;
}
/*checks current phase for the current color of the crossing(if the crossing found),
 * other cases and phases, just return red.
 */
TrafficColor sim_mob::Signal_SCATS::getPedestrianLight (Crossing const & crossing) const
{
	const sim_mob::Phase & phase = getCurrPhase();
	const sim_mob::Phase::crossings_map_const_iterator it = phase.getCrossingMaps().find((const_cast<Crossing *>(&crossing)));
	if(it != phase.getCrossingMaps().end())
	{
		return (*it).second.currColor;
	}
	return sim_mob::Red;
}


void sim_mob::Signal_SCATS::addSignalSite(centimeter_t /* xpos */, centimeter_t /* ypos */,
		std::string const & /* typeCode */, double /* bearing */) {
	// Not implemented yet.
}

/* Set Split plan  for the signal*/
//might not be very necessary(not in use)
void sim_mob::Signal_SCATS::setSplitPlan(sim_mob::SplitPlan plan)
{
	splitPlan = plan;
}
void sim_mob::Signal_SCATS::initializePhases() {

	 /* you have each phase percentage from the choice set,
		so you may set the phase percntage and phase offset of each phase,
		then initialize phases(calculate its phase length, green time ...)*/
	std::vector<double> choice = splitPlan.CurrSplitPlan();
	if(choice.size() != getNOF_Phases())
		throw std::runtime_error("Mismatch on number of phases");
	int i = 0 ; double percentage_sum =0;
	//setting percentage and phaseoffset for each phase
	for(int ph_it = 0; ph_it < getPhases().size(); ph_it++, i++)
	{
		//this ugly line of code is due to the fact that multi index renders constant versions of its elements
		sim_mob::Phase & target_phase = const_cast<sim_mob::Phase &>(getPhases()[ph_it]);
		if( i > 0) percentage_sum += choice[i - 1]; // i > 0 : the first phase has phase offset equal to zero,
		(target_phase).setPercentage(choice[i]);
		(target_phase).setPhaseOffset(percentage_sum * splitPlan.getCycleLength() / 100);
	}
	//Now Initialize the phases(later you  may put this back to the above phase iteration loop
	for(int ph_it = 0; ph_it < getPhases().size(); ph_it++, i++)
		const_cast<sim_mob::Phase &>((getPhases()[ph_it])).initialize(splitPlan);//phaselength,and green time..
}

/**Signal Initialization */
void sim_mob::Signal_SCATS::initialize() {
	createStringRepresentation("");
	//initialize phases......
//	splitPlan.initialize();
	NOF_Phases = getNOF_Phases();//remove this error prone line later
	initializePhases();
	Phase_Density.resize(getNOF_Phases(), 0);

}

/* Get Split plan  for the signal*/
//might not be very necessary(not in use)
const sim_mob::SplitPlan & sim_mob::Signal_SCATS::getPlan() const
{
	return splitPlan;
}

sim_mob::SplitPlan & sim_mob::Signal_SCATS::getPlan()
{
	return splitPlan;
}

/**
 * computes and returns the signal phases in the next t seconds if the signal algorithm is Fixed.
 * returns a vector of pairs of <phase, time in the phase>
 */
std::vector<std::pair<sim_mob::Phase, double> > sim_mob::Signal_SCATS::predictSignal(double t)
{
	throw std::runtime_error("Signal_SCATS::predictSignal() is based on an old Signal.hpp; you'll have to change it.");
/*	std::vector<std::pair<sim_mob::Phase, double> > phaseTimes;
	if(signalTimingMode == 0  && t > 0){
		int phaseId = currPhaseID;
		sim_mob::Phase p = splitPlan.phases[phaseId];
		// add the remaining time in the current phase
		double remainingTimeInCurrPhase = p.phaseLength - (currCycleTimer - p.phaseOffset);
		phaseTimes.push_back(std::make_pair(p, std::min(remainingTimeInCurrPhase, t)));
		t = t - remainingTimeInCurrPhase;

		// add the subsequent phases which fit into this time window
		while(t > 0) {
			phaseId = (phaseId + 1) % splitPlan.NOF_Plans;
			sim_mob::Phase p = splitPlan.phases[phaseId];
			if (p.phaseLength <= t) {
				phaseTimes.push_back(std::make_pair(p, p.phaseLength));
				t = t - p.phaseLength;
			}
			else {
				phaseTimes.push_back(std::make_pair(p, t));
				t = 0;
			}
		}
	}
	return phaseTimes;*/
}



