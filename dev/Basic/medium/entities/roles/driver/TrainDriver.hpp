/*
 * TrainDriver.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#include <atomic>
#include "entities/Person_MT.hpp"
#include "entities/roles/Role.hpp"
#include "TrainUpdateParams.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/waitTrainActivity/WaitTrainActivity.hpp"
namespace sim_mob {
namespace medium{
class TrainBehavior;
class TrainMovement;
class TrainDriver : public sim_mob::Role<Person_MT>, public UpdateWrapper<TrainUpdateParams> {
public:
	enum TRAIN_STATUS{
		NO_STATUS=0,
		ARRIVAL_AT_PLATFORM,
		WAITING_LEAVING,
		LEAVING_FROM_PLATFORM,
		MOVE_TO_PLATFROM,
		MOVE_TO_DEPOT
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
	 * get current train status
	 * @return current status in the train
	 */
	TRAIN_STATUS getCurrentStatus() const;
	/**
	 * set next status
	 */
	void setCurrentStatus(TRAIN_STATUS status);
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
	 * computing dwell time
	 * @param totalNum is the total number of boarding and alighting
	 */
	void calculateDwellTime(int totalNum);
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
	int alightPassenger(std::list<Passenger*>& alightingPassenger);
	/**
	 * passenger boarding
	 * @param boardingPassenger is the list of boarding person
	 */
	int boardPassenger(std::list<WaitTrainActivity*>& boardingPassenger,timeslice now);
	/**
	 * update passengers inside the train
	 */
	void updatePassengers();
private:
	/**get next train driver*/
	const TrainDriver* nextDriver;
	/**current status*/
	std::atomic<TRAIN_STATUS> trainStatus;
	/**current waiting time*/
	double waitingTimeSec;
	/**passengers list*/
	std::list<Passenger*> passengerList;
private:
	friend class TrainBehavior;
	friend class TrainMovement;
};
}
} /* namespace sim_mob */


