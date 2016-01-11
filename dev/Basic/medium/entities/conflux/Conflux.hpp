//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <deque>
#include <map>
#include <vector>
#include "entities/Agent.hpp"
#include "entities/Person_MT.hpp"
#include "geospatial/network/Node.hpp"
#include "message/MT_Message.hpp"
#include "SegmentStats.hpp"

namespace sim_mob
{
class RoadSegment;
class Worker;

namespace medium
{

struct GreaterRemainingTimeThisTick: public std::greater<Person_MT*>
{
	bool operator()(const Person_MT* x, const Person_MT* y) const;
};

/**
 * Sort all persons in non-increasing order of remaining times in the current tick
 * @param personList list of persons to be sorted
 */
void sortPersonsDecreasingRemTime(std::deque<Person_MT*>& personList);

/**
 * simple class to contain role-wise and total count of persons
 */
struct PersonCount
{
public:
	unsigned int pedestrians;
	unsigned int busPassengers;
	unsigned int trainPassengers;
	unsigned int carDrivers;
	unsigned int carSharers;
	unsigned int motorCyclists;
	unsigned int busDrivers;
	unsigned int busWaiters;
	unsigned int activityPerformers;

	PersonCount();
	const PersonCount& operator+=(const PersonCount& dailytime);
	unsigned int getTotal();
};

/**
 * Class to represent a grouping of an intersection along with the links which
 * are directly upstream to that intersection.
 *
 * \author Harish Loganathan
 */
class Conflux : public Agent
{
private:
	//typedefs
	typedef std::deque<Person_MT*> PersonList;
	typedef std::vector<SegmentStats*> SegmentStatsList;
	typedef std::map<const Link*, const SegmentStatsList> UpstreamSegmentStatsMap;
	typedef std::map<const Link*, PersonList> VirtualQueueMap;
	typedef std::map<const RoadSegment*, SegmentStatsList> SegmentStatsMap;

	/**
	 * helper to capture the status of a person before and after update
	 */
	struct PersonProps
	{
	public:
		const RoadSegment* segment;
		const Lane* lane;
		bool isQueuing;
		bool isMoving;
		unsigned int roleType;
		double vehicleLength;
		SegmentStats* segStats;
		const Conflux* conflux;
		double distanceToSegEnd;

		PersonProps(const Person_MT* person, const Conflux* conflux);
		void printProps(std::string personId, uint32_t frame, std::string prefix) const;
	};

	/**
	 *  MultiNode (intersection) around which this conflux is constructed
	 */
	const Node* confluxNode;

	/**
	 * Link-wise list of road segments in this conflux
	 * stores std::map<link upstream to multiNode, segment stats on link>
	 */
	UpstreamSegmentStatsMap upstreamSegStatsMap;

	/**
	 * virtual queues are used to hold persons who want to move in from adjacent
	 * confluxes when this conflux is not processed for the current tick yet.
	 * Each link in the conflux has 1 virtual queue.
	 */
	VirtualQueueMap virtualQueuesMap;

	/**
	 * set of confluxes that own links downstream to this Conflux node
	 * This is the set of adjacent confluxes to this conflux
	 */
	std::set<Conflux*> connectedConfluxes;

	/**
	 *  Map which stores the list of SegmentStats for all road segments on upstream links
	 *  The Segment stats in-turn contain LaneStats which contain the persons.
	 */
	SegmentStatsMap segmentAgents;

	/**
	 *  flag to indicate whether this conflux belongs to some worker
	 *  Note: this is used only during the initial assignment of confluxes to workers
	 */
	bool parentWorkerAssigned;

	/**
	 * mutex to protect virtual queues
	 */
	boost::recursive_mutex mutexOfVirtualQueue;

	/**
	 * For each downstream link, this map stores the number of persons that can
	 * be accepted by that link from this conflux in the current tick
	 */
	std::map<const Link*, unsigned int> vqBounds;

	/**holds the current frame number for which this conflux is being processed*/
	timeslice currFrame;

	/**list of persons performing activities within the vicinity of this conflux*/
	PersonList activityPerformers;

	/**list of persons with pedestrian role performing walking activities*/
	PersonList pedestrianList;

	/**list of persons currently on MRT train bound to some node in this conflux*/
	PersonList mrt;

	/**
	 * list of persons who are hidden in this conflux awaiting a wake-up call
	 * All persons whose roles resolve to teleportation (e.g. car sharing and private bus) are kept in this list.
	 */
	PersonList stashedPersons;

	/**flag to indicate whether this conflux is a person loading conflux*/
	bool isLoader;

	/**list of persons who are about to get into the simulation in the next tick*/
	PersonList loadingQueue;
        
	/**interval of output updates*/
	static uint32_t updateInterval;

	/**time in seconds of a single tick*/
	const double tickTimeInS;

	/**
	 * number of times update function was called for this conflux in current tick
	 */
	unsigned int numUpdatesThisTick;

	/**
	 * updates agents in this conflux
	 */
	void processAgents();

	/**
	 * update agent in infinite lanes
	 */
	void processInfiniteAgents();

	/**
	 * loads newly starting persons and dispatches them to the correct starting conflux.
	 */
	void loadPersons();

	/**
	 * moves the person and does housekeeping for the conflux
	 * @param person the person to move
	 */
	void updateAgent(Person_MT* person);

	/**
	 * calls frame_tick() of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to move
	 */
	UpdateStatus movePerson(timeslice now, Person_MT* person);

	/**
	 * calls frame_tick() for bus stop agent
	 */
	void updateBusStopAgents();

	/**
	 * assign a waiting person to bus stop agent
	 * @param person is with the role "waiting bus activity"
	 */
	void assignPersonToBusStopAgent(Person_MT* person);

	/**
	 * assign person to MRT
	 * @param person who wants to board MRT
	 */
	void assignPersonToMRT(Person_MT* person);

	/**
	 * assign person to pedestrian list
	 * @param person with pedestrian role
	 */
	void assignPersonToPedestrianlist(Person_MT* person);

	/**
	 * assign person to car
	 * @param person is going to board car
	 */
	void stashPerson(Person_MT* person);

	/**
	 * calls frame_init of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to initialize
	 * @return true if the role corresponding to this subtrip has been constructed successfully; false otherwise
	 */
	bool callMovementFrameInit(timeslice now, Person_MT* person);

	/**
	 * calls frame_tick of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to tick
	 * @return update status
	 */
	Entity::UpdateStatus callMovementFrameTick(timeslice now, Person_MT* person);

	/**
	 * calls frame_tick of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person whose frame output is required
	 */
	void callMovementFrameOutput(timeslice now, Person_MT* person);

	/**
	 * removes agent from conflux and kills the agent.
	 * @param ag the person to be removed
	 * @param beforeUpdate person properties before update
	 */
	void killAgent(Person_MT* person, PersonProps& beforeUpdate);

/*	bool insertIncidentS(const std::string fileName)
	{
		ifstream in(fileName.c_str());
		if (!in.is_open())
		{
			ostringstream out("");
			out << "File " << fileName << " not found";
			throw runtime_error(out.str());
			//return false;
		}
		StreetDirectory & stDir = StreetDirectory::instance();
		typedef tokenizer<escaped_list_separator<char> > Tokenizer;
		vector < string > record;
		string line;

		while (getline(in, line))
		{
			Tokenizer record(line);
			unsigned int sectionId = lexical_cast<unsigned int>(*(record.begin())); //first element
			double newFlowRate = lexical_cast<double>(*(record.end())); //second element
			const RoadSegment* rs = stDir.getRoadSegment(sectionId);
			const std::vector<SegmentStats*>& stats = rs->getParentConflux()->findSegStats(rs);
			SegmentStats* ss;
			BOOST_FOREACH(ss,stats)
			{
				Conflux::insertIncident(ss, newFlowRate);
			}
		}
		return true;
	}*/

	/**
	 * Resets the remainingTime of persons who remain in
	 * lane infinities and virtual queues across ticks
	 * Note: This may include
	 * 1. newly starting persons who (were supposed to, but) could not get added
	 * to the simulation in the previous tick due to traffic congestion in their
	 * starting segment. (lane infinity)
	 * 2. Persons who got added to and remained virtual queue in the previous tick
	 */
	void resetPersonRemTimes();

	/**
	 * handles house keeping for the conflux based on state change of person after his update
	 * @param beforeUpdate person properties before update
	 * @param afterUpdate person properties after update
	 * @param person the person being handled
	 */
	void housekeep(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person_MT* person);

	/**
	 * handles change of role related house keeping for person
	 * @param beforeUpdate person properties before update
	 * @param afterUpdate person properties after update
	 * @param person the person being handled
	 * @return true if there was a role change; false otherwise
	 */
	bool handleRoleChange(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person_MT* person);

	/**
	 * Gets the person to switch to the next trip chain item
	 * @param person the person to switch
	 * @return Entity::UpdateStatus update status
	 */
	Entity::UpdateStatus switchTripChainItem(Person_MT* person);

	/**
	 * gets the context of the agents right if the agent has moved out of this conflux
	 * @param beforeUpdate person properties before update
	 * @param afterUpdate person properties after update
	 */
	void updateAgentContext(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person_MT* person) const;

protected:
	/**
	 * Function to initialize the conflux before its first update.
	 * frame_init() of the Agent is overridden to register the conflux as a
	 * message handler. This function is also ideal for registering message
	 * handlers of all the bus stops which (permanently)  belong to segment
	 * stats of this conflux.
	 *
	 * @param now the frame number in which the function is called
	 * @return UpdateStatus::Continue if initialization was successful; throws error otherwise.
	 */
	virtual Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * frame_tick overridden from agent class to throw error if called
	 * @param now (description is irrelevant)
	 * @return (description is irrelevant)
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * frame_output overridden from agent class to throw error if called
	 * @param now (description is irrelevant)
	 */
	virtual void frame_output(timeslice now);

public:
	Conflux(Node* confluxNode, const MutexStrategy& mtxStrat, int id=-1, bool isLoader=false);
	virtual ~Conflux() ;

	/** Confluxes are non-spatial in nature. */
	virtual bool isNonspatial();

	virtual void registerChild(Entity* child);

	/**
	 * overriden update function which is called every tick from the workers
	 * @param frameNumber represents current simulation time step
	 */
	virtual Entity::UpdateStatus update(timeslice frameNumber);

	const Node* getConfluxNode() const
	{
		return confluxNode;
	}

	bool hasParentWorker() const
	{
		return parentWorkerAssigned;
	}

	void setParentWorkerAssigned()
	{
		this->parentWorkerAssigned = true;
	}

	std::set<Conflux*>& getConnectedConfluxes()
	{
		return connectedConfluxes;
	}

	/**
	 * initializes the conflux
	 * @param now timeslice when initialize is called
	 */
	void initialize(const timeslice& now);

	/**
	 * checks whether the virtual queue can accommodate a vehicle
	 * @param link the link whose VQ is to be checked
	 * @return true if vq bound is not zero; false otherwise
	 */
	bool hasSpaceInVirtualQueue(const Link* lnk);

	/**
	 * puts person on VQ
	 * @param lnk link of target VQ
	 * @param person person to be added
	 */
	void pushBackOntoVirtualQueue(const Link* lnk, Person_MT* person);

	/**
	 * adds a conflux to the set of connected confluxes
	 * @param conflux Conflux to add
	 */
	void addConnectedConflux(Conflux* conflux);

	/**
	 * adds a person into this conflux
	 * @param person person to be added
	 * @param rdSeg starting road segment of ag
	 */
	void addAgent(Person_MT* person);

	/**
	 * Searches upstream segments to get the segmentStats for the requested road segment
	 * @param rdSeg road segment corresponding to the stats to be found
	 * @param statsNum position of the requested stats in the segment
	 * @return segment stats
	 */
	SegmentStats* findSegStats(const RoadSegment* rdSeg, uint16_t statsNum) const;

	/**
	 * returns the list of segment stats for a road segment
	 * @param rdSeg input road segment
	 * @return const list of segstats for rdSeg
	 */
	const std::vector<SegmentStats*>& findSegStats(const RoadSegment* rdSeg) const;

	/**
	 * gets current speed of segStats
	 * @param segStats seg stats for which speed is requested
	 * @return speed in cm/s
	 */
	double getSegmentSpeed(SegmentStats* segStats) const;

	/**
	 * resets position of last updated agent to -1 for all lanes in all seg stats in this conflux
	 */
	void resetPositionOfLastUpdatedAgentOnLanes();

	/**
	 * increments flow counter in segstats by 1
	 * @param road segment for seg stats
	 * @param statsNum position of stats within segment
	 */
	void incrementSegmentFlow(const RoadSegment* rdSeg, uint16_t statsNum);

	/**
	 * resets flow counter in segstats to 0
	 */
	void resetSegmentFlows();

	/**
	 * updates lane params for all lanes within the conflux
	 * @param frameNumber current time slice
	 */
	void updateAndReportSupplyStats(timeslice frameNumber);

	/** process persons in the virtual queue */
	void processVirtualQueues();

	/**
	 * helper struct to maintain sum of travel times experienced by drivers crossing a link along with the count of those drivers
	 */
	struct LinkTravelTimes
	{
	public:
		double linkTravelTime_;
		unsigned int personCnt;

		LinkTravelTimes(double linkTravelTime, unsigned int agentCount) :
				linkTravelTime_(linkTravelTime), personCnt(agentCount)
		{
		}
	};

	/** map of link->LinkTravelTimes struct */
	std::map<const Link*, LinkTravelTimes> linkTravelTimesMap;

	/**
	 * puts a travel time entry on to LinkTravelTimesMap
	 * @param travelTime experienced travel time for link
	 * @param link the link just traversed by some person
	 */
	void setLinkTravelTimes(double travelTime, const Link* link);

	/**
	 * clears off the LinkTravelTimesMap
	 */
	void resetLinkTravelTimes(timeslice frameNumber);

	/**
	 * outputs average travel times in all links of this conflux for current frame
	 * @param frameNumber current frame
	 */
	void reportLinkTravelTimes(timeslice frameNumber);

	/**
	 * update the number of persons that can be added to the downstream confluxes
	 * from this conflux
	 * @return existing number of agents in virtual queues for output
	 */
	unsigned int resetOutputBounds();

	/**
	 * get a list of all persons in this conflux
	 * @return list of all persons in this conflux including activity performers and pedestrians
	 */
	std::deque<Person_MT*> getAllPersons();

	/**
	 * counts the number of persons active in this conflux
	 * @return struct containing role-wise and total number of persons in this conflux
	 */
	PersonCount countPersons() const;

	/**
	 * get an ordered list of all persons in this conflux
	 * @param mergedPersonDeque output list that must contain the merged list of persons
	 */
	void getAllPersonsUsingTopCMerge(std::deque<Person_MT*>& mergedPersonDeque);

	/**
	 * merges the ordered list of persons on each link of the conflux into 1
	 * @param mergedPersonDeque output list that must contain the merged list of persons
	 * @param allPersonLists list of list of persons to merge
	 * @param capacity capacity till which the relative ordering of persons is important
	 */
	void topCMergeDifferentLinksInConflux(std::deque<Person_MT*>& mergedPersonDeque,
			std::vector< std::deque<Person_MT*> >& allPersonLists, int capacity);

	/**
	 * get number of persons in lane infinities of this conflux
	 */
	unsigned int getNumRemainingInLaneInfinity();

	/**
	 * collect current person travel time
	 */
	void collectTravelTime(Person_MT* person);

	/**
	 * Handles all possible messages that can be dispatched to this Conflux
	 * @param type of the message.
	 * @param message payload received.
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

	/**
	 * given a person with a trip chain, create path for his first trip and
	 * return his starting conflux.
	 *
	 * @param person person for whom the starting conflux is needed
	 * @param currentTime the current time (in ms) in which the function is called
	 * @return pointer to the starting conflux of the person's constructed path
	 */
	static Conflux* findStartingConflux(Person_MT* person, unsigned int currentTime);

	/**
	 * finds the conflux owning a road segment
	 *
	 * @param rdSeg input road segment
	 *
	 * @return Conflux to with rdSeg belongs to
	 */
	static Conflux* getConflux(const RoadSegment* rdSeg);

	/**
	 * Inserts an Incident by updating the flow rate for all lanes of a road segment to a new value.
	 *
	 * @param rdSeg roadSegment to insert incident
	 * @param newFlowRate new flow rate to be updated
	 */
	static void insertIncident(SegmentStats* segStats, double newFlowRate);

	/**
	 * Removes a previously inserted incident by restoring the flow rate of each lane of a road segment to normal values
	 *
	 * @param segStats road segment stats to remove incident
	 */
	static void removeIncident(SegmentStats* segStats);

	/**
	 * constructs confluxes around each multinode
	 * @param rdnw the road network
	 */
	static void CreateConfluxes();

	/**
	 * creates a list of SegmentStats for a given segment depending on the stops
	 * in the segment. The list splitSegmentStats will contain SegmentStats objects
	 * containing bus stops (and quite possibly a last SegmentStats with no bus stop)
	 * @param rdSeg the road segment for which stats must be created
	 * @param splitSegmentStats vector of SegmentStats* to be filled up
	 */
	static void CreateSegmentStats(const RoadSegment* rdSeg, Conflux* conflux, std::list<SegmentStats*>& splitSegmentStats);

	/**
	 * Creates lane groups for every SegmentStats in each link.
	 * Lane groups are elicited based on the lane connections (turnings) of the last segment of the link.
	 */
	static void CreateLaneGroups();

	/**
	 * generate cars statistics on the road for diagnosis
	 * @param now indicate current time
	 */
	void driverStatistics(timeslice now);
};

} // namespace medium
} // namespace sim_mob

