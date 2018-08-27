/*
 * MobilityServiceController.hpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha, Andrea Araldo
 */

#ifndef OnCallController_HPP_
#define OnCallController_HPP_
#include <string>
#include <vector>
#include <unordered_map>

#include "entities/Agent.hpp"
#include "entities/controllers/Rebalancer.hpp"
#include "message/Message.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "MobilityServiceController.hpp"


namespace sim_mob
{

class RoadSegment;

/**
 * This class is a sort of smart implementation of set. Indeed, in debug mode, it assures unicity of its members.
 * In release mode this check is not operated and the insertion is faster. Use this when you know that your code
 * never tries to add twice the same object
 */
template <class T> class Group
{
public:
	Group() : elements(std::list<T>())
	{};

	~Group()
	{};

	bool operator==(const Group<T> &other) const
	{
		return (elements == other.getElements());
	}

	bool operator<(const Group<T> &other) const
	{
		if (size() < other.size())
		{ return true; }
		if (size() > other.size())
		{ return false; }

		auto itMe = elements.begin();
		auto itOther = other.getElements().begin();

		while (itMe != elements.end())
		{
#ifndef NDEBUG
			if (itOther == other.getElements().end())
			{
				throw std::runtime_error(
						"this group and the other have the same size. If itMe is not at the end, the other should not be at the end as well");
			}
#endif
			if (*itMe < *itOther)
			{ return true; }
			if (*itMe > *itOther)
			{ return false; }

			itMe++;
			itOther++;
		}

		return false;
	}

	void insert(const T &r)
	{
#ifndef NDEBUG
		if (std::find(elements.begin(), elements.end(), r) != elements.end())
		{
			std::stringstream msg;
			msg << "Trying to insert " << r << " to a group that already contains it. The group is ";
			for (const T &element : elements)
			{
				msg << element << ", ";
			}
			msg << ". This denotes there is a bug somewhere";
			throw std::runtime_error(msg.str());
		}
#endif
		elements.push_back(r);
	}

	const std::list<T> &getElements() const
	{ return elements; }

	size_t size() const
	{ return elements.size(); }

	const T front() const
	{ return elements.front(); }

	bool contains(const T &r) const
	{
		return (std::find(elements.begin(), elements.end(), r) != elements.end());
	}

protected:
	std::list<T> elements;
};

/**
 * See how they are used inside the function getTT(..)
 */
enum TT_EstimateType
{
	OD_ESTIMATION,
	SHORTEST_PATH_ESTIMATION,
	EUCLIDEAN_ESTIMATION // The least computationally expensive one
};

class OnCallController : public MobilityServiceController
{
protected:
	// We use explicit to avoid accidentally passing an integer instead of a MobilityServiceControllerType
	// (see https://stackoverflow.com/a/121163/2110769).
	// The constructor is protected to avoid instantiating an OnCallController directly, since it is conceptually abstract
	explicit OnCallController(const MutexStrategy& mtxStrat, unsigned int computationPeriod,
			MobilityServiceControllerType type_, unsigned id, std::string tripSupportMode_,TT_EstimateType ttEstimateType,
                              unsigned maxAggregatedRequests_, bool studyAreaEnabledController_,unsigned int toleratedExtraTime_,unsigned int maxWaitingTime_,bool parkingEnabled);

public:
	virtual ~OnCallController();

	/*
	 * It returns the pointer to the driver closest to the node
	 */
	virtual const Person* findClosestDriver(const Node* node) const;

	virtual const std::string getRequestQueueStr() const;

	virtual void sendCruiseCommand(const Person* driver, const Node* nodeToCruiseTo, const timeslice currTick ) const;

	/**
	 * Estimates the travel time to go from node1 to node2. In seconds
	 */
	double getTT(const Node* node1, const Node* node2, TT_EstimateType typeOD) const;

	/**
	 * Estimates the travel time to go from point1 to point2. In seconds
	 */
	double getTT(const Point& point1, const Point& point2) const;

	/**
	 * Converts from number of clocks to milliseconds
	 */
	double toMs(int c) const;

	/**
	 * Checks if the driver is cruising
	 */
	virtual bool isCruising(const Person* driver) const;
	virtual bool isParked(const Person *driver) const;
	virtual bool isJustStated(const Person *driver) const;
	virtual const Node* getCurrentNode(const Person* driver) const;

	/**
	 * Unsubscribes a vehicle driver from the controller
	 * @param person Driver to be removed
	 */
	virtual void unsubscribeDriver(Person* person);

	virtual std::map<const Person*, Schedule> & getControllerCopyDriverSchedulesMap()
	{
		return 	driverSchedules;
	}

	virtual std::set<const Person *> getAvailableDriverSet()
	{
		return 	availableDrivers;

	}

protected:
	/** Store list of available drivers */
	std::set<const Person *> availableDrivers;

	/** Store queue of requests */
	std::list<TripRequestMessage> requestQueue;

	/**List of drivers who have been assigned a schedule, but are carrying only 1 passenger,
	 * so they can potentially serve 1 more request (used by incremental controller)*/
	std::set<const Person *> partiallyAvailableDrivers;
    std::set<const Person *> driversServingSharedReq;

	/** Item being performed by each shared driver */
	std::map<const Person *, ScheduleItem> currentReq;

	/** Keeps track of current local tick */
	unsigned int localTick = 0;

	/** Keeps track of how often to process messages. The messages will be process at every
	 * scheduleComputationPeriod frame ticks*/
	unsigned int scheduleComputationPeriod;

    unsigned int toleratedExtraTime;

    unsigned int maxWaitingTime;
	std::string tripSupportMode;

	bool studyAreaEnabledController;

	//jo
	/** Keeps track of when to rebalance */
	unsigned int nextRebalancingFrame = 0;
	unsigned int rebalancingInterval = 720;
	Rebalancer* rebalancer;

	/**
	 * Associates to each driver her current schedule. If a driver has nothing to do,
	 * an empty schedule is associated to her
	 */
	std::map<const Person*, Schedule> driverSchedules;

	//TODO: These should not be hardcoded
	const double additionalDelayThreshold = std::numeric_limits<double>::max();
	const double waitingTimeThreshold = std::numeric_limits<double>::max();

	//Number of passengers (the driver is not considered in this number)
	//TODO: it should be vehicle based
	const unsigned maxVehicleOccupancy = 2;

	TT_EstimateType ttEstimateType;

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

	/**
	 * Sends the schedule to the driver
	 * @param driver The person to whom the schedule is to be sent
	 * @param schedule The computed schedule that is to be sent
	 * @param isUpdatedSchedule indicates whether the schedule is an update to a previously sent schedule
	 * (this parameter is used by the incremental sharing controller)
	 */
	virtual void assignSchedule(const Person* driver, const Schedule& schedule, bool isUpdatedSchedule = false);

	/**
	 * Looks at the beginning of the schedules and deletes all items that are performed at the same
	 * node as the current item.
	 * @param driver Driver who needs to be sent the schedule
	 * @param schedule The schedule that is scanned to remove items
	 * @param currItem Current item in schedule
	 */
	void checkItemsAhead(const Person* driver, Schedule& schedule, const ScheduleItem currItem);

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
	                               const Group<TripRequestMessage>& additionalRequests, Schedule& newSchedule,
	                               bool isOptimalityRequired) const;

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
	virtual double evaluateSchedule(const Node* initialPositon, const Schedule& schedule,
	                                double additionalDelayThreshold, double waitingTimeThreshold) const;

	/**
	 * True if it is possible to combine these two requests together while respecting the constraints
	 */
	virtual bool canBeShared(const TripRequestMessage& r1, const TripRequestMessage& r2,
	                         double additionalDelayThreshold, double waitingTimeThreshold ) const;

	/**
	 * Inherited from base class to update this agent
	 */
	Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Subscribes a vehicle driver to the controller
	 * @param person Driver to be added
	 */
	virtual void subscribeDriver(Person* person);

	/**
	 * Makes a vehicle driver available to the controller
	 * @param person Driver to be added
	 */
	virtual void driverAvailable(const Person* person);

	/**
	 * Marks the schedule assigned to the driver as complete
	 * (by simply replacing it with an empty schedule)
	 * @param person the driver who has completed the scheule
	 */
	virtual void onDriverShiftEnd(Person *person);

	/**
	 * Updates the controller's copy of the driver schedule
	 * @param person the driver
	 */
	virtual void onDriverScheduleStatus(Person *driver);

	/**
	 * Updates the controller's copy to match the driver's copy.
	 * @param person driver who has sent the sync msg
	 * @param schedule schedule of the respective driver, which needs to
	 * be stored on controller's side as well
	 */
	virtual void onDriverSyncSchedule(Person *person, Schedule schedule);

	void assignSchedules(const std::unordered_map<const Person*, Schedule>& schedulesToAssign,
				bool isUpdatedSchedule = false);

#ifndef NDEBUG
	bool isComputingSchedules; //true during computing schedules. Used for debug purposes
	void consistencyChecks(const std::string& label) const;
#endif
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
