/*
 * TrainStationAgent.hpp
 *
 *  Created on: Feb 24, 2016
 *      Author: zhang huai peng
 */

#ifndef TRAINSTATIONAGENT_HPP_
#define TRAINSTATIONAGENT_HPP_
#include "entities/Agent.hpp"
#include "geospatial/network/PT_Stop.hpp"
namespace sim_mob {
namespace medium
{
class TrainDriver;
class TrainStationAgent : public sim_mob::Agent {
public:
	TrainStationAgent();
	virtual ~TrainStationAgent();
	void setStation(const Station* station);

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
private:
	/**
	 * dispatch pending train
	 * @param now current time slice
	 */
	void dispathPendingTrains(timeslice now);

private:
	/**the reference to the station*/
	const Station* station;
	/**the reference */
	std::list<TrainDriver*> trainDriver;
	/**record last train in each line*/
	std::map<std::string, TrainDriver*> lastTrainDriver;
	/**record pending trains in each line*/
	std::map<std::string, std::list<TrainDriver*>> pendingTrainDriver;
	/**record usage in each line*/
	std::map<std::string, bool> lastUsage;

};
}
} /* namespace sim_mob */

#endif /* TRAINSTATIONAGENT_HPP_ */
