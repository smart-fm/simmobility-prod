/*
 * TrainStationAgent.hpp
 *
 *  Created on: Feb 24, 2016
 *      Author: zhang huai peng
 */

#ifndef TRAINSTATIONAGENT_HPP_
#define TRAINSTATIONAGENT_HPP_
#include "entities/Agent.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/waitTrainActivity/WaitTrainActivity.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "entities/incident/IncidentManager.hpp"
namespace sim_mob
{
namespace medium
{

struct ForceReleaseEntity
{
	int trainId;
	std::string lineId;
};
class TrainDriver;
class Conflux;
class TrainStationAgent : public sim_mob::Agent {
public:
	TrainStationAgent();
	virtual ~TrainStationAgent();

	/**
	 * This interface sets the station for the train station agent
	 * @param station is the pointer to the station
	 */
	void setStation(const Station* station);

	/**
	 * This interface sets the conflux for the train station agent
	 * @param conflux is the pointer to the conflux
	 */
	void setConflux(Conflux* conflux);

	/**
	 * This interface sets the station name for the train station agent
	 * @param name is the reference name of the station
	 */
	void setStationName(const std::string& name);

	/**
	 * This interface sets the lines for the train station agent
	 */
	void setLines();

	/**
	 * This function sets the last train driver arrived at the station for a particular line
	 * @param lineId is the id of the line in whose station the driver has arrived
	 * @param driver is the pointer to the train driver which arrived last at the station
	 */
	void setLastDriver(std::string lineId,TrainDriver *driver);

	/**
	 * This function adds the train driver to the train station agent so that its movement frame tick
	 * is called by it
	 * @param driver is the pointer to the train driver to be added to the train station agent
	 */
	void addTrainDriverInToStationAgent(TrainDriver * driver);

	/**
	 * This function gets the list of trains from train statin agent
	 * @return list of trains from train station agent
	 */
	std::list<TrainDriver*>& getTrains();

	/**
	 * This function gets the last train driver
	 * @return the map of last train drivers with their corresponding lines
	 */
	std::map<std::string, TrainDriver*> getLastDriver();

protected:
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);
	//Virtual overrides
	virtual Entity::UpdateStatus frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);
	virtual bool isNonspatial();
	/**
	 * calls frame_tick of the movement facet for the person's role
	 * @param now current time slice
	 * @param person person to tick
	 * @return update status
	 */
	Entity::UpdateStatus callMovementFrameTick(timeslice now, TrainDriver* person);
	/**
	 * Inherited from EventListener.
	 * @param eventId
	 * @param ctxId
	 * @param sender
	 * @param args
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);

private:
	/**
	 * dispatch pending train
	 * @param now current time slice
	 */
	void dispathPendingTrains(timeslice now);

	/**
	 * remove ahead train
	 * @param aheadTrain is pointer to the ahead train
	 */
	void removeAheadTrain(TrainDriver* aheadDriver);

	/**
	 * alighting passenger leave platform
	 * @param now current time slice
	 */
	void passengerLeaving(timeslice now);

	/**
	 * update wait persons
	 */
	void updateWaitPersons();

	/**
	 * trigger reroute event
	 * @param args is event parameter
	 * @param now current time slice
	 */
	void triggerRerouting(const event::EventArgs& args, timeslice now);

	/**
	 * perform disruption processing
	 * @param now is current time
	 */
	void performDisruption(timeslice now);

	/**
	 * This interface checks and inserts any unscheduled trains in all the platforms of all the lines in the station
	 */
	void checkAndInsertUnscheduledTrains();

	/**
	 * This interface sets the route and platforms for opposite line for Uturn
	 */
	void prepareForUturn(TrainDriver *driver);

	/**
	 * calculate walking time
	 */
	double calculateWalkingTime();

	/**
	 * collect passengers' exit time
	 */
	void collectExitTime(std::list<Passenger*>& exitPassenger);

	/**
	 * This interface pushes the force alighted passengers into waiting queue of the corresponding platforms
	 * depending upon whether they alighted before their alighting station or after they alighting station.
	 * If they alighted before their alighting station then they are pushed into waiting queue of they same platform they alighted.
	 * If they alighted after their alighting station then they are pushed into opposite platform queue so that they can take the train in
	 * opposite direction
	 * @param platform is the pointer to the platform where they alighted from the train
	 */
	void pushForceAlightedPassengersToWaitingQueue(const Platform *platform);

private:
	/**record the walking time parameters at current station*/
	const WalkingTimeParams* walkingTimeParams = nullptr;
	/**the reference to the station*/
	const Station* station;
	/**the reference */
	std::list<TrainDriver*> trainDriver;
	/**record last train in each line*/
	std::map<std::string, TrainDriver*> lastTrainDriver;
	/**record pending trains in each line*/
	std::map<std::string, std::list<TrainDriver*>> pendingTrainDriver;
	/**record pending unscheduled trains in each line*/
	std::map<std::string, std::list<TrainDriver*>> pendingUnscheduledTrainDriver;
	/**record usage in each line*/
	std::map<std::string, bool> lastUsage;
	/**waiting person for boarding*/
	std::map<const Platform*, std::list<WaitTrainActivity*>> waitingPersons;
	std::map<const Platform*, std::list<WaitTrainActivity*>> entryPersons;
	std::map<const Platform*, std::list<Passenger*>> forceAlightedPersons;
	std::list<Person_MT*> personsForcedAlighted;
	std::vector<std::string> unscheduledTrainLines;
	/**alighting person for next trip*/
	std::map<const Platform*, std::list<Passenger*>> leavingPersons;
	/**alighting person to exit from platform */
	std::map<const Platform*, std::list<Passenger*>> exitPersons;
	/** parent conflux */
	Conflux* parentConflux;
	/**recording disruption information*/
	boost::shared_ptr<DisruptionParams> disruptionParam;
	/**station name*/
	std::string stationName;
	std::map<std::string,bool> IsStartStation;
	bool arePassengersreRouted = false;
	static boost::mutex insertTrainOrUturnlock;
};
}
} /* namespace sim_mob */

#endif /* TRAINSTATIONAGENT_HPP_ */
