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
namespace sim_mob {
namespace medium {

class TaxiDriver: public Driver, public MobilityServiceDriver {
public:
	TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat,
			TaxiDriverBehavior* behavior, TaxiDriverMovement* movement,
			std::string roleName,
			Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TAXIDRIVER);

	virtual ~TaxiDriver();

	TaxiDriver(Person_MT* parent, const MutexStrategy& mtx);
	/**
	 * add a new passenger into taxi
	 * @param passenger is a pointer to the taxi passenger
	 * @return true if boarding successfully
	 */
	bool addPassenger(Passenger* passenger);
	/**
	 * alight a passenger when arriving the destination
	 */
	void alightPassenger();
	/**
	 * get current driver status
	 * @return status value
	 */
	MobilityServiceDriver::ServiceStatus getServiceStatus();
	/**
	 * get current Node
	 * @return current Node
	 */
	const Node* getCurrentNode();
	/**
	 * passenger route choice after boarding into taxi
	 * @param origin is a pointer to original node
	 * @param destination is a pointer to destination node
	 * @param currentRouteChoice hold the route choice result
	 */
	void passengerChoiceModel(const Node *origin, const Node *destination, std::vector<WayPoint> &currentRouteChoice);
	/**
	 * get parent object
	 * @return parent object
	 */
	Person_MT* getParent();
	/**
	 * perform pickup at the node
	 * @param parentConflux is a pointer to the current conflux
	 * @param personId is a pointer to the person id, default value is zero
	 */
	void pickUpPassngerAtNode(Conflux *parentConflux, std::string* personId=nullptr);
	/**
	 * get movement facet
	 * @return movement facet which is in charge of movement
	 */
	TaxiDriverMovement * getMovementFacet();
	/**
	 * clone taxi-driver object
	 * @return cloned result which hold a taxi-driver role
	 */
	virtual Role<Person_MT>* clone(Person_MT *parent) const;
	/**
	 * make parameters for current frame-tick update
	 * @param now is current simulation time
	 */
	void make_frame_tick_params(timeslice now);
	/**
	 * override from base class
	 * @return a shared buffer if used
	 */
	std::vector<BufferedBase*> getSubscriptionParams();
	/**
	 * set current driving mode
	 * @param mode hold current mode
	 */
	void setTaxiDriveMode(const DriverMode &mode);
	/**
	 * get current driving mode
	 * @return current mode
	 */
	const DriverMode & getDriverMode() const;
	/**
	 * get current passenger
	 * @return a passenger object if have.
	 */
	Passenger* getPassenger();
	/**
	 * message handler which provide a chance to handle message transfered from parent agent.
	 * @param type of the message.
	 * @param message data received.
	 */
	virtual void HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message);
	/**
	 * export service driver
	 * @return exporting result
	 */
	virtual MobilityServiceDriver* exportServiceDriver();

private:
	/**hold passenger object*/
	Passenger *taxiPassenger = nullptr;
	/**hold movement facet object*/
	TaxiDriverMovement *taxiDriverMovement;
	/**hold behavior facet object*/
	TaxiDriverBehavior *taxiDriverBehaviour;
	/** store current mode*/
	DriverMode taxiDriverMode=DRIVE_START;

public:
	friend class TaxiDriverBehavior;
	friend class TaxiDriverMovement;
};
}
}

#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_ */

