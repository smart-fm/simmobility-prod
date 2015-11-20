//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>
#include <stdexcept>
#include <sstream>

#include "conf/settings/DisableMPI.h"
#include "Cycle.hpp"
#include "entities/Agent.hpp"
#include "entities/Sensor.hpp"
#include "metrics/Length.hpp"
#include "Offset.hpp"
#include "Phase.hpp"
#include "SplitPlan.hpp"

namespace sim_mob
{

class Node;
class Lane;
class Link;
class LoopDetector;
class BasicLogger;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**Structure to track the number of vehicles passing over a loop detector*/
struct VehicleCounter
{
public:
	/**The associated traffic signal*/
	const Signal_SCATS *signal;
	
	/**Start time of the simulation*/
	const DailyTime &simStartTime;
	
	/**Accumulation Period length in milliseconds seconds: E.g. return total count of vehicle in every "600,000" milliseconds*/
	const unsigned int frequency;
	
	/**Map of lane vs the number of vehicles detected by the loop detector*/
	std::map<const Lane *, int> counter;
	
	/**Instance of the logger*/
	BasicLogger &logger;
	
	/**Current time*/
	timeslice curTimeSlice;

public:
	VehicleCounter();
	~VehicleCounter();

	/**
	 * Initialises the vehicle counts for the loop detectors at the given signal 
	 * @param signal the signal
	 */
	void initialise(const Signal_SCATS *signal);

	/**
	 * Resets the vehicle counts of all loop detectors
	 */
	void resetCounter();

	/**
	 * Writes the vehicle counts to file
	 * @param time
	 */
	void serialize(const uint32_t &time);

	/**
	 * Updates the vehicle counts
	 */
	void update();

	/**
	 * Aggregates the vehicle counts according to the set frequency
	 * @param tick
	 */
	void aggregateCounts(const timeslice &tick);
};

/**Defines the supported types of signals*/
enum SignalType
{
	SIGNAL_TYPE_INVALID = 0,
	SIGNAL_TYPE_FIXED_TIME = 1, 
	SIGNAL_TYPE_SCATS = 2
};

/**Defines the abstract class for a traffic signal*/
class Signal : public Agent
{
protected:
	/**The id of the traffic signal*/
	unsigned int trafficLightId;
	
	/*The node associated with this traffic Signal */
	const Node *node;
	
	/**Indicates the type of the signal*/
	SignalType signalType;
	
	/**Container for the phases of the signal*/
	std::vector<Phase *> phases;

public:
	Signal(const Node *node, const MutexStrategy &mtxStrat, unsigned int agentId = -1, SignalType = SIGNAL_TYPE_INVALID);
	virtual ~Signal();
	
	const Node* getNode() const;	
	SignalType getSignalType() const;
	
	const std::vector<Phase *>& getPhases();
	
	/**
	 * Indicates whether the agent is non-spatial in nature
	 * 
	 * @return true, as signals are non-spatial
	 */
	virtual bool isNonspatial();

	/**
	 * Based on the current phase, gets the traffic light colour shown to the drivers accessing the given turning
	 * group
	 * 
	 * @param fromLink the id of the link the driver is arriving from
	 * @param toLink the id of the link the driver is moving towards
	 * 
	 * @return traffic light colour
	 */
	virtual TrafficColor getDriverLight(unsigned int fromLink, unsigned int toLink) const = 0;

	/**
	 * Compulsory override from Agent class (Does nothing for signals)
	 * @param 
	 */
	virtual void load(const std::map<std::string, std::string> &) = 0;
	
	/**
	 * This method is called for the first tick of the traffic signal to perform any initialisation tasks necessary
	 * (Does nothing for basic signals)
	 * @param 
	 * @return false, resulting in the Signal to be removed from the simulation
	 */
	virtual bool frame_init(timeslice) = 0;
	
	/**
	 * This method is called for every tick of the traffic signal. This is where the behaviour of the traffic signal is captured
	 * (Does nothing for basic signals)
	 * @param 
	 * @return UpdateStatus::Continue
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice) = 0;
	
	/**
	 * This method is called for every tick of the traffic signal. It generates the output from the signals to be 
	 * written to the output file
	 * @param 
	 */
	virtual void frame_output(timeslice) = 0;
} ;

class Signal_SCATS : public Signal
{
private:
	/**The interval on which the frame_tick method is called for the signal*/
	double updateInterval;
	
	/**
	 * Represents the split plan(s) used in this traffic signal. This has two main tasks:
	 * 1. Hold the phase information of the plan and the different combinations of phase time shares i.e. the choice set
	 * 2. Select the next split plan based on the input DS
	 */
	SplitPlan *splitPlan;
	
	/**The phase which is currently undergoing green*/
	unsigned int currPhaseAtGreen;
	
	/**The amount of time passed since the current cycle started.(in millisecond)*/
	double currCycleTimer;

	/**Stores the densities of the phases. Density of phase 'i' is stored at the 'i'th index*/
	std::vector<double> phaseDensity;
	
	/**Indicates whether operations pertaining to a new cycle should be performed*/
	bool isNewCycle;
	
	/**
	 * Initialises the signal
	 */
	void initialise();
	
	/**
	 * Updates the current current cycle timer
	 * 
	 * @return true, if we've reached the end of the cycle
	 */
	bool updateCurrCycleTimer();
	
	/**
	 * Computes the current phase
	 * 
	 * @param currCycleTimer
	 * @return the current phase
	 */
	std::size_t computeCurrPhase(double currCycleTimer);
	
	/**
	 * Calculates the degree of saturation (DS) at the end of each phase considering only the maximum DS of the lane in the LinkFrom(s).
	 * LinkFrom(s) are the links from which vehicles enter the intersection during the corresponding phase
	 * 
	 * @param phaseId the phase for which the DS is to be calculated
	 * @param now the current time frame
	 * 
	 * @return the degree of saturation for the given phase
	 */
	double computePhaseDS(int phaseId, const timeslice &now);
	
	/**
	 * This method does the actual DS computation.  It calculates the DS on a specific Lane at the moment 
	 * totalGreen amounts to totalGreen at each phase. 
	 * However this function doesn't care the scope (phase/cycle) the totalGreen comes from.
	 * 
	 * @param ctPair
	 * @param totalGreen
	 * 
	 * @return the degree of saturation for the given phase
	 */
	double computeLaneDS(const Sensor::CountAndTimePair &ctPair, double totalGreen);
	
	/**
	 * Updates the new cycle. This method is called only when we change to a new cycle
	 */
	void updateNewCycle();
	
	/**
	 * Resets the the phase densities of all the phases
	 */
	void resetCycle();
	
	/**
	 * Creates the split plans
	 */
	void createPlans();
	
	/**
	 * Creates the phases
	 */
	void createPhases();
	
	/**
	 * Initialises the phases
	 */
	void initialisePhases();

protected:
	VehicleCounter curVehicleCounter;
	Sensor *loopDetectorAgent;
	
	/**
	 * This method is called for the first tick of the traffic signal to perform any initialisation tasks necessary
	 * (Does nothing for SCATS signals)
	 * @param 
	 * @return true
	 */
	virtual bool frame_init(timeslice now);
	
	/**
	 * This method is called for every tick of the traffic signal. This method does the following:
	 * 1. Update the current cycle timer
	 * 2. Update the current phase
	 * 3. Update the current phase colour
	 * 4. If the cycle has ended:
	 *	4.1 Compute the degree of saturation (DS)
	 *	4.2 Update cycle length
	 *	4.3 Update split plan
	 *	4.4 Update offset
	 * 5. Reset the loop detector for the next cycle
	 * @param now
	 * @return UpdateStatus::Continue
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	
	/**
	 * This method is called for every tick of the traffic signal. It generates the output from the signals to be 
	 * written to the output file
	 * @param 
	 */
	virtual void frame_output(timeslice now);
	
	/**
	 * Compulsory override from Agent class (Does nothing for signals)
	 * @param 
	 */
	virtual void load(const std::map<std::string, std::string> &)
	{
	}

public:
	Signal_SCATS(const Node *node, const MutexStrategy &mtxStrat);
	virtual ~Signal_SCATS();

	/**
	 * Based on the current phase, gets the traffic light colour shown to the drivers accessing the given turning
	 * group
	 * 
	 * @param fromLink the id of the link the driver is arriving from
	 * @param toLink the id of the link the driver is moving towards
	 * 
	 * @return traffic light colour
	 */
	TrafficColor getDriverLight(unsigned int fromLink, unsigned int toLink) const;
	
	const Sensor* getLoopDetector() const
	{
		return loopDetectorAgent;
	}
	
	void setLoopDetector(Sensor *sensor)
	{
		loopDetectorAgent = sensor;
	}
	
	unsigned int getNumOfPhases() const
	{
		return phases.size();
	}

#ifndef SIMMOB_DISABLE_MPI
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);
	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

} ;

}

