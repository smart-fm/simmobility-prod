/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-5-1
 *      Author: xrm
 *      Autore: vahid
 */

#pragma once

//If we're not using the "new signal" flag, just forward this header file to the old location.
//  This allows us to simply include "entities/signal/Signal.hpp" without reservation.
#include "GenConfig.h"
#ifndef SIMMOB_NEW_SIGNAL
#include "entities/Signal.hpp"
#include "util/SignalStatus.hpp"
#else
#include <map>
#include <vector>
#include "entities/Agent.hpp"
#include "metrics/Length.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "SplitPlan.hpp"
#include "Phase.hpp"
#include "Cycle.hpp"
#include "Offset.hpp"
#include "defaults.hpp"


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace sim_mob
{

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
		boost::multi_index::indexed_by<
		boost::multi_index::random_access<>
  >
> linkToLink_ck_C;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

class Signal_Parent  : public sim_mob::Agent
{
public:
	Signal_Parent(Node const & node, const MutexStrategy& mtxStrat, int id=-1)
	  : Agent(mtxStrat, id), node_(node){};
   virtual LinkAndCrossingByLink const & getLinkAndCrossingsByLink() const {};
   virtual LinkAndCrossingByCrossing const & getLinkAndCrossingsByCrossing() const{};
   virtual TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const {} ;
   virtual TrafficColor getPedestrianLight(Crossing const & crossing) const {};
   virtual std::string toString() const{};
   Node  const & getNode() const { return node_; }

//   virtual unsigned int getSignalId()   {return TMP_SignalID;}
//   virtual unsigned int getSignalId() const  {return TMP_SignalID;}

   static std::vector<Signal_Parent *> all_signals_;
   typedef std::vector<sim_mob::Signal_Parent *>::const_iterator all_signals_const_Iterator;
   typedef std::vector<sim_mob::Signal_Parent *>::iterator all_signals_Iterator;

//	static sim_mob::Signal_Parent::all_signals all_signals_;
private:
   /*The node associated with this traffic Signal */
   sim_mob::Node const & node_;
};


class Signal  : public sim_mob::Signal_Parent {

public:

	/*--------Initialization----------*/
	void initialize();
	void setSplitPlan(sim_mob::SplitPlan);
//	void setCycleLength(sim_mob::Cycle);
	Signal(Node const & node,const MutexStrategy& mtxStrat,int id=-1);
    static Signal const & signalAt(Node const & node, const MutexStrategy& mtxStrat,bool *isNew = false);//bool isNew : since this function will create and return new signal if already existing signals not found, a switch to indicate what happened in the function would be nice
    void addSignalSite(centimeter_t xpos, centimeter_t ypos,std::string const & typeCode, double bearing);
    void findIncomingLanes();
    void findSignalLinks();
    void findSignalLinksAndCrossings();
    LinkAndCrossingByLink const & getLinkAndCrossingsByLink() const {return LinkAndCrossings_.get<2>();}
    LinkAndCrossingByCrossing const & getLinkAndCrossingsByCrossing() const {return LinkAndCrossings_.get<4>();}
//    const std::vector<sim_mob::Link const *> & getSignalLinks() const;
    LoopDetectorEntity const & loopDetector() const { return loopDetector_; }


	/*--------Updation----------*/
	void updateTrafficLights();
	void updatecurrSplitPlan();
	void updateOffset();
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	void newCycleUpdate();
	bool updateCurrCycleTimer();

	/*--------Split Plan----------*/
	void startSplitPlan();
//	void setnextSplitPlan(double DS[]);
	int getcurrSplitPlanID();
	int getnextSplitPlanID();
	sim_mob::SplitPlan & getPlan();

	/*--------Phase----------*/
	int getcurrPhase();
	int getphaseCounter(){return phaseCounter;}

	/*--------Degree of Saturation----------*/
	double computeDS();
	double computePhaseDS(int phaseId);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g);
	void calProDS_MaxProDS(std::vector<double> &proDS,std::vector<double>  &maxproDS);

	/*--------Miscellaneous----------*/
//	Node  const & getNode() const { return node_; }
	void frame_output(frame_t frameNumber);
	int fmin_ID(const  std::vector<double>  maxproDS);
	///Return the loggable representation of this Signal.
	std::string toString() const { return strRepr; }
	unsigned int getSignalId()   {return TMP_SignalID;}
	unsigned int getSignalId() const  {return TMP_SignalID;}
	bool isIntersection() { return isIntersection_;}
	void createStringRepresentation(std::string newLine = "\n");

	/*--------The cause of this Module----------*/
    TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const ;
	TrafficColor getPedestrianLight(Crossing const & crossing) const;
	double getUpdateInterval(){return updateInterval; }
	void cycle_reset();

//	typedef std::vector<sim_mob::Signal *> all_signals;
//
//	static sim_mob::Signal::all_signals all_signals_;

    void updateIndicators();
    void outputTrafficLights(frame_t frameNumber,std::string newLine)const;

private:
    bool isIntersection_;
    double updateInterval;
    unsigned int TMP_SignalID;//todo change the name to withouth TMP

    /* Fixed time or adaptive control */
    int signalAlgorithm;
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
    std::vector<sim_mob::Lane const *>  IncomingLanes_;//The data for this vector is generated
    //used (probabely in createloopdetectors()

    LinkAndCrossingC LinkAndCrossings_;
    std::vector<sim_mob::Link const *>  SignalLinks;//The data for this vector is generated

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

//	std::vector<double> currSplitPlan;//a chunk in "choiceSet" container,Don't think I will need it anymore coz job is distributed to a different class
	int currSplitPlanID;//Don't think I will need it anymore
	int phaseCounter;//Don't think I will need it anymore coz apparently currCycleTimer will replace it
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

	//previous,current and next cycle length
	double currCL;
	int currPhaseID;
	sim_mob::Phase currPhase;

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

//	//private override for friends ;)
//	static Signal & signal_at(Node const & node, const MutexStrategy& mtxStrat);
	friend class DatabaseLoader;
protected:
        LoopDetectorEntity loopDetector_;

protected:
//        void setupIndexMaps();
        void outputToVisualizer(frame_t frameNumber);

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil){};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil){};
#endif
//	static std::vector< std::vector<double> > SplitPlan;
};//class Signal

}//namespace sim_mob
#endif
