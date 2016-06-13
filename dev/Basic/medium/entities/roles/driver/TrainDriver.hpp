/*
 * TrainDriver.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#pragma once
#include <atomic>
#include "entities/Person_MT.hpp"
//#include "shared/entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "TrainUpdateParams.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/waitTrainActivity/WaitTrainActivity.hpp"
#include "behavioral/ServiceController.hpp"
#include "entities/incident/IncidentManager.hpp"
namespace sim_mob{
namespace medium{
class TrainBehavior;
class TrainMovement;


class TrainDriver : public sim_mob::Role<Person_MT>, public UpdateWrapper<TrainUpdateParams> {
public:
	enum TRAIN_NEXTREQUESTED{
		NO_REQUESTED=0,
		REQUESTED_AT_PLATFORM,
		REQUESTED_WAITING_LEAVING,
		REQUESTED_LEAVING_PLATFORM,
		REQUESTED_TO_PLATFROM,
		REQUESTED_TO_DEPOT
	};
	virtual ~TrainDriver();

	TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior = nullptr,
		sim_mob::medium::TrainMovement* movement = nullptr,
		std::string roleName = std::string(),
		Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TRAINDRIVER);

	virtual Role<Person_MT>* clone(Person_MT *parent) const;
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	void setNextDriver(const TrainDriver* driver);
	const TrainDriver* getNextDriver() const;
	/**
	 * leave from current platform
	 */
	void leaveFromCurrentPlatform();
	/**
	 * get next requested
	 * @return next requested in the train
	 */
	TRAIN_NEXTREQUESTED getNextRequested() const;
	/**
	 * set next requested
	 */
	void setNextRequested(TRAIN_NEXTREQUESTED res);
	/**
	 * get waiting time when parking at platform
	 * @return current waiting time
	 */
	double getWaitingTime() const;
	/**
	 * reduce current waiting time
	 * @val is the value to reduce waiting time
	 */
	void reduceWaitingTime(double val);
	/**
	 * set waiting time
	 * @val is the value to be set
	 */
	void setWaitingTime(double val);
	/**
	 * computing dwell time
	 * @param boardingNum is the boarding number of passengers
	 * @param alightingNum is alighting number of passengers
	 * @return dwell time in seconds
	 */
	double calculateDwellTime(int boardingNum,int alightingNum);
	/**
	 * get train line id
	 * @return line id
	 */
	std::string getTrainLine() const;
	/**
	 * get trip id
	 * @return trip id;
	 */
	int getTripId() const;
	/**
	 * get next platform
	 * @return next platform
	 */
	Platform* getNextPlatform() const;
	/**
	 * get current passengers inside
	 * @retutn passenger list
	 */
	std::list<Passenger*>& getPassengers();
	/**
	 * check whether the train is full
	 * @return the number for empty occupation
	 */
	unsigned int getEmptyOccupation();
	/**
	 * passenger alighting
	 * @param alightingPassenger is the list of alighting person
	 */
	int alightPassenger(std::list<Passenger*>& alightingPassenger,timeslice now);
	/**
	 * passenger boarding
	 * @param boardingPassenger is the list of boarding person
	 */
	int boardPassenger(std::list<WaitTrainActivity*>& boardingPassenger,timeslice now);
	/**
	 * update passengers inside the train
	 */
	void updatePassengers();


	int AlightAllPassengers(std::list<Passenger*>& alightingPassenger);

	void TeleportToOppositeLine(std::string station,std::string lineId);

	void SetTrainDriverInOpposite(TrainDriver *trainDriver);
	TrainDriver *GetDriverInOppositeLine();

	/**
	 * alight all passengers
	 * @param alightingPassenger is the list of alighting person
	 * @param now is current time
	 * @return the number of alighting persons
	 */
	int AlightAllPassengers(std::list<Passenger*>& alightingPassenger,timeslice now);


	TrainMovement * GetMovement();
	/**
	 * store waiting time
	 * @param waitingActivity is pointer to the waiting people
	 * @param now is current time
	 */
	void storeWaitingTime(WaitTrainActivity* waitingActivity,timeslice now) const;
	/**
	 * store arrival time
	 * @param currentTime is current time
	 * @param waitTime is waiting time at current platform
	 */
	void storeArrivalTime(const std::string& currentTime, const std::string& waitTime);
	/* to get traiId*/
	int getTrainId() const;
	/**
	 * Event handler which provides a chance to handle event transfered from parent agent.
	 * @param sender pointer for the event producer.
	 * @param id event identifier.
	 * @param args event arguments.
	 */
	virtual void onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);


private:
	/**get next train driver*/
	const TrainDriver* nextDriver;
	/**next requested*/
	TRAIN_NEXTREQUESTED nextRequested;
	/**current waiting time*/
	double waitingTimeSec;
	double initialDwellTime;
	/**passengers list*/
	std::list<Passenger*> passengerList;
	/**the locker for this driver*/
	mutable boost::mutex driverMutex;
	/**sequence no for platforms*/
	unsigned int platSequenceNumber;
	TrainDriver *nextDriverInOppLine;
	bool holdTrain=false;
	static int counter;

	/**recording disruption information*/
	boost::shared_ptr<DisruptionParams> disruptionParam;

private:
	friend class TrainBehavior;
	friend class TrainMovement;
};
}
} /* namespace sim_mob */


