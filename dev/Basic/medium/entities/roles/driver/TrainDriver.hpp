/*
 * TrainDriver.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#pragma once
#include <atomic>
#include "entities/Person_MT.hpp"
#include "entities/roles/Role.hpp"
#include "TrainUpdateParams.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/waitTrainActivity/WaitTrainActivity.hpp"
#include "entities/incident/IncidentManager.hpp"
#include "entities/misc/TrainTrip.hpp"
namespace sim_mob{
namespace medium{
struct StopPointEntity
{
	PolyPoint point;
	double duration;
};

struct PlatformHoldingTimeEntity
{
  std::string pltaformName;
  double holdingTime;

  bool operator ==(const PlatformHoldingTimeEntity &other)
  {
     if(this->pltaformName==other.pltaformName&&this->holdingTime==other.holdingTime)
     {
    	 return true;
     }
     return false;
  }
};

enum passengerMovement
{
	BOARDING=0,
	ALIGHTING,
	BOTH
};
struct RestrictPassengersEntity
{
	std::string platformName;
	passengerMovement type;
};

class TrainBehavior;
class TrainMovement;


class TrainDriver : public sim_mob::Role<Person_MT>, public UpdateWrapper<TrainUpdateParams> {
public:
	enum TRAIN_NEXTREQUESTED{
		NO_REQUESTED=0,
		REQUESTED_AT_PLATFORM,
		REQUESTED_WAITING_LEAVING,
		REQUESTED_LEAVING_PLATFORM,
		REQUESTED_TAKE_UTURN,
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
	void setNextDriver(TrainDriver* driver);
	 TrainDriver* getNextDriver() ; //needs to be checked
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

	void calculateDwellTime(int boarding,int alighting,int noOfPassengerInTrain,timeslice now);

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

	int boardForceAlightedPassengersPassenger(std::list<Passenger*>& forcealightedPassengers,timeslice now);

	/**
	 * update passengers inside the train
	 */
	void updatePassengers();

	/* Force Alights all passengers in the train */
	int AlightAllPassengers(std::list<Passenger*>& alightingPassenger);

	/* stores the next train(ahead train ) in opposite direction before teleport*/
	void SetTrainDriverInOpposite(TrainDriver *trainDriver);
	/* gets the train driver in opposite line to assign it to next driver */
	TrainDriver *GetDriverInOppositeLine();
	const TrainTrip *getTrainTrip() const;


	/**
	 * alight all passengers
	 * @param alightingPassenger is the list of alighting person
	 * @param now is current time
	 * @return the number of alighting persons
	 */
	int AlightAllPassengers(std::list<Passenger*>& alightingPassenger,timeslice now);


	TrainMovement * getMovement();
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
	/**
	 * set arrival time
	 * @param currentTime is current time when train arrive at platform
	 */
	void setArrivalTime(const std::string& currentTime);
	/* to get traiId*/
	int getTrainId() const;

	/* inserts stop point that is the point where train has to be stopped as requested by service controller*/
	void InsertStopPoint(PolyPoint point,double duration);

	/* gets all the stop points of the train */
	std::vector<StopPointEntity>& GetStopPoints();

	/* checks if the train is stopped at the point */
	bool IsStoppedAtPoint();
	/* Sets the parametrs of stop point Entities ,that is the POLypoint and duration of stopping */
	void SetStoppingParameters(PolyPoint point,double duration);

	/* reduces the stopping duration left after every frame tick */
	double ReduceStoppingTime(double secsInframeTick);
	/* sets stopping time of train at stop point */
	void SetStoppingTime(double stoppingTime);
	/* set the stopping status  of train ,that is whether train is stopped ta point */
	void  SetStoppingStatus(bool);
    /* sets flag to terminate train service */

	void clearStopPoints();
	void SetTerminateTrainService(bool terminate);
	/* checks the staus whether the train is to be terminated or not */
	bool GetTerminateStatus();
	/* inserts the instances when the train is supposed to be held by platform as requested by service controller */

	void InsertPlatformHoldEntities(std::string platformName,double duration);
	/* Insert the Entities ,that is the passenger movement of train to restricted at particular platform
	 * and type of restriction like boarding only(no alighting),alighting only(no boarding)
	 * And boarding ,alighting both
	 */
	void InsertRestrictPassengerEntity(std::string platformName,int type);

	/* checks if is boarding restricted by restricting passenger entity
	 * The request given by service controller
	 */
	bool IsBoardingRestricted();
	/* checks if is alighting restricted by restricting passenger entity
		 * The request given by service controller
		 */
	bool IsAlightingRestricted();

	/* The train is not supposed to stop at certain platforms as requested by service controller
	 * This function returns those list of platforms
	 */

	std::vector<std::string> GetPlatformsToBeIgnored();

	/* add platforms to ignore (where the train wont stop )
	 * by service controller
	 */
	void AddPlatformsToIgnore(std::vector<std::string> PlatformsToIgnore);
	/* mutex lock when modifying or calling the boarding alighting functions
	 *
	 */
	void LockUnlockRestrictPassengerEntitiesLock(bool lockunlock);

	/*
	 * Resets the holding time at platform which is due to the request given by service controller
	 * To override the dwell time calculated
	 */
	void ResetHoldingTime();

	void SetUnsetUturnFlag(bool set);

	bool GetUTurnFlag();

	void setForceAlightFlag(bool flag);

	bool getForceAlightFlag();

	void setForceAlightStatus(bool status);

	bool getForceAlightStatus();

	TrainDriver::TRAIN_NEXTREQUESTED getSubsequentNextRequested();

	void setSubsequentNextRequested(TrainDriver::TRAIN_NEXTREQUESTED nextReq);

	/**
	 * Event handler which provides a chance to handle event transfered from parent agent.
	 * @param sender pointer for the event producer.
	 * @param id event identifier.
	 * @param args event arguments.
	 */
	virtual void onParentEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);


private:
	/**get next train driver*/
     TrainDriver* nextDriver;
	/**next requested*/
	TRAIN_NEXTREQUESTED nextRequested;

	TRAIN_NEXTREQUESTED subsequent_nextRequested;
	/**current waiting time*/
	double waitingTimeSec;
	double initialDwellTime;
	double remainingStopTime;
	double minDwellTimeRequired;

	PolyPoint currStopPoint;
	/**passengers list*/
	std::list<Passenger*> passengerList;
	std::vector<StopPointEntity> stopPointEntities;
    bool shouldTerminateService=false;
    bool forceAlightPassengers_ByServiceController=false;
    bool isForceAlighted=false;
    std::vector<PlatformHoldingTimeEntity> platformHoldingTimeEntities;
    std::map<std::string,passengerMovement> restrictPassengersEntities;
    std::vector<std::string> platformsToBeIgnored;
	/**the locker for this driver*/
	mutable boost::mutex driverMutex;
	mutable boost::mutex platformHoldingTimeEntitiesLock;;
	mutable boost::mutex stopPointEntitiesLock;
	mutable boost::mutex restrictEntitiesLock;
	mutable boost::mutex restrictPassengersEntitiesLock;
	mutable boost::mutex platformsToBeIgnoredLock;
	mutable boost::mutex terminateTrainServiceLock;
	bool uTurnFlag=false;
	/**sequence no for platforms*/
	unsigned int platSequenceNumber;

	TrainDriver *nextDriverInOppLine;
	bool holdTrain=false;
	bool stoppedAtPoint=false;
	static int counter;

	/**recording disruption information*/
	boost::shared_ptr<DisruptionParams> disruptionParam;
	/**arrival time when stopping the platform*/
	std::string arrivalTimeAtPlatform;

private:
	friend class TrainBehavior;
	friend class TrainMovement;
};
}
} /* namespace sim_mob */


