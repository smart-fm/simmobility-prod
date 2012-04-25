/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 *      Autore: vahid
 */

#pragma once

#include <map>
#include <vector>

#include "GenConfig.h"
#include "Agent.hpp"
#include "metrics/Length.hpp"
#include "util/SignalStatus.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "plan.hpp"
#include "Cycle.hpp"
#include "Offset.hpp"
#include "defaults.hpp"
namespace sim_mob
{

// Forwared declarations.
class Node;
class Lane;
class Crossing;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

class Signal  : public sim_mob::Agent {

public:
	/*--------Initialization----------*/
	void initializeSignal();
	void setSplitPlan(sim_mob::SplitPlan);
	void setCycleLength(sim_mob::Cycle);
	Signal(Node const & node, const MutexStrategy& mtxStrat, int id=-1);
    static Signal const & signalAt(Node const & node, const MutexStrategy& mtxStrat);
    void addSignalSite(centimeter_t xpos, centimeter_t ypos,std::string const & typeCode, double bearing);
    void findIncomingLanes();
    LoopDetectorEntity const & loopDetector() const { return loopDetector_; }


	/*--------Updation----------*/
	void updateSignal(double DS[]);
	void updateTrafficLights();
	void updatecurrSplitPlan();
	void updateOffset();
	virtual Entity::UpdateStatus update(frame_t frameNumber);


	/*--------Split Plan----------*/
	void startSplitPlan();
	void setnextSplitPlan(double DS[]);
	int getcurrSplitPlanID();
	int getnextSplitPlanID();
	std::vector<double> getNextSplitPlan();
	std::vector<double> getCurrSplitPlan();


	/*--------Split Plan----------*/
	int getcurrPhase();
	int getphaseCounter(){return phaseCounter;}


	/*--------Degree of Saturation----------*/
	double computeDS(double total_g);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g);
	void calProDS_MaxProDS(std::vector<double> &proDS,std::vector<double>  &maxproDS);


	/*--------Miscellaneous----------*/
	Node const & getNode() const { return node_; }
	std::string toString() const;
	void frame_output(frame_t frameNumber);
	static double fmax(const double proDS[]);
	static int fmin_ID(const double maxproDS[]);


	/*--------The cause of this Module----------*/
	struct VehicleTrafficColors getDriverLight(Lane const & lane) const;
    TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const;
	TrafficColor getPedestrianLight(Crossing const & crossing) const;


    static std::vector<Signal*> all_signals_;
private:
    /* Fixed time or adaptive control */
    int signalAlgorithm;
    /*-------------------------------------------------------------------------
     * -------------------Geo Spatial indicators--------------------------------
     * ------------------------------------------------------------------------*/
    /*The node associated with this traffic Signal */
    sim_mob::Node const & node_;
    //todo check whether we realy need it? (this container and the function filling it)
    std::vector<sim_mob::Lane *> const & IncomingLanes_;//The data for this vector is generated

    /*-------------------------------------------------------------------------
     * -------------------split plan Indicators--------------------------------
     * ------------------------------------------------------------------------*/
    /*
     * the split plan(s) used in this traffic signal are represented using this single variable
     * This variable has two main tasks
     * 1-hold plans' phase information and different combinations of phase time shares(percentage)
     * 2-decides/outputs/selects the next split plan(percentage combination) based on the the inputted DS
     */
    sim_mob::SplitPlan plan_;

	std::vector<double> currSplitPlan,nextSplitPlan;//the percentages only
	int currSplitPlanID,nextSplitPlanID;//Don't think I will need it anymore
	int phaseCounter;

    /*-------------------------------------------------------------------------
     * -------------------Density Indicators-----------------------------------
     * ------------------------------------------------------------------------*/
     /* -donna what this is, still change it to vector-----
     * update 1: it is used as an argument in updateSignal
     * update 2: probabely this is the DS at each lane(curent assumption) */
    std::vector<double> Density;
    //so far this value is used to store the max value in the above(Density) vector
    double DS_all;

    /*-------------------------------------------------------------------------
     * -------------------Cycle Length Indicators------------------------------
     * ------------------------------------------------------------------------*/
    sim_mob::Cycle cycle_;
	//previous,current and next cycle length
	double prevCL,currCL,nextCL;

	//two previous RL for calculating the current RL0
	double prevRL1,prevRL2;

    /*-------------------------------------------------------------------------
     * -------------------Offset Indicators------------------------------
     * ------------------------------------------------------------------------*/
	//current and next Offset
	sim_mob::Offset offset_;
	double currOffset,nextOffset;




	//String representation, so that we can retrieve this information at any time.
	std::string strRepr;
	sim_mob::Shared<SignalStatus> buffered_TC;

protected:
        LoopDetectorEntity loopDetector_;

protected:
        void setupIndexMaps();
        void outputToVisualizer(frame_t frameNumber);

#ifndef SIMMOB_DISABLE_MPI
public:
    virtual void pack(PackageUtils& packageUtil){}
    virtual void unpack(UnPackageUtils& unpackageUtil){}

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

//	static std::vector< std::vector<double> > SplitPlan;
};//class Signal
}//namespace sim_mob
