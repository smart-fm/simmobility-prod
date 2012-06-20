/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.hpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */

#pragma once

//See entities/signal/Signal.hpp; we're just cross-dispatching here.
#include "GenConfig.h"
#ifdef SIMMOB_NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include <map>
#include <vector>
#include "Agent.hpp"
#include "metrics/Length.hpp"
#include "util/SignalStatus.hpp"
#include "entities/LoopDetectorEntity.hpp"

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


/**
 * Basic Signal class.
 *
 * \author Runmin Xu
 * \author Wang Xinyuan
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author Li Zhemin
 * \author LIM Fung Chai
 * \author Xu Yan
 *
 * NOTES:
 *1.The signal light uses adaptive signal control strategy (SCATS).
 *
 *2.The input value for SCATS is the degree of saturation(DS),this value
 *  can be recorded by loop detector, for now the loop section is
 *  nothing.We can define the value of DS as default or random.
 *
 *3.I am not so sure about how to add class Signal into the Basic Project,
 *  such as which folder to put it in, entities or roles? It needs to be
 *  updated and every agent should be able to get its information.
 *
 *4.According to MITSIMLab Definition, "1" means "red", "2" means yellow, "3" means "green"
 *
 */


class Signal  : public sim_mob::Agent {

public:
	void initializeSignal();

	//DS:degree of saturation
	void updateSignal(double DS[]);

	//set for the parameters in SCATS
	void updateprevCL();
	void updatecurrCL();
	void updateprevRL1 (double RL1);
	void updateprevRL2 (double RL2);
	void setnextCL (double DS);

	void setCL (double prevCL1, double currCL1, double nextCL1) {
		prevCL = prevCL1;
		currCL = currCL1;
		nextCL = nextCL1;
	}
	void setRL (double RL1, double RL2) {
		prevRL1 = RL1;
		prevRL2 = RL2;
	}

	///Return the loggable representation of this Signal.
	std::string toString() const { return strRepr; }

	//initialize the SplitPlan for SCATS
	void startSplitPlan();
	void setnextSplitPlan(double DS[]);
	void updatecurrSplitPlan();

	//Offset
	void setnextOffset(double nextCL);
	void updateOffset();

	double computeDS(double total_g);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,double total_g);


	//get the parameters in SCATS
	double getprevCL() {return prevCL;}
	double getcurrCL() {return currCL;}
	double getnextCL() {return nextCL;}
	double getpreRL1() {return prevRL1;}
	double getpreRL2() {return prevRL2;}
	int getcurrSplitPlanID() {return currSplitPlanID;}
	int getnextSplitPlanID() {return nextSplitPlanID;}
	double * getnextSplitPlan() {return &nextSplitPlan[0];}
	double getcurrOffset() {return currOffset;}
	double getnextOffset() {return nextOffset;}

	//Abstract methods. You will have to implement these eventually.
	virtual Entity::UpdateStatus update(frame_t frameNumber);
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	void frame_output(frame_t frameNumber);


	static double fmax(const double proDS[]);
	static int fmin_ID(const double maxproDS[]);
	static int calvote(unsigned int vote1, unsigned int vote2, unsigned int vote3, unsigned int vote4, unsigned int vote5);

	void setCurrLink(sim_mob::Link* link){currLink = link;}
	sim_mob::Link* getCurrLink() {return currLink;}

public:
        /**
         * Traffic light colors for both vehicles and pedestrians.
         *
         * Amber would not appear as a color to the pedestrians.  For modellers who are not
         * familiar with the Amber color, it is the same as Yellow.
         */
        enum TrafficColor
        {
            Red,    ///< Stop, do not go beyond the stop line.
            Amber,  ///< Slow-down, prepare to stop before the stop line.
            Green   ///< Proceed either in the forward, left, or right direction.
        };

        /**
         * Traffic light colors for vehicles in the forward, turn-left, and turn-right directions.
         *
         * Note that even if the color for the turn-left (or turn-right) direction is Red,
         * vehicles can still proceed to turn if Lane::can_turn_on_red_signal() returns true.
         * Vehicles must execute the turn carefully, giving way to crossing pedestrians and
         * oncoming traffic.
         *
         * Similarly even if the color is Green or Amber, vehicles cannot proceed if
         * Lane::can_go_straight(), Lane::can_turn_left(), Lane::can_turn_right() is false.
         * Modellers should check these functions first and execute lane-changing if needed.
         */
        struct VehicleTrafficColors
        {
            TrafficColor left;     ///< Traffic-color for the left direction.
            TrafficColor forward;  ///< Traffic-color for the forward direction.
            TrafficColor right;    ///< Traffic-color for the right direction.

            /// Constructor.
            VehicleTrafficColors(TrafficColor l, TrafficColor f, TrafficColor r)
              : left(l)
              , forward(f)
              , right(r)
            {
            }
        };

	Signal(Node const & node, const MutexStrategy& mtxStrat, int id=-1);

        /**
         * Return the road-network node where this Signal is located.
         */
        Node const & getNode() const { return node_; }

	int getcurrPhase();
	int getphaseCounter(){return phaseCounter;}
	void updateTrafficLights();

        /**
         * Return the traffic light colors for vehicles driving in the specified lane.
         *
         * This method returns the colors for the 3 directions (turn-left, forward, turn-right).
         * Modellers should check Lane::can_go_straight(), Lane::can_turn_left(), or
         * Lane::can_turn_right() as well as the corresponding color to decide whether to proceed,
         * slow down, or stop.
         */
	struct VehicleTrafficColors getDriverLight(Lane const & lane) const;

        /**
         * Return the traffic light color for vehicles travelling from one lane to another.
         *   \param fromLane The current lane.
         *   \param toLane The intended lane.
         *
         * This method returns the color for driving from \c fromLane to \c toLane.  Since the
         * movement entails travelling in either the forward, turn-left, or turn-right direction,
         * modellers
         * should check fromLane.can_go_straight(), fromLane.can_turn_left(),
         * or fromLane::can_turn_right() as well as the returned color.
         *
         * Currently the method does not handle U-turn at signalized intersections.
         */
        TrafficColor getDriverLight(Lane const & fromLane, Lane const & toLane) const;

        /**
         * Return the traffic light color for pedestrians wishing to walk on the specified crossing.
         */
	TrafficColor getPedestrianLight(Crossing const & crossing) const;

        /**
         * Return the internal links mapping.
         */
        std::map<Link const *, size_t> const & links_map() const { return links_map_; }
        /**
         * Return the internal crossings mapping.
         */
        std::map<Crossing const *, size_t> const & crossings_map() const { return crossings_map_; }

        /**
         * Return the Signal that is located at the specified \c node, creating one if necessary.
         */
        static Signal const & signalAt(Node const & node, const MutexStrategy& mtxStrat);

        /**
         * The list of all Signal objects in the simulator.
         */
        static std::vector<Signal*> all_signals_;


        /**
         * Furnish this Signal information about the position, type, and direction of its various
         * signal equipment.
         *
         * There are various equipment associated with a traffic signal.  Examples are pedestrian
         * signals, green filter arrow signals, overhead signals, B-signals (bus signals), etc.
         * This method is used to supply information about the position, type, and direction
         * of one equipment to the Signal object, which uses the information to determine its
         * "responsibilities".  For example, at a 4-way traffic intersection, pedestrians may not
         * be allowed to cross on one of the 4 sides.  In that case, no pedestrian signal will be
         * erected in that direction and the Signal object should not "cater" to pedestrians on
         * that side.
         */
        void addSignalSite(centimeter_t xpos, centimeter_t ypos,
                           std::string const & typeCode, double bearing);

        /** \cond doNotExpose -- Start of block to be ignored by doxygen.  */
        // This method is meant for the simulator kernel.  It is a public method because it is
        // needed in another area of the kernel (conf/simpleconf.cpp) and I want to avoid the use
        // of the friend keyword.
        LoopDetectorEntity const & loopDetector() const { return loopDetector_; }
        /** \endcond doNotExpose -- End of block to be ignored by doxygen.  */

private:

    int signalAlgorithm;
    double Density[4];
    double DS_all;

        Node const & node_;

	//previous,current and next cycle length
	double prevCL,currCL,nextCL;

	//two previous RL for calculating the current RL0
	double prevRL1,prevRL2;

	//SplitPlan that can be chosen to use
	static const double SplitPlan1[], SplitPlan2[], SplitPlan3[], SplitPlan4[], SplitPlan5[], fixedSplitPlan[];
	//current and next SplitPlan
	std::vector<double>currSplitPlan;
	std::vector<double>nextSplitPlan;
	//double currSplitPlan[4],nextSplitPlan[4];


	int currSplitPlanID,nextSplitPlanID;

	//votes for determining next SplitPlan
	int vote1, vote2, vote3, vote4, vote5;

	//current and next Offset
	double currOffset,nextOffset;

	int phaseCounter;
	int currPhase;

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
};

}
#endif
