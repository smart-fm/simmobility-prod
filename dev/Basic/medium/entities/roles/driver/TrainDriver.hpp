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

	static long boardPassengerCount;
	mutable boost::mutex boardPassengerLock;
	virtual ~TrainDriver();

	TrainDriver(Person_MT* parent,
		sim_mob::medium::TrainBehavior* behavior = nullptr,
		sim_mob::medium::TrainMovement* movement = nullptr,
		std::string roleName = std::string(),
		Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TRAINDRIVER);

	virtual Role<Person_MT>* clone(Person_MT *parent) const;
	virtual void make_frame_tick_params(timeslice now);
	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();

	/*
	 * function to modify the next driver for the train
	 * @param driver is the next driver (train ahead) the train should point to
	 */
	void setNextDriver(TrainDriver* nextDriver);

	bool operator< (TrainDriver * &other);

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
	int alightAllPassengers(std::list<Passenger*>& alightingPassenger);

	/* stores the next train(ahead train ) in opposite direction before teleport*/
	void setTrainDriverInOpposite(TrainDriver *trainDriver);
	/* gets the train driver in opposite line to assign it to next driver */
	TrainDriver *getDriverInOppositeLine();
	const TrainTrip *getTrainTrip() const;


	/**
	 * alight all passengers
	 * @param alightingPassenger is the list of alighting person
	 * @param now is current time
	 * @return the number of alighting persons
	 */
	int alightAllPassengers(std::list<Passenger*>& alightingPassenger,timeslice now);


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

	/* Interafce to get the train id
	 * @return train id
	 */
	int getTrainId() const;

	/**
	 * Interface that inserts stop point that is the point where train has to be stopped as requested by service controller
	 * @param point is the polypoint where the train is to be stopped
	 * @param duration is the duration for which it has  be stopped
	 */
	void insertStopPoint(PolyPoint point,double duration);

	/**
	 * Interface that gets all the stop current points of the train
	 * @return returns the vector of stop points
	 */
	std::vector<StopPointEntity>& getStopPoints();

	/**
	 * Interface that checks if the train has stopped at a point or not
	 * @return true if it has stopped at a point
	 */
	bool isStoppedAtPoint();

	/**
	 * Interface that sets the parameters of stop point Entities ,that is the POLypoint and duration of stopping
	 * @param point is the point where it is currently stopped at
	 * @duration is the duration to be stopped at the point
	 */
	void setStoppingParameters(PolyPoint point,double duration);

	/**
	 * Function that reduces the stopping duration left after every frame tick
	 * @param secsInframeTick is the number of seconds in frame tick
	 * @return remaining stopping time for that point
	 */
	double reduceStoppingTime(double secsInframeTick);

	/**
	 * Interface to set stopping time of train at stop point
	 * @param stoppingTime is the time required to stop
	 */
	void setStoppingTime(double stoppingTime);

	/**
	 * Interface that sets the stopping status  of train ,that is whether train is stopped point
	 * @param status is the parameter to set true or false for stopping status
	 */
	void  setStoppingStatus(bool status);

	/**
	 * Function that clears all stop points
	 */
	void clearStopPoints();

	/**
	 * Function that terminates the train service
	 * @param terminate whether to terminate or not
	 */
	void setTerminateTrainService(bool terminate);

	/**
	 * Function that checks the status whether the train is to be terminated or not
	 * @return true if the train service of that train is to be terminated else false
	 */
	bool getTerminateStatus();

	/**
	 * Function that inserts the instances when the train is supposed to be held by platform as requested by service controller
	 * @param platformName is the name of the platform where train has to held
	 * @param duration is the duration for which the train has to be held
	 */
	void insertPlatformHoldEntities(std::string platformName,double duration);


	/**
	 * Insert the Entities ,that is the passenger movement of train to restricted at particular platform
	 * and type of restriction like boarding only(no alighting),alighting only(no boarding)
	 * and boarding ,alighting both
	 * @param platformName is the name of the platform where passenger activity is to be restricted
	 * @param type is the type of restriction boarding,alighting or both
	 */
	void insertRestrictPassengerEntity(std::string platformName,int type);

	/**
	 * checks if is boarding restricted by restricting passenger entity as requested by service controller
	 * @return true if boarding is restricted else false
	 */
	bool isBoardingRestricted();

	/**
	 * checks if is alighting restricted by restricting passenger entity as requested by service controller
	 * @return true if boarding is restricted else false
	 */
	bool isAlightingRestricted();

	/**
	 * This function returns those list of platforms where the train is not supposed to stop at certain platforms as requested by service controller
	 * @return the list of platforms to be ignored
	 */
	std::vector<std::string> getPlatformsToBeIgnored();

	/**
	 * add platforms to ignore (where the train wont stop )  by service controller
	 * @param PlatformsToIgnore is the vector of platforms to be ignored
	 */
	void AddPlatformsToIgnore(std::vector<std::string> PlatformsToIgnore);

	/**
	 * mutex lock when modifying or calling the boarding alighting functions
	 * @param lockunlock is to lock unlock boarding alighting operation
	 */
	void lockUnlockRestrictPassengerEntitiesLock(bool lockunlock);

	/**
	 * Function that resets the holding time at platform which is due to the request given by service controller
	 * To override the dwell time calculated
	 */
	void resetHoldingTime();

	/**
	 * function which sets or unsets U-turn signal
	 * @param set sets the signal to U turn
	 */
	void setUturnFlag(bool set);

	/**
	 * function which gets the status of uturn flag
	 * @return bool returns true if U-turn flag is set
	 */
	bool getUTurnFlag();

	/**
	 * function to set the force alight flag that is the signal when the train is supposed to force alight
	 * @param flag is set to true if the train is supposed to force alight
	 */
	void setForceAlightFlag(bool flag);

	/**
	 * function to know if force alight flag is set
	 * @return true if force alight flag is set.
	 */
	bool getForceAlightFlag();

	/**
	 * function which sets the status of force alighted ,that is it is set to true if the passenegers have already force alighted
	 * @param status is true if passengers have already force alighted
	 */
	void setForceAlightStatus(bool status);

	/**
	 * function which return the status of force alighted whether the passengers have force alighted or not
	 * @return the status of force alighted
	 */
	bool getForceAlightStatus();

	/*
	 * function which gets the subsequent next requested for the train ,that is the next requested in future
	 * @return the subsequent next requested
	 */
	TrainDriver::TRAIN_NEXTREQUESTED getSubsequentNextRequested();

	/**
	 * function which sets the subsequent next requested for the train
	 * @param nextReq is the subsequent next requested
	 */
	void setSubsequentNextRequested(TrainDriver::TRAIN_NEXTREQUESTED nextReq);

	bool getIsToBeRemoved();

	void setIsToBeRemoved(bool);

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
	/** The future next requested for the train **/
	TRAIN_NEXTREQUESTED subsequent_nextRequested;
	/**current waiting time*/
	double waitingTimeSec;
	/** dwell time set for the fist time before time elapses or the dwell time is reset by service controller**/
	double initialDwellTime;
	/** The remaining stop time after train has stopped at point and some time is elapsed**/
	double remainingStopTime;
	/** the minimum dwell time required for the train at the station depending on whether its normal station,interchange or terminal station **/
	double minDwellTimeRequired;
	/** The current stop point of the train **/
	PolyPoint currStopPoint;
	/**The list of passengers for the train */
	std::list<Passenger*> passengerList;
	/** These are the data entities of stop points,which has attributes such as stop point,stop suration */
	std::vector<StopPointEntity> stopPointEntities;
	/** flag to terminate train service **/
    bool shouldTerminateService=false;
    /** flag to force alight passengers by service controller **/
    bool forceAlightPassengers_ByServiceController=false;
    /** The bool which indicates whether passengers have force alighted **/
    bool isForceAlighted=false;
    /** vector which holds the data about overriding the default dwell time calculated at platform  as requested by service controller**/
    std::vector<PlatformHoldingTimeEntity> platformHoldingTimeEntities;
    /** map which holds the data for restricting passenger movement at certain platforms along train route **/
    std::map<std::string,passengerMovement> restrictPassengersEntities;
    /** vector which specifies the platforms to be ignored by the train on its route as specified by service controller **/
    std::vector<std::string> platformsToBeIgnored;
	/**the locker for this driver*/
	mutable boost::mutex driverMutex;
	/** lock for platformHoldingTimeEntities **/
	mutable boost::mutex platformHoldingTimeEntitiesLock;
	/** lock for stop point entities **/
	mutable boost::mutex stopPointEntitiesLock;
	mutable boost::mutex restrictEntitiesLock;
	/** lock for restrict passenger entities **/
	mutable boost::mutex restrictPassengersEntitiesLock;
	/** lock for platform ignore list **/
	mutable boost::mutex platformsToBeIgnoredLock;
	/** lock for terminate train service **/
	mutable boost::mutex terminateTrainServiceLock;
	/** the flag or signal for U Turn **/
	bool uTurnFlag=false;
	/**sequence no for platforms*/
	unsigned int platSequenceNumber;
	/** the nearest train driver in opposite line after the opposite platform,it is required for U-Turn **/
	TrainDriver *nextDriverInOppLine;
	/** indicated whether train is stopped at stop point **/
	bool stoppedAtPoint=false;
	static int counter;
	/**recording disruption information*/
	boost::shared_ptr<DisruptionParams> disruptionParam;
	/**arrival time when stopping the platform*/
	std::string arrivalTimeAtPlatform;
	bool isToBeRemovedFromStationAgent=false;

	/* board passenger count */


private:
	friend class TrainBehavior;
	friend class TrainMovement;
};
}
} /* namespace sim_mob */


