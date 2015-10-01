//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>

#include "entities/Agent.hpp"
#include "geospatial/MultiNode.hpp"
#include "boost/thread/shared_mutex.hpp"


namespace sim_mob {
class Person;
class RoadSegment;
class SegmentStats;
class Worker;

namespace aimsun
{
//Forward declaration
class Loader;
}

enum {
	MSG_PEDESTRIAN_TRANSFER_REQUEST = 5000000,
	MSG_INSERT_INCIDENT,
	MSG_WAITING_PERSON_ARRIVAL_AT_BUSSTOP,
	MSG_MRT_PASSENGER_TELEPORTATION,
	MSG_WAKEUP_CAR_PASSENGER_TELEPORTATION,
	MSG_WAKE_UP,
	MSG_WARN_INCIDENT
};

/**
 * Message to wrap a Person
 */
class PersonMessage : public messaging::Message {
public:
	PersonMessage(Person* inPerson):person(inPerson){;}
	virtual ~PersonMessage() {}
	Person* person;
};

/**
 * Subclasses message, This is to allow it to function as an message callback parameter.
 */
class InsertIncidentMessage : public messaging::Message {
public:
	InsertIncidentMessage(const std::vector<sim_mob::SegmentStats*>& stats, double newFlowRate);
	virtual ~InsertIncidentMessage();
	const std::vector<sim_mob::SegmentStats*>& stats;
	double newFlowRate;
};

/**
 * Subclass wraps a bus stop into message so as to make alighting decision.
 * This is to allow it to function as an message callback parameter.
 */
class ArrivalAtStopMessage : public messaging::Message {
public:
	ArrivalAtStopMessage(Person* person):waitingPerson(person){;}
	virtual ~ArrivalAtStopMessage() {}
	Person* waitingPerson;
};

struct cmp_person_remainingTimeThisTick : public std::greater<Person*> {
  bool operator() (const Person* x, const Person* y) const;
};

/**
 * Sort all persons in non-increasing order of remaining times in the current tick
 * @param personList list of persons to be sorted
 */
void sortPersons_DecreasingRemTime(std::deque<Person*>& personList);

/**
 * Class representing a grouping of an intersection along with the links which
 * are directly upstream to that intersection.
 *
 * \author Harish Loganathan
 */
class Conflux : public sim_mob::Agent {

	friend class sim_mob::aimsun::Loader;

private:
	//typedefs
	typedef std::deque<sim_mob::Person*> PersonList;
	typedef std::vector<sim_mob::SegmentStats*> SegmentStatsList;
	typedef std::map<sim_mob::Link*, const SegmentStatsList> UpstreamSegmentStatsMap;
	typedef std::map<sim_mob::Link*, PersonList> VirtualQueueMap;
	typedef std::map<const sim_mob::RoadSegment*, SegmentStatsList> SegmentStatsMap;

	/**
	 * helper to capture the status of a person before and after update
	 */
    struct PersonProps
    {
    public:
    	const sim_mob::RoadSegment* segment;
    	const sim_mob::Lane* lane;
    	bool isQueuing;
    	bool isMoving;
    	unsigned int roleType;
    	double vehicleLength;
    	sim_mob::SegmentStats* segStats;
    	const sim_mob::Conflux* conflux;

    	PersonProps(const sim_mob::Person* person, const sim_mob::Conflux* conflux);
    	void printProps(unsigned int personId, uint32_t frame, std::string prefix) const;
    };

	/**
	 *  MultiNode (intersection) around which this conflux is constructed
	 */
	const sim_mob::MultiNode* multiNode;

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
	 * data structure to hold a pointer to a road segment on each link to
	 * keep track of the current segment that is being processed.
	 */
	std::map<sim_mob::Link*, sim_mob::SegmentStats*> currSegsOnUpLinks;

	/**
	 * segments on downstream links
	 * These links conceptually belong to the adjacent confluxes.
	 */
	std::set<const sim_mob::RoadSegment*> downstreamSegments;

	/**
	 *  Map which stores the list of SegmentStats for all road segments on upstream links
	 *  The Segment stats in-turn contain LaneStats which contain the persons.
	 */
	SegmentStatsMap segmentAgents;

	/**
	 *  Worker to which this conflux belongs to.
	 *  Note: this
	 */
	sim_mob::Worker* parentWorker;

	/**
	 * structure to store the frontal agents in each road segment
	 */
	std::map<sim_mob::SegmentStats*, sim_mob::Person* > candidateAgents;

	/**
	 * mutex to protect virtual queues
	 */
	boost::recursive_mutex mutexOfVirtualQueue;

	/**
	 * For each downstream link, this map stores the number of persons that can
	 * be accepted by that link from this conflux in the current tick
	 */
	std::map<sim_mob::Link*, unsigned int> vqBounds;

	/**holds the current frame number for which this conflux is being processed*/
	timeslice currFrame;

	/**list of persons performing activities within the vicinity of this conflux*/
	PersonList activityPerformers;

	/**list of persons with pedestrian role performing walking activities*/
	PersonList pedestrianList;

	/**list of persons currently on MRT train bound to some node in this conflux*/
	PersonList mrt;

	/**list of persons currently on Car Sharing in this condflux*/
	PersonList carSharing;

	/**time in seconds of a single tick*/
	const double tickTimeInS;

	/**
	 * updates agents in this conflux
	 */
	void processAgents();

	/**
	 * moves the person and does housekeeping for the conflux
	 * @param person the person to move
	 * */
	void updateAgent(sim_mob::Person* person);

	/**
	 * calls frame_tick() of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to move
	 */
	UpdateStatus movePerson(timeslice now, Person* person);

	/**
	 * calls frame_tick() for bus stop agent
	 */
	void updateBusStopAgents();

	/**
	 * assign a waiting person to bus stop agent
	 * @param person is with the role "waiting bus activity"
	 */
	void assignPersonToBusStopAgent(Person* person);

	/**
	 * assign person to MRT
	 * @param person is going to board MRT
	 */
	void assignPersonToMRT(Person* person);

	/**
	 * assign person to car
	 * @param person is going to board car
	 */
	void assignPersonToCar(Person* person);

	/**
	 * calls frame_init of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to initialize
	 * @return true if the role corresponding to this subtrip has been constructed successfully; false otherwise
	 */
	bool callMovementFrameInit(timeslice now, Person* person);

	/**
	 * calls frame_tick of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to tick
	 * @return update status
	 */
	Entity::UpdateStatus callMovementFrameTick(timeslice now, Person* person);

	/**
	 * calls frame_tick of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person whose frame output is required
	 */
	void callMovementFrameOutput(timeslice now, Person* person);

	/** sets the iterators on currSegToUpLinks to the segments at the end of the links*/
	void resetCurrSegsOnUpLinks();

	/**
	 * removes the agent from the conflux and marks it for removal by the worker.
	 * The person gets removed from the simulation at the end of the current tick.
	 * @param ag the person to be removed
	 * @param beforeUpdate person properties before update
	 */
	void killAgent(sim_mob::Person* person, PersonProps& beforeUpdate);

	/**
bool sim_mob::insertIncidentS(const std::string fileName){

	ifstream in(fileName.c_str());
	if (!in.is_open()){
		ostringstream out("");
		out << "File " << fileName << " not found";
		throw runtime_error(out.str());
		//return false;
	}
	sim_mob::StreetDirectory & stDir = sim_mob::StreetDirectory::instance();
	typedef tokenizer< escaped_list_separator<char> > Tokenizer;
	vector< string > record;
	string line;

	while (getline(in,line))
	{
		Tokenizer record(line);
		unsigned int sectionId = lexical_cast<unsigned int>(*(record.begin()));//first element
		double newFlowRate = lexical_cast<double>(*(record.end()));//second element
		const sim_mob::RoadSegment* rs = stDir.getRoadSegment(sectionId);
		const std::vector<sim_mob::SegmentStats*>& stats = rs->getParentConflux()->findSegStats(rs);
		sim_mob::SegmentStats* ss;
		BOOST_FOREACH(ss,stats){
			sim_mob::Conflux::insertIncident(ss,newFlowRate);
		}
	}
	return true;
}
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
	void housekeep(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person* person);

	/**
	 * Gets the person to switch to the next trip chain item
	 * @param person the person to switch
	 * @return Entity::UpdateStatus update status
	 */
	Entity::UpdateStatus switchTripChainItem(Person* person);

	/**
	 * gets the context of the agents right if the agent has moved out of this conflux
	 * @param beforeUpdate person properties before update
	 * @param afterUpdate person properties after update
	 */
	void updateAgentContext(PersonProps& beforeUpdate, PersonProps& afterUpdate, Person* person) const;

protected:
	/**
	 * Function to initialize the conflux before its first update.
	 * frame_init() of the Agent is overridden to register the conflux as a
	 * message handler. This function is also ideal for registering message
	 * handlers of all the bus stops which (permanently)  belong to segment
	 * stats of this conflux.
	 *
	 * @param now the frame number in which the function is called
	 * @return true if initialization was successful; false otherwise.
	 */
	virtual bool frame_init(timeslice now);

	virtual Entity::UpdateStatus frame_tick(timeslice now) { throw std::runtime_error("frame_tick() is not required and are not implemented for Confluxes."); }
	virtual void frame_output(timeslice now) { throw std::runtime_error("frame_output methods are not required and are not implemented for Confluxes."); }


	//Inherited from Agent.
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

	//Inherited from Agent.
	 virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);

public:
	Conflux(sim_mob::MultiNode* multinode, const MutexStrategy& mtxStrat, int id=-1);
	virtual ~Conflux() ;

	//Confluxes are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

	virtual std::vector<BufferedBase *> buildSubscriptionList();

	// functions from agent
	virtual void load(const std::map<std::string, std::string>&) {}
	virtual Entity::UpdateStatus update(timeslice frameNumber);

	// Getters
	const sim_mob::MultiNode* getMultiNode() const {
		return multiNode;
	}

	std::set<const sim_mob::RoadSegment*> getDownstreamSegments() {
		return downstreamSegments;
	}

	sim_mob::Worker* getParentWorker() const {
		return parentWorker;
	}

	void setParentWorker(sim_mob::Worker* parentWorker) {
		this->parentWorker = parentWorker;
	}

	/**
	 * initializes the conflux
	 * @param now timeslice when initialize is called
	 */
	void initialize(const timeslice& now);

	bool hasSpaceInVirtualQueue(sim_mob::Link* lnk);
	void pushBackOntoVirtualQueue(sim_mob::Link* lnk, sim_mob::Person* p);

	/**
	 * adds a person into this conflux
	 * @param ag person to be added
	 * @param rdSeg starting road segment of ag
	 */
	void addAgent(sim_mob::Person* ag);

	/**
	 * Searches upstream segments to get the segmentStats for the requested road segment
	 * @param rdSeg road segment corresponding to the stats to be found
	 * @param statsNum position of the requested stats in the segment
	 * @return segment stats
	 */
	sim_mob::SegmentStats* findSegStats(const sim_mob::RoadSegment* rdSeg, uint16_t statsNum);

	/**
	 * returns the list of segment stats corresponding to a road segment
	 * @param rdSeg segment for which the stats list is required
	 * @return constant list of segment stats corresponding to this segment
	 */
	const std::vector<sim_mob::SegmentStats*>& findSegStats(const sim_mob::RoadSegment* rdSeg);

	/**
	 * supply params related functions
	 */
	double getSegmentSpeed(SegmentStats* segStats, bool hasVehicle) const;

	void resetPositionOfLastUpdatedAgentOnLanes();
	void incrementSegmentFlow(const RoadSegment* rdSeg, uint16_t statsNum);
	void resetSegmentFlows();

	/** updates lane params for all lanes within the conflux */
	void updateAndReportSupplyStats(timeslice frameNumber);

	/**process persons in the virtual queue*/
	void processVirtualQueues();

	//TODO: To be removed after debugging.
	std::stringstream debugMsgs;

	//=======link travel time computation for current frame tick =================
	struct LinkTravelTimes
	{
	public:
		double linkTravelTime_;
		unsigned int agCnt;

		LinkTravelTimes(double linkTravelTime, unsigned int agentCount)
		: linkTravelTime_(linkTravelTime), agCnt(agentCount) {}
	};

	std::map<const Link*, LinkTravelTimes> LinkTravelTimesMap;
	void setLinkTravelTimes(Person* ag, double linkExitTime, const Link* link);
	void resetLinkTravelTimes(timeslice frameNumber);
	void reportLinkTravelTimes(timeslice frameNumber);

	//================ end of road segment travel time computation ========================

	/**
	 * update the number of persons that can be added to the downstream confluxes
	 * from this conflux
	 */
	unsigned int resetOutputBounds();

	/**
	 * get a list of all persons in this conflux
	 */
	std::deque<sim_mob::Person*> getAllPersons();

	/**
	 * counts the number of persons active in this conflux
	 * @return number of persons in this conflux
	 */
	unsigned int countPersons();

	/**
	 * get an ordered list of all persons in this conflux
	 * @param mergedPersonDeque output list that must contain the merged list of persons
	 */
	void getAllPersonsUsingTopCMerge(std::deque<sim_mob::Person*>& mergedPersonDeque);

	/**
	 * merges the ordered list of persons on each link of the conflux into 1
	 * @param mergedPersonDeque output list that must contain the merged list of persons
	 * @param allPersonLists list of list of persons to merge
	 * @param capacity capacity till which the relative ordering of persons is important
	 */
	void topCMergeDifferentLinksInConflux(std::deque<sim_mob::Person*>& mergedPersonDeque,
			std::vector< std::deque<sim_mob::Person*> >& allPersonLists, int capacity);

	/**
	 * get number of persons in lane infinities of this conflux
	 */
	unsigned int getNumRemainingInLaneInfinity();

//	void reportRdSegTravelTimes(timeslice frameNumber);

	/**
	 * determines if this conflux is connected to any conflux that belongs to
	 * another worker; also determines if all connected confluxes have the same
	 * worker or not
	 */
	void findBoundaryConfluxes();

	/**
	 * given a person with a trip chain, create path for his first trip and
	 * return his starting conflux.
	 *
	 * @param person person for whom the starting conflux is needed
	 * @param currentTime the current time (in ms) in which the function is called
	 * @return pointer to the starting conflux of the person's constructed path
	 */
	static sim_mob::Conflux* findStartingConflux(Person* person, unsigned int currentTime);

	/**
	 * Inserts an Incident by updating the flow rate for all lanes of a road segment to a new value.
	 *
	 * @param rdSeg roadSegment to insert incident
	 * @param newFlowRate new flow rate to be updated
	 */
	static void insertIncident(sim_mob::SegmentStats* segStats, const double & newFlowRate);
	///Same as above. Just, single road segment can have 'multiple' SegmentStats
	static void insertIncident(const std::vector<sim_mob::SegmentStats*>  &segStats, const double & newFlowRate);

	/**
	 * Removes a previously inserted incident by restoring the flow rate of each lane of a road segment to normal values
	 *
	 * @param segStats road segment stats to remove incident
	 */
	static void removeIncident(sim_mob::SegmentStats* segStats);

	/**
	 * collect current person travel time
	 */
	void collectTravelTime(Person* person);

	bool isBoundary; //A conflux that receives person from at least one conflux that belongs to another worker
	bool isMultipleReceiver; //A conflux that receives persons from confluxes that belong to multiple other workers
};

} /* namespace sim_mob */

