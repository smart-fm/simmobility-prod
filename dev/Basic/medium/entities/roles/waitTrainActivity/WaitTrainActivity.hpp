//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/Role.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "entities/Person_MT.hpp"

namespace sim_mob
{
namespace medium
{
class TrainDriver;
class WaitTrainActivityBehavior;
class WaitTrainActivityMovement;

/**
 * A medium-term WaitTrainActivity.
 * \author zhang huai peng
 */
class WaitTrainActivity: public sim_mob::Role<Person_MT>, public UpdateWrapper<UpdateParams>
{
public:
	explicit WaitTrainActivity(Person_MT* parent, sim_mob::medium::WaitTrainActivityBehavior* behavior = nullptr,
			sim_mob::medium::WaitTrainActivityMovement* movement = nullptr, std::string roleName = std::string("WaitTrainActivity_"),
			Role<Person_MT>::Type roleType = Role<Person_MT>::RL_WAITTRAINACTIVITY);

	virtual ~WaitTrainActivity();

	virtual sim_mob::Role<Person_MT>* clone(Person_MT *parent) const;

	virtual void make_frame_tick_params(timeslice now);

	virtual std::vector<sim_mob::BufferedBase*> getSubscriptionParams();
	/**
	 * collect travel time for current role
	 */
	virtual void collectTravelTime();
	/**
	 * increase waiting time every frame tick.
	 * @param incrementMs is accumulation time in milli-seconds
	 */
	void increaseWaitingTime(unsigned int incrementMs);
	/**
	 * increase failed boarding times
	 */
	void incrementDeniedBoardingCount();
	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);
	/**
	 * get waiting time
	 * @return waiting time in seconds
	 */
	unsigned int getWaitingTime() const;;
	/**
	 * get denied boarding count
	 * @return denied boarding count
	 */
	unsigned int getDeniedBoardingCount() const;
	/**
	 * get starting platform
	 * @return the starting platform
	 */
	const Platform* getStartPlatform() const;
	/**
	 * set starting platform
	 * @param platform is a pointer to the Platform object
	 */
	void setStartPlatform(const Platform* platform);
	/**
	 * get train line
	 * @return the train line
	 */
	const std::string& getTrainLine() const;

	double getWalkTimeToPlatform() const;
	void setWalkTimeToPlatform(double walkTime);

	void reduceWalkingTime();
private:
	friend class WaitTrainActivityBehavior;
	friend class WaitTrainActivityMovement;

	unsigned int walkingTimeToPlatform;
	/**record waiting time (in milliseconds) in the bus stop*/
	unsigned int waitingTime;
	/**pointer to waiting bus stop*/
	const Platform* platform;
	/**failed boarding times*/
	unsigned int failedToBoardCount;
};
}
}
