/*
 * MobilityServiceController.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha, Andrea Araldo
 */

#ifndef MobilityServiceController_HPP_
#define MobilityServiceController_HPP_
#include <vector>
#include <string>

#include "entities/Agent.hpp"
#include "message/Message.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "MobilityServiceController.hpp"
#include "entities/controllers/Rebalancer.hpp"

namespace sim_mob
{

/**
 * This class is a sort of smart implementation of set. Indeed, in debug mode, it assures unicity of its members.
 * In release mode this check is not operated and the insertion is faster. Use this when you know that your code
 * never tries to add twice the same object
 */
template <class T> class Group
{
public:
	Group():elements( std::list<T>() ){};
	~Group(){};


	bool operator==(const Group<T>& other) const
	{
		return (elements == other.getElements() );
	}

	void insert(const T& r)
	{
		#ifndef NDEBUG
		if ( std::find(elements.begin(), elements.end(), r) != elements.end() )
		{
			std::stringstream msg; msg<<"Trying to insert "<<r<<" to a request group that already contains it. This denotes there is a bug somewhere";
			throw std::runtime_error(msg.str() );
		}
		#endif
		elements.push_back(r);
	}

	const std::list<T>& getElements() const
	{		return elements; }

	size_t size() const
	{ return elements.size(); }

	const T front() const
	{ return elements.front();}

protected:
	std::list<T> elements;
};






class OnCallController : public MobilityServiceController
{
protected:
	explicit OnCallController(const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered, unsigned int computationPeriod=0,
			MobilityServiceControllerType type_ = SERVICE_CONTROLLER_UNKNOWN)
		: MobilityServiceController(mtxStrat, computationPeriod, type_), scheduleComputationPeriod(computationPeriod)
	{
		rebalancer = new SimpleRebalancer();
#ifndef NDEBUG
		isComputingSchedules = false;
#endif
	}

public:

	virtual ~OnCallController();


	/*
	 * It returns the pointer to the driver closest to the node
	 */
	virtual const Person* findClosestDriver(const Node* node) const;



protected:


	/**
	 * Inherited from base class to output result
	 */
	virtual void frame_output(timeslice now);

	/**
	 * Inherited from base class to handle message
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * Makes a vehicle driver unavailable to the controller
	 * @param person Driver to be removed
	 */
	virtual void driverUnavailable(Person* person);

	/** Store list of available drivers */
	std::vector<const Person*> availableDrivers;

	/** Store queue of requests */
	//TODO: It should be vector<const TripRequest>, but it does not compile in that case:
	// check why
	std::list<TripRequestMessage> requestQueue;

	virtual void assignSchedule(const Person* driver, const Schedule& schedule);

	virtual bool isCruising(const Person* driver) const;
	virtual const Node* getCurrentNode(const Person* driver) const;
	/**
	 * Performs the controller algorithm to assign vehicles to requests
	 */
	virtual void computeSchedules() = 0;


	/**
	 * Computes a hypothetical schedule such that a driver located at a certain position can serve her current schedule
	 * as well as additional requests. The hypothetical schedule is written in newSchedule.
	 * The return value is the travel time if a feasible schedule is found, otherwise -1 is returned.
	 * If isOptimalityRequired is true, the function computes the schedule that minimizes the travel time. Otherwise, any feasible
	 * schedule is computed.
	 */
	virtual double computeSchedule(const Node* initialPositon, const Schedule& currentSchedule,
			const Group<TripRequestMessage>& additionalRequests,
			Schedule& newSchedule, bool isOptimalityRequired) const;

	/**
	 * Checks if the schedule is feasible, i.e. if:
	 * 1. no user is scheduled to be dropped off before being picked up
	 * 2. no user will experience an additional delay above the additionalDelayThreshold. The additonal delay is the
	 *    increase in her total time (including waiting and travel time
	 *    with respect to the case when she moves with a private vehicle
	 * 3. no user will experience a waiting time above waitingTimeThreshold
	 * If the schedule is feasible, it returns the travel time. Otherwise, it returns a negative number.
	 * The return value and the thresholds are expressed in ms.
	 */
	virtual double evaluateSchedule(const Node* initialPositon, const Schedule& schedule, double additionalDelayThreshold, double waitingTimeThreshold) const;

	/**
	 * True if it is possible to combine these two requests together while respecting the constraints
	 */
	virtual bool canBeShared(const TripRequestMessage& r1, const TripRequestMessage& r2,
			double additionalDelayThreshold, double waitingTimeThreshold ) const;

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);




#ifndef NDEBUG
	bool isComputingSchedules; //true during computing schedules. Used for debug purposes
	void consistencyChecks(const std::string& label) const;
#endif

	/**
	 * Subscribes a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	virtual void subscribeDriver(Person* person);

	/**
	 * Unsubscribes a vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	virtual void unsubscribeDriver(Person* person);

	/**
	 * Makes a vehicle driver available to the controller
	 * @param person Driver to be added
	 */
	virtual void driverAvailable(const Person* person);


	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages */
	unsigned int scheduleComputationPeriod;

	Rebalancer* rebalancer;

	/**
	 * Associates to each driver her current schedule. If a driver has nothing to do,
	 * an empty schedule is associated to her
	 */
	std::map<const Person*, Schedule> driverSchedules;

	//TODO: These should not be hardcoded
	const double additionalDelayThreshold = std::numeric_limits<double>::max();
	const double waitingTimeThreshold = std::numeric_limits<double>::max();
	const unsigned maxVehicleOccupancy = 3;

};
}

template <class T> std::ostream& operator<<(std::ostream& strm, const sim_mob::Group<T>& group)
{
	strm<<"RequestGroup [";
	for (const sim_mob::TripRequestMessage& r: group.getElements())
	{
		strm<< r <<", ";
	}
	strm<<" ]";
	return strm;
}

#endif /* MobilityServiceController_HPP_ */
