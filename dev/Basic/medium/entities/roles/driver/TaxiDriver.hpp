/*
 * TaxiDriver.hpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#ifndef ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_
#define ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_

#include "Driver.hpp"
#include "TaxiDriverFacets.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "buffering/Shared.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"

namespace sim_mob
{

namespace medium
{

class TaxiDriver : public Driver, public MobilityServiceDriver
{
public:
	TaxiDriver(Person_MT *parent, const MutexStrategy &mtxStrat,
	           TaxiDriverBehavior *behavior, TaxiDriverMovement *movement,
	           std::string roleName,
	           Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TAXIDRIVER);

	virtual ~TaxiDriver();

	TaxiDriver(Person_MT *parent, const MutexStrategy &mtx);

	/**
	 * add a new passenger into taxi
	 * @param passenger is a pointer to the taxi passenger
	 * @return true if boarding successfully
	 */
	bool addPassenger(Passenger *passenger);

	/**
	 * alight a passenger when arriving the destination
	 */
	void alightPassenger();

	/**
	 * get current Node
	 * @return current Node
	 */
	const Node *getCurrentNode() const;

	/**
	 * passenger route choice after boarding into taxi
	 * @param origin is a pointer to original node
	 * @param destination is a pointer to destination node
	 * @param currentRouteChoice hold the route choice result
	 */
	void
	passengerChoiceModel(const Node *origin, const Node *destination, std::vector<WayPoint> &currentRouteChoice);

	/**
	 * get parent object
	 * @return parent object
	 */
	Person_MT *getParent();

	/**
	 * perform pickup at the node
	 * @param personId is the id of the person to be picked up, if it is an empty string,
	 * on hail drivers pick up a person from the node (without knowing the id)
	 */
	void pickUpPassngerAtNode(const std::string personId = "");

	/**
	 * get movement facet
	 * @return movement facet which is in charge of movement
	 */
	TaxiDriverMovement *getMovementFacet();

	/**
	 * clone taxi-driver object
	 * @return cloned result which hold a taxi-driver role
	 */
	virtual Role<Person_MT> *clone(Person_MT *parent) const;

	/**
	 * make parameters for current frame-tick update
	 * @param now is current simulation time
	 */
	void make_frame_tick_params(timeslice now);

	/**
	 * override from base class
	 * @return a shared buffer if used
	 */
	std::vector<BufferedBase *> getSubscriptionParams();

	/**
	 * Process the next schedule item and updates the currScheduleItem
	 * @param isMoveToNextScheduleItem Indicates whether we increment the iterator
	 * pointing to the schedule item
	 */
	void processNextScheduleItem(bool isMoveToNextScheduleItem = true);

	/**
	 * get current passenger
	 * @return a passenger object if have.
	 */
	Passenger *getPassenger();

	/**
	 * @return the count of passengers on board
	 */
	const unsigned long getPassengerCount() const;

	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &message);

	/**
	 * export service driver
	 * @return exporting result
	 */
	virtual const MobilityServiceDriver *exportServiceDriver() const;

	virtual const std::vector<MobilityServiceController*>& getSubscribedControllers() const;

private:
	/**Holds all the passengers on board, the key to the map is the person db id*/
	std::map<const std::string, Passenger *> taxiPassengers;

	/**The taxiPassenger that will be dropped off next*/
	Passenger *taxiPassenger = nullptr;

	/**hold movement facet object*/
	TaxiDriverMovement *taxiDriverMovement;

	/**hold behavior facet object*/
	TaxiDriverBehavior *taxiDriverBehaviour;

	/**Holds the schedule assigned by the controller*/
	Schedule assignedSchedule;

	/**Points to the current schedule item*/
	Schedule::const_iterator currScheduleItem;

	/**The mobility service controller that sent the current schedule*/
	messaging::MessageHandler *controller = nullptr;

	/**The parking location of the vehicle*/
	const SMSVehicleParking *parkingLocation = nullptr;

public:
	friend class TaxiDriverBehavior;

	friend class TaxiDriverMovement;
};

}
}
#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_ */

