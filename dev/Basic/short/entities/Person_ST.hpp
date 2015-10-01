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

	/**The previous role that was played by the person.*/
	Role<Person_ST>* prevRole;

	/**The current role being played by the person*/
	Role<Person_ST>* currRole;

	/**The next role that the person will play. However, this variable is only temporary and will not be used to update the currRole*/
	Role<Person_ST>* nextRole;

	/**Indicates whether we have registered to receive communication simulator related messages*/
	bool commEventRegistered;

	/**
	 * Advances the current trip chain item to the next item if all the sub-trips in the trip have been completed.
	 * If not, calls the advanceCurrentTripChainItem() method
     *
	 * @return true, if the trip chain item is advanced
     */
	bool advanceCurrentTripChainItem();

	/**
	 * Enable Region support
	 * See RegionAndPathTracker for more information.
	 */
	void enableRegionSupport()
	{
		regionAndPathTracker.enable();
	}

protected:
	/**
	 * Called during the first call to update() for the person
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return false to indicate failure; The person will be removed from the simulation with no
	 * further processing.
	 */
	virtual bool frame_init(timeslice now);

	/**
	 * Called during every call to update() for a given person. This is called after frame_tick()
	 * for the first call to update().
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 *
	 * @return an UpdateStatus indicating future action
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * Called after frame_tick() for every call to update() for a given agent.
	 * Use this method to display output for this time tick.
	 *
	 * @param now The timeslice representing the time frame for which this method is called
	 */
	virtual void frame_output(timeslice now);
	
	/**Inherited from EventListener*/
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args);

	/**Inherited from MessageHandler.*/
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message &message);

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

	Person_ST(const std::string &src, const MutexStrategy &mtxStrat, int id = -1, std::string databaseID = "");
	Person_ST(const std::string &src, const MutexStrategy &mtxStrat, const std::vector<sim_mob::TripChainItem *> &tc);
	virtual ~Person_ST();

	/**Sets the person's characteristics by some distribution*/
	virtual void setPersonCharacteristics();

	/**
	 * Sets the simulation start time of the entity
	 *
     * @param value The simulation start time to be set
     */
	virtual void setStartTime(unsigned int value);

	/**
	 * Load a Person's properties (specified in the configuration file), creating a placeholder trip chain if
	 * requested.
     *
	 * @param configProps The properties specified in the configuration file
     */
	virtual void load(const std::map<std::string, std::string> &configProps);

	/**
	 * Initialises the trip chain
     */
	void initTripChain();

	/**
	 * Check if any role changing is required.

	 * @return
     */
	Entity::UpdateStatus checkTripChain();

	/**
	 * Finds the person's next role based on the person's trip-chain
	 *
     * @return true, if the next role is successfully found
     */
	bool findPersonNextRole();

	/**
	 * Updates the person's current role to the given role.
	 *
     * @param newRole Indicates the new role of the person. If the new role is not provided,
	 * a new role is created based on the current sub-trip
	 *
     * @return true, if role is updated successfully
     */
	bool updatePersonRole();

	/**
	 * Builds a subscriptions list to be added to the managed data of the parent worker
	 *
	 * @return the list of Buffered<> types this entity subscribes to
	 */
	virtual std::vector<BufferedBase *> buildSubscriptionList() = 0;

	/**
	 * Change the role of this person
	 *
     * @param newRole the new role to be assigned to the person
     */
	void changeRole();

	/**
	 * Ask this person to re-route to the destination with the given set of blacklisted RoadSegments
	 * If the Agent cannot complete this new route, it will fall back onto the old route.
	 *
	 * @param blacklisted the black-listed road segments
	 */
	virtual void rerouteWithBlacklist(const std::vector<const sim_mob::RoadSegment *> &blacklisted);

	void handleAMODArrival();
	
	void handleAMODPickup();

	void setPath(std::vector<WayPoint> &path);
	
	void handleAMODEvent(sim_mob::event::EventId id, sim_mob::event::Context ctxId, sim_mob::event::EventPublisher *sender, const AMOD::AMODEventArgs &args);

	double getBoardingCharacteristics() const
	{
		return boardingTimeSecs;
	}

	double getAlightingCharacteristics() const
	{
		return alightingTimeSecs;
	}

	Role<Person_ST>* getRole() const
	{
		return currRole;
	}

	Role<Person_ST>* getPrevRole() const
	{
		return prevRole;
	}

	void setNextRole(Role<Person_ST> *newRole)
	{
		nextRole = newRole;
	}

	Role<Person_ST>* getNextRole() const
	{
		return nextRole;
	}
};
}