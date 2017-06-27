/*
 * MobilityServiceDriver.hpp
 *
 *  Created on: May 2, 2017
 *      Author: zhang huai peng
 */

#ifndef MOBILITYSERVICEDRIVER_HPP_
#define MOBILITYSERVICEDRIVER_HPP_

#include <iostream>
#include "entities/controllers/MobilityServiceController.hpp"

namespace sim_mob {
class Node;
class MobilityServiceController;


enum MobilityServiceDriverStatus
{
	DRIVE_START =0,
	CRUISING,
	DRIVE_TO_TAXISTAND,
	DRIVE_WITH_PASSENGER,
	DRIVE_FOR_DRIVER_CHANGE_SHIFT,
	QUEUING_AT_TAXISTAND,
	DRIVE_FOR_BREAK,
	DRIVER_IN_BREAK,
	DRIVE_ON_CALL
};



class MobilityServiceDriver {
public:


	MobilityServiceDriver():driverStatus(DRIVE_START){};
	virtual ~MobilityServiceDriver(){};

	/**
	 * the interface function to get current node
	 * @return current node.
	 */
	virtual const Node* getCurrentNode() const = 0;


	virtual const MobilityServiceDriverStatus getDriverStatus() const;
	virtual const std::string getDriverStatusStr() const;
	virtual void setDriverStatus(const MobilityServiceDriverStatus status);
	virtual bool canSheMove() const;
	virtual const std::vector<MobilityServiceController*>& getSubscribedControllers() const = 0;
	virtual const std::string getSubscribedControllerTypesStr() const;
	virtual bool hasMultipleSubscriptions() const;

protected:
	MobilityServiceDriverStatus driverStatus;
};

bool isMobilityServiceDriver(const Person* person);

} /* namespace sim_mob */

#endif /* MOBILITYSERVICEDRIVER_HPP_ */
