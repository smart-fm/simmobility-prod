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
#include  "entities/roles/passenger/Passenger.hpp"
#include "buffering/Shared.hpp"
#include "buffering/BufferedDataManager.hpp"
namespace sim_mob {
namespace medium {

class TaxiDriver: public Driver {
public:
	TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat,
			TaxiDriverBehavior* behavior, TaxiDriverMovement* movement,
			std::string roleName,
			Role<Person_MT>::Type roleType = Role<Person_MT>::RL_TAXIDRIVER);

	TaxiDriver(Person_MT* parent, const MutexStrategy& mtx);
	bool addPassenger(Passenger* passenger);
	void alightPassenger();
	void boardPassenger(Passenger *passenger);
	void driveToDestinationNode(Node * destinationNode);
	void runRouteChoiceModel(const Node *origin, const Node *destination,SubTrip &currSubTrip, std::vector<WayPoint> &currentRouteChoice);
	void setCurrentNode(const Node *currNode);
	void setDestinationNode(const Node *destinationNode);
	const Node* getDestinationNode();
	Person_MT* getParent();
	const Node* getCurrentNode();
	void checkPersonsAndPickUpAtNode(Conflux *parentConflux);
	TaxiDriverMovement * getMovementFacet();
	virtual Role<Person_MT>* clone(Person_MT *parent) const;
	virtual ~TaxiDriver();
	void make_frame_tick_params(timeslice now);
	std::vector<BufferedBase*> getSubscriptionParams();
	void setTaxiDriveMode(const DriverMode &mode) {
		taxiDriverMode = mode;
		driverMode = mode;
	}
	const DriverMode & getDriverMode() const;
	Passenger* getPassenger();

private:
	Passenger *taxiPassenger = nullptr;
	const Node * destinationNode = nullptr;
	const Node *originNode = nullptr;
	const Node *currentNode = nullptr;
	const Node *pickupNode = nullptr;
	RoadSegment *currSegment = nullptr;
	TaxiDriverMovement *taxiDriverMovement;
	TaxiDriverBehavior *taxiDriverBehaviour;
	bool personBoarded = false;
	std::vector<WayPoint> currentRouteChoice;
	DriverMode taxiDriverMode;

public:
	friend class TaxiDriverBehavior;
	friend class TaxiDriverMovement;
};
}
}

#endif /* ENTITIES_ROLES_DRIVER_TAXIDRIVER_HPP_ */
