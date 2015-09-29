//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/Person.hpp"

namespace sim_mob
{

class Person_ST : public Person
{
private:
	/**Time taken by the person to board a bus*/
	double boardingTimeSecs;

	/**Time taken by the person to alight from a bus*/
	double alightingTimeSecs;

public:
	/**The lane in which the person starts*/
	int laneID;

	/**The segment in which the person starts*/
	int initSegId;

	/**The offset from the segment at which the person starts*/
	int initDis;

	/**The speed at which the person starts*/
	int initSpeed;

	/**Id of the autonomous vehicle. If it is a normal vehicle, this id is -1*/
	std::string amodId;

	/**The path selected by the autonomous vehicle*/
	std::vector<WayPoint> amodPath;

	/**The segment at which an autonomous vehicle has been requested for a pick-up*/
	std::string amodPickUpSegmentStr;

	/**The segment at which an autonomous vehicle is to drop-off the passenger*/
	std::string amodDropOffSegmentStr;

	/**The offset distance from the start of a segment where an autonomous vehicle has been requested for a pick-up*/
	double amodPickUpOffset;

	/**The offset distance from the start of a segment where an autonomous vehicle is to drop off the person*/
	double amodDropOffset;

	/**The trip id of the AMOD system*/
	std::string amodTripId;

	Person_ST(const std::string& src, const MutexStrategy& mtxStrat, int id = -1, std::string databaseID = "");
	Person_ST(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc);
	virtual ~Person_ST();

	/**Sets the person's characteristics by some distribution*/
	virtual void setPersonCharacteristics();

	/**
	 * Load a Person's properties (specified in the configuration file), creating a placeholder trip chain if
	 * requested.
     *
	 * @param configProps The properties specified in the configuration file
     */
	virtual void load(const std::map<std::string, std::string>& configProps);

	/**
	 * Initialises the trip chain
     */
	void initTripChain();

	void handleAMODArrival();
	
	void handleAMODPickup();

	void setPath(std::vector<WayPoint>& path);
	
	void handleAMODEvent(sim_mob::event::EventId id, sim_mob::event::Context ctxId, sim_mob::event::EventPublisher* sender, const AMOD::AMODEventArgs& args);

	double getBoardingCharacteristics() const
	{
		return boardingTimeSecs;
	}

	double getAlightingCharacteristics() const
	{
		return alightingTimeSecs;
	}
};
}