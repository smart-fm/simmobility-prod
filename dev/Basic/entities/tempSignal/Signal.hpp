/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 *      Autore: vahid
 */

#pragma once
#define NUMBER_OF_VOTING_CYCLES 5
#include <map>
#include <vector>

#include "GenConfig.h"
#include "Agent.hpp"
#include "metrics/Length.hpp"
#include "util/SignalStatus.hpp"
#include "entities/LoopDetectorEntity.hpp"
#include "plan.hpp"

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
	void initializeSignal();
	void updateSignal(double DS[]);

	void updateprevCL();
	void updatecurrCL();
	void updateprevRL1 (double RL1);
	void updateprevRL2 (double RL2);
	void setnextCL (double DS);
	void setCL (double prevCL1, double currCL1, double nextCL1);
	void setRL (double RL1, double RL2);
	std::string toString() const;
	void startSplitPlan();
	void setnextSplitPlan(double DS[]);
	void updatecurrSplitPlan();
	void setnextOffset(double nextCL);
	void updateOffset();
	double computeDS(double total_g);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g);
	double getprevCL() {return prevCL;}
	double getcurrCL() {return currCL;}
	double getnextCL() {return nextCL;}
	double getpreRL1() {return prevRL1;}
	double getpreRL2() {return prevRL2;}
	int getcurrSplitPlanID();
	int getnextSplitPlanID();
	double * getnextSplitPlan();
	double getcurrOffset();
	double getnextOffset();
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	void frame_output(frame_t frameNumber);
	static double fmax(const double proDS[]);
	static int fmin_ID(const double maxproDS[]);
	static int calvote(unsigned int vote1, unsigned int vote2, unsigned int vote3, unsigned int vote4, unsigned int vote5);
	void calProDS_MaxProDS(std::vector<double> &proDS,std::vector<double>  &maxproDS);
	Signal(Node const & node, const MutexStrategy& mtxStrat, int id=-1);
    Node const & getNode() const { return node_; }
	int getcurrPhase();
	int getphaseCounter(){return phaseCounter;}
	void updateTrafficLights();
	struct VehicleTrafficColors getDriverLight(Lane const & lane) const;
    TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const;
	TrafficColor getPedestrianLight(Crossing const & crossing) const;
    std::map<Link const *, size_t> const & links_map() const { return links_map_; }
    std::map<Crossing const *, size_t> const & crossings_map() const { return crossings_map_; }
    static Signal const & signalAt(Node const & node, const MutexStrategy& mtxStrat);
    static std::vector<Signal*> all_signals_;
    void addSignalSite(centimeter_t xpos, centimeter_t ypos,
                       std::string const & typeCode, double bearing);
    LoopDetectorEntity const & loopDetector() const { return loopDetector_; }
private:

    sim_mob::SplitPlan plan_;
    int signalAlgorithm;
    std::vector<double> Density; //donna what this is, still change it to vector
    double DS_all;


    Node const & node_;

	//previous,current and next cycle length
	double prevCL,currCL,nextCL;

	//two previous RL for calculating the current RL0
	double prevRL1,prevRL2;

	//SplitPlan that can be chosen to use
	static const double fixedSplitPlan[];
	static std::vector< std::vector<double> > SplitPlan;
	sim_mob::SplitPlan currSplitPlan,nextSplitPlan;

	int currSplitPlanID,nextSplitPlanID;//Don't think I will need it anymore

	//votes for determining next SplitPlan

/*  "votes" is a 2-D matrix where
 *  rows: is the totoal number of split plans
 *  columns: is the number of cycles we keep history of
 */
	std::vector< std::vector<int> > votes;//the size of this vector is = the number of available split plans

	//current and next Offset
	double currOffset,nextOffset;

	int phaseCounter;
//	int currPhase;
	sim_mob::Phase currPhase;

	//int TC_for_Driver[4][3];
	//Note: Making const* to make re-assigning easier. ~Seth
	//Need to serialize the attribute, fiexed array needed. (need to talk with Seth)
	int TC_for_Driver[4][3];
	int TC_for_Pedestrian[4];

	sim_mob::Shared<SignalStatus> buffered_TC;

	//String representation, so that we can retrieve this information at any time.
	std::string strRepr;

protected:
        std::map<Link const *, size_t> links_map_;
        std::map<Crossing const *, size_t> crossings_map_;

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
