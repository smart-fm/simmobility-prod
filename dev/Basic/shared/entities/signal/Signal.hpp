/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-5-1
 *      Author: xrm
 *      Autore: vahid
 */

#pragma once

#include "conf/settings/DisableMPI.h"

#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>
#include "entities/Agent.hpp"
#include "metrics/Length.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "SplitPlan.hpp"
#include "Phase.hpp"
#include "Cycle.hpp"
#include "Offset.hpp"
#include "defaults.hpp"

//For forward declarations (for friend functions)
#include "geospatial/xmlWriter/xmlWriter.hpp"


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>


namespace sim_mob
{
namespace xml
{
class Signal_t_pimpl;
}
// Forwared declarations.
class Node;
class Lane;
class Crossing;
class Link;

typedef struct
{
    sim_mob::Link* LinkTo;
    sim_mob::Link* LinkFrom;
    mutable sim_mob::TrafficColor currColor;//can change it directly as it is not a member of any key and it is mutable
}linkToLink_signal;

typedef boost::multi_index_container<
		linkToLink_signal,
		boost::multi_index::indexed_by<	boost::multi_index::random_access<> >
> linkToLink_ck_C;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

enum signalType
{
	SIG_BASIC = 3,
	SIG_SCATS = 4,
	SIG_BACKPRESSURE = 5
};
class Signal  : public sim_mob::Agent
{
public:
	typedef std::vector<sim_mob::Phase> phases;
	friend class sim_mob::xml::Signal_t_pimpl;
	Signal(Node const & node, const MutexStrategy& mtxStrat, int id=-1, signalType = SIG_BASIC)
	  : Agent(mtxStrat, id), node_(node){};
   signalType getSignalType() const { return signalType_;}
   void setSignalType(signalType sigType) { signalType_ = sigType;}
   virtual LinkAndCrossingByLink const & getLinkAndCrossingsByLink() const { throw std::runtime_error("getLinkAndCrossingsByLink Not implemented"); };
   virtual LinkAndCrossingByLink  & getLinkAndCrossingsByLink()  { throw std::runtime_error("getLinkAndCrossingsByLink Not implemented"); };
   virtual LinkAndCrossingByCrossing  & getLinkAndCrossingsByCrossing() const{ throw std::runtime_error("getLinkAndCrossingsByCrossing Not implemented"); };
   void setLinkAndCrossing(LinkAndCrossingC & LinkAndCrossings) { LinkAndCrossings_ =LinkAndCrossings; }
   LinkAndCrossingC const& getLinkAndCrossing()const { return LinkAndCrossings_;}
   LinkAndCrossingC & getLinkAndCrossing() { return LinkAndCrossings_;}
   virtual TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const { throw std::runtime_error("getDriverLight Not implemented"); } ;
   virtual TrafficColor getPedestrianLight (Crossing const & crossing)const { throw std::runtime_error("getPedestrianLight Not implemented"); };
   virtual std::string toString() const{ throw std::runtime_error("toString Not implemented"); };
   Node  const & getNode() const { return node_; }

   virtual void outputTrafficLights(timeslice now,std::string newLine)const{};

   virtual unsigned int getSignalId(){ return -1;}
   
	//Signals are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

   virtual void createStringRepresentation(std::string){};
   virtual ~Signal(){}
   virtual void load(const std::map<std::string, std::string>&) {}
   //virtual Entity::UpdateStatus update(timeslice now){ return Entity::UpdateStatus::Continue; }
   virtual sim_mob::Signal::phases &getPhases(){ return phases_;}
   virtual const sim_mob::Signal::phases &getPhases() const{ return phases_;}
   void addPhase(sim_mob::Phase phase) { phases_.push_back(phase); }
   bool frame_init(timeslice){}
   sim_mob::Entity::UpdateStatus frame_tick(timeslice){ return UpdateStatus::Continue; }
   void frame_output(timeslice){}

   typedef std::vector<Signal *> All_Signals;
   static All_Signals all_signals_;
   typedef std::vector<sim_mob::Signal *>::const_iterator all_signals_const_Iterator;
   typedef std::vector<sim_mob::Signal *>::iterator all_signals_Iterator;

private:
   /*The node associated with this traffic Signal */
   sim_mob::Node const & node_;
   sim_mob::signalType signalType_;
   LinkAndCrossingC LinkAndCrossings_;
   sim_mob::Signal::phases phases_;
};


class Signal_SCATS  : public sim_mob::Signal {
	friend class sim_mob::xml::Signal_t_pimpl;
friend  void sim_mob::WriteXMLInput_TrafficSignal(TiXmlElement * Signals,sim_mob::Signal *signal);
public:
	typedef std::vector<sim_mob::Phase>::iterator phases_iterator;

	/*--------Initialization----------*/
	void initialize();
	void setSplitPlan(sim_mob::SplitPlan);
	Signal_SCATS(Node const & node,const MutexStrategy& mtxStrat,int id=-1, signalType = SIG_SCATS);
    static Signal_SCATS const & signalAt(Node const & node, const MutexStrategy& mtxStrat,bool *isNew=nullptr);//bool isNew : since this function will create and return new signal if already existing signals not found, a switch to indicate what happened in the function would be nice

	//Note: You need a virtual destructor or else superclass destructors won't be called. ~Seth
    //created virtual for the immediate parent.
	~Signal_SCATS() {}

    void addSignalSite(centimeter_t xpos, centimeter_t ypos,std::string const & typeCode, double bearing);
//    void findIncomingLanes();
//    void findSignalLinks();
    void findSignalLinksAndCrossings();
    LinkAndCrossingByLink const & getLinkAndCrossingsByLink() const {return getLinkAndCrossing().get<2>();}
    LinkAndCrossingByLink & getLinkAndCrossingsByLink()  {return getLinkAndCrossing().get<2>();}
    LinkAndCrossingByCrossing  & getLinkAndCrossingsByCrossing()  {return getLinkAndCrossing().get<4>();}
    LoopDetectorEntity const & loopDetector() const { return *loopDetector_; }


	/*--------Updation----------*/
	void updateTrafficLights();
	void updatecurrSplitPlan();
	void updateOffset();
	void newCycleUpdate();
	bool updateCurrCycleTimer();

protected:
	virtual bool frame_init(timeslice now) { return true; }
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

private:
	//Output hack
	void buffer_output(timeslice now, std::string newLine);
	std::stringstream buffOut;

public:
	//virtual Entity::UpdateStatus update(timeslice now);

	/*--------Split Plan----------*/
//	void startSplitPlan();
//	void setnextSplitPlan(double DS[]);
	int getcurrSplitPlanID();
	int getnextSplitPlanID();
	sim_mob::SplitPlan & getPlan();

	/*--------Degree of Saturation----------*/
	double computeDS();
	double computePhaseDS(int phaseId);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g);

	/*--------Miscellaneous----------*/

	int getSignalTimingMode() { return signalTimingMode;}
	void setSignalTimingMode(int mode) { signalTimingMode = mode;}
	
	int fmin_ID(const  std::vector<double>  maxproDS);
	///Return the loggable representation of this Signal.
	std::string toString() const;
	unsigned int getSignalId();
	unsigned int getSignalId() const;
	bool isIntersection();
	void createStringRepresentation(std::string newLine = "\n");
	void cycle_reset();
	Crossing const * getCrossing(RoadSegment const * road);
	virtual const sim_mob::Link* getCurrLink(){ return nullptr;}
	virtual void setCurrLink(const sim_mob::Link*){}
	virtual const sim_mob::Lane* getCurrLane() const{return nullptr; }
	virtual void setCurrLane(const sim_mob::Lane* lane){}
//	virtual const sim_mob::Signal::phases getPhases(){ return phases_;}

	/*--------The cause of this Module----------*/
    TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const;
	TrafficColor getPedestrianLight  (Crossing const & crossing) const;
	double getUpdateInterval(){return updateInterval; }

   // void outputTrafficLights(timeslice now,std::string newLine)const;


    /* phase
     *
     */
    std::size_t getNOF_Phases() const{
    	return getPhases().size();
//    return getNOF_Phases();
    }
    std::size_t & getCurrPhaseID() { return currPhaseID; }
    std::size_t computeCurrPhase(double currCycleTimer);
    const sim_mob::Phase & getCurrPhase() const {
    	return getPhases()[currPhaseID]; }

    void initializePhases();
    void printColors(double currCycleTimer);

/* From workers (may not need)
    void updateIndicators();
    void outputTrafficLights(frame_t frameNumber,std::string newLine)const;
    void updateLaneState(int phaseId);//for mid-term use
*/
    std::vector<std::pair<sim_mob::Phase, double> > predictSignal(double t);


private:
    bool isIntersection_;//generated
    //this is the interval on which the signal's update is called
    double updateInterval;//generated
    unsigned int signalID;//currently is equal to nodeId

    /* Fixed time or adaptive control */
    int signalTimingMode;//0: fixed, 1: adaptive  //todo: change this old name to a more decent name with enum values
    /*-------------------------------------------------------------------------
     * -------------------Geo Spatial indicators--------------------------------
     * ------------------------------------------------------------------------*/
//    /*The node associated with this traffic Signal */
//    sim_mob::Node const & node_;
    //todo check whether we realy need it? (this container and the function filling it)
    /*check done! only Phase_Density vector needs it for its size!!! i.e a count for the number of lines would also do
     * the job. I don't think we need this but I am not ommitting it until I check wether it will be usefull for the loop detector
     * (how much usful)
     * else, no need to store so many lane pointers unnecessarily
     */
//    std::vector<sim_mob::Lane const *>  IncomingLanes_;//The data for this vector is generated
    //used (probabely in createloopdetectors()


//    std::vector<sim_mob::Link const *>  SignalLinks;//The data for this vector is generated

    /*-------------------------------------------------------------------------
     * -------------------split plan Indicators--------------------------------
     * ------------------------------------------------------------------------*/
    /*
     * the split plan(s) used in this traffic signal are represented using this single variable
     * This variable has two main tasks
     * 1-hold plans' phase information and different combinations of phase time shares(choiceSet)
     * 2-decides/outputs/selects the next split plan(choiceSet combination) based on the the inputted DS
     */
    sim_mob::SplitPlan plan_;
    /*--------------Amendments: going to separatre phase from split plan-------*/

    std::size_t NOF_Phases; //getNOF_Phases() = number of phases = phases_.size()
    std::size_t currPhaseID;//Better Name is: phaseAtGreen (according to TE terminology)The phase which is currently undergoing green, f green, amber etc..

    /*-------------------------------------------------------------------------*/
//	std::vector<double> currSplitPlan;//a chunk in "choiceSet" container,Don't think I will need it anymore coz job is distributed to a different class
//	int currSplitPlanID;//Don't think I will need it anymore
//	int phaseCounter;//Don't think I will need it anymore coz apparently currCycleTimer will replace it
	double currCycleTimer;//The amount of time passed since the current cycle started.(in millisecond)

    /*-------------------------------------------------------------------------
     * -------------------Phase_Density Indicators-----------------------------------
     * ------------------------------------------------------------------------*/
     /* -donna what this is, still change it to vector-----
     * update 1: it is used as an argument in updateSignal
     * update 2: probabely this is the DS at each lane(curent assumption) */
    std::vector<double> Phase_Density;
    //so far this value is used to store the max value in the above(Phase_Density) vector
    double DS_all;

    /*-------------------------------------------------------------------------
     * -------------------Cycle Length Indicators------------------------------
     * ------------------------------------------------------------------------*/

	bool isNewCycle; //indicates whether operations pertaining to a new cycle should be performed

    /*-------------------------------------------------------------------------
     * -------------------Offset Indicators------------------------------
     * ------------------------------------------------------------------------*/
	//current and next Offset
	sim_mob::Offset offset_;
	double currOffset;

	//String representation, so that we can retrieve this information at any time.
	std::string strRepr;
//	sim_mob::Shared<SignalStatus> buffered_TC;

	friend class DatabaseLoader;
protected:
		//NOTE: See the old Signal class for why this has to (temporarily) be a pointer. ~Seth
        LoopDetectorEntity* loopDetector_;


#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif
};//class Signal_SCATS

}//namespace sim_mob

