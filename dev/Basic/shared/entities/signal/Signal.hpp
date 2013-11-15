//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Signal.hpp
 *
 *  Created on: 2011-5-1
 *      Author: xrm
 *      Author: vahid
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
//#include "geospatial/xmlWriter/xmlWriter.hpp"

namespace sim_mob {
namespace xml {
class Signal_t_pimpl;
}
// Forwared declarations.
class Node;
class Lane;
class Crossing;
class Link;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

enum signalType {
	SIG_BASIC = 3, SIG_SCATS = 4, SIG_BACKPRESSURE = 5
};
class Signal: public sim_mob::Agent {
public:
	typedef std::vector<sim_mob::Phase> phases;
	friend class sim_mob::xml::Signal_t_pimpl;
	Signal(Node const & node, const MutexStrategy& mtxStrat, int id = -1,
			signalType = SIG_BASIC);
	signalType getSignalType() const;
	void setSignalType(signalType sigType);
	void setLinkAndCrossing(LinkAndCrossingC & LinkAndCrossings);
	LinkAndCrossingC const& getLinkAndCrossing() const;
	LinkAndCrossingC & getLinkAndCrossing();
	virtual TrafficColor getDriverLight(Lane const & fromLane,
			Lane const & toLane) const;
	virtual TrafficColor getPedestrianLight(Crossing const & crossing) const;
	virtual std::string toString() const;
	Node const & getNode() const;
	virtual void outputTrafficLights(timeslice now, std::string newLine) const;
	virtual unsigned int getSignalId() const;

	//Signals are non-spatial in nature.
	virtual bool isNonspatial();

	virtual void createStringRepresentation(std::string);
	virtual ~Signal();
	virtual void load(const std::map<std::string, std::string>&);
	//virtual Entity::UpdateStatus update(timeslice now){ return Entity::UpdateStatus::Continue; }
	virtual sim_mob::Signal::phases &getPhases();
	virtual const sim_mob::Signal::phases &getPhases() const;
	void addPhase(sim_mob::Phase &phase);
	//xuyan: no return here
	bool frame_init(timeslice);
	sim_mob::Entity::UpdateStatus frame_tick(timeslice);
	void frame_output(timeslice);

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

class Signal_SCATS: public sim_mob::Signal {
	friend class sim_mob::xml::Signal_t_pimpl;
//friend  void sim_mob::WriteXMLInput_TrafficSignal(TiXmlElement * Signals,sim_mob::Signal *signal);
public:
	void *tempLoop;
	typedef std::vector<sim_mob::Phase>::iterator phases_iterator;

	/*--------Initialization----------*/
	void initialize();
	void setSplitPlan(sim_mob::SplitPlan);
	Signal_SCATS(Node const & node, const MutexStrategy& mtxStrat, int id = -1,
			signalType = SIG_SCATS);
	static Signal_SCATS const & signalAt(Node const & node,
			const MutexStrategy& mtxStrat, bool *isNew = nullptr); //bool isNew : since this function will create and return new signal if already existing signals not found, a switch to indicate what happened in the function would be nice

	//Note: You need a virtual destructor or else superclass destructors won't be called. ~Seth
	//created virtual for the immediate parent.
	~Signal_SCATS() {
	}

	void addSignalSite(centimeter_t xpos, centimeter_t ypos,
			std::string const & typeCode, double bearing);
//    void findIncomingLanes();
//    void findSignalLinks();
	void findSignalLinksAndCrossings();
	LoopDetectorEntity const & loopDetector() const {
		return *loopDetector_;
	}

	/**
	 * --------Updation----------
	 */
	void updateTrafficLights();
	void updatecurrSplitPlan();
	void updateOffset();
	void newCycleUpdate();
	bool updateCurrCycleTimer();

protected:
	virtual bool frame_init(timeslice now) {
		return true;
	}
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

private:
	//Output hack
	void buffer_output(timeslice now, std::string newLine);
	std::stringstream buffOut;

public:
	/*--------Split Plan----------*/
	int getcurrSplitPlanID();
	int getnextSplitPlanID();
	const sim_mob::SplitPlan & getPlan() const;
	sim_mob::SplitPlan & getPlan();

	/*--------Degree of Saturation----------*/
	double computeDS();
	double computePhaseDS(int phaseId);
	double LaneDS(const LoopDetectorEntity::CountAndTimePair& ctPair,
			double total_g);

	/*--------Miscellaneous----------*/

	int fmin_ID(const std::vector<double> maxproDS);
	///	Return the loggable representation of this Signal.
	std::string toString() const;
	unsigned int getSignalId();
	unsigned int getSignalId() const;
	bool isIntersection();
	void createStringRepresentation(std::string newLine = "\n");
	void cycle_reset();
	Crossing const * getCrossing(RoadSegment const * road);
	virtual const sim_mob::Link* getCurrLink() {
		return nullptr;
	}
	virtual void setCurrLink(const sim_mob::Link*) {
	}
	virtual const sim_mob::Lane* getCurrLane() const {
		return nullptr;
	}
	virtual void setCurrLane(const sim_mob::Lane* lane) {
	}
	/**
	 * --------The cause of this Module----------
	 **/
	TrafficColor getDriverLight(Lane const & fromLane,
			Lane const & toLane) const;
	TrafficColor getPedestrianLight(Crossing const & crossing) const;
	double getUpdateInterval() {
		return updateInterval;
	}

	/* phase
	 *
	 */
	std::size_t getNOF_Phases() const {
		return getPhases().size();
//    return getNOF_Phases();
	}
	std::size_t & getCurrPhaseID() {
		return currPhaseID;
	}
	std::size_t computeCurrPhase(double currCycleTimer);
	const sim_mob::Phase & getCurrPhase() const {
		return getPhases()[currPhaseID];
	}

	void initializePhases();
	void printColors(double currCycleTimer);
	std::vector<std::pair<sim_mob::Phase, double> > predictSignal(double t);

private:
	bool isIntersection_;	//generated
	///	this is the interval on which the signal's update is called
	double updateInterval;    //generated
	///	currently is equal to nodeId
	unsigned int signalID;
	/*-------------------------------------------------------------------------
	 * -------------------split plan Indicators--------------------------------
	 * ------------------------------------------------------------------------*/
	/*
	 * the split plan(s) used in this traffic signal are represented using this single variable
	 * This variable has two main tasks
	 * 1-hold plans' phase information and different combinations of phase time shares(choiceSet)
	 * 2-decides/outputs/selects the next split plan(choiceSet combination) based on the the inputted DS
	 */
	sim_mob::SplitPlan splitPlan;
	std::size_t NOF_Phases; ///	getNOF_Phases() = number of phases = phases_.size()
	std::size_t currPhaseID; ///	Better Name is: phaseAtGreen (according to TE terminology)The phase which is currently undergoing green, f green, amber etc..
	double currCycleTimer; ///	The amount of time passed since the current cycle started.(in millisecond)

	/*-------------------------------------------------------------------------
	 * -------------------Phase_Density Indicators-----------------------------------
	 * ------------------------------------------------------------------------*/
	std::vector<double> Phase_Density;
	///	so far this value is used to store the max value in the above(Phase_Density) vector
	double DS_all;
	/*-------------------------------------------------------------------------
	 * -------------------Cycle Length Indicators------------------------------
	 * ------------------------------------------------------------------------*/
	bool isNewCycle; ///	indicates whether operations pertaining to a new cycle should be performed

	/*-------------------------------------------------------------------------
	 * -------------------Offset Indicators------------------------------
	 * ------------------------------------------------------------------------*/
	///	current and next Offset
	sim_mob::Offset offset_;
	double currOffset;

	///	String representation, so that we can retrieve this information at any time.
	std::string strRepr;

	friend class DatabaseLoader;
protected:
	LoopDetectorEntity* loopDetector_;

#ifndef SIMMOB_DISABLE_MPI
public:
	virtual void pack(PackageUtils& packageUtil) {}
	virtual void unpack(UnPackageUtils& unpackageUtil) {}

	virtual void packProxy(PackageUtils& packageUtil) {};
	virtual void unpackProxy(UnPackageUtils& unpackageUtil) {};
#endif
};

}//namespace sim_mob

