/*
 * MobilityServiceDriver.cpp
 *
 *  Created on: 22 Jun 2017
 *      Author: Andrea Araldo, araldo@mit.edu
 */

#include <conf/ConfigManager.hpp>
#include "MobilityServiceDriver.hpp"
#include "entities/controllers/MobilityServiceController.hpp"


using namespace sim_mob;

const MobilityServiceDriverStatus MobilityServiceDriver::getDriverStatus() const
{
    return driverStatus;
}

const unsigned MobilityServiceDriver::getNumAssigned() const
{
    return 0;
}

const std::string MobilityServiceDriver::getDriverStatusStr() const
{
    switch(driverStatus)
    {
    case DRIVE_START: {return "DRIVE_START"; }
    case CRUISING:{ return "CRUISING"; }
    case DRIVE_TO_TAXISTAND:{return "DRIVE_TO_TAXISTAND"; }
    case DRIVE_WITH_PASSENGER:{return "DRIVE_WITH_PASSENGER"; }
    case DRIVE_FOR_DRIVER_CHANGE_SHIFT: {return "DRIVE_FOR_DRIVER_CHANGE_SHIFT";}
    case QUEUING_AT_TAXISTAND: {return "QUEUING_AT_TAXISTAND";}
    case DRIVE_FOR_BREAK: {return "DRIVE_FOR_BREAK";}
    case DRIVER_IN_BREAK: {return "DRIVER_IN_BREAK";}
    case DRIVE_ON_CALL:{return "DRIVE_ON_CALL";}
    case DRIVE_TO_PARKING:{return "DRIVE_TO_PARKING";}
    case PARKED:{return "PARKED";}
    default: throw std::runtime_error("drive mode not recognized");
    }
}

void MobilityServiceDriver::setDriverStatus(const MobilityServiceDriverStatus status)
{
    driverStatus = status;
}

bool MobilityServiceDriver::hasMultipleSubscriptions() const
{
#ifndef NDEBUG
    if (getSubscribedControllers().size()>1)
    {
        std::stringstream msg; msg << "A driver is subscribed to more than a controller. This is possible in general, but for the moment we do not allow this.";
        throw std::runtime_error(msg.str() );
    }
#endif
    return (getSubscribedControllers().size()>1);
}

bool MobilityServiceDriver::isDriverControllerStudyAreaEnabled()
{
    const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
    std::vector<MobilityServiceController *> driverSubscribedToControllers = getSubscribedControllers();
    bool studyAreaEnabledAmodControllerOfDriver = false;

    for (auto thisDriverController : driverSubscribedToControllers)
    {
        if(thisDriverController->controllerServiceType == MobilityServiceControllerType::SERVICE_CONTROLLER_AMOD
            && thisDriverController->studyAreaEnabledController)
        {
            studyAreaEnabledAmodControllerOfDriver = true;
            break;
        }
    }
    return studyAreaEnabledAmodControllerOfDriver;
}

const std::string MobilityServiceDriver::getSubscribedControllerTypesStr() const
{
    std::string returnString = "";
    for (const MobilityServiceController* controller : getSubscribedControllers() )
    {
        returnString += ( toString(controller->getServiceType() ) + ", " );
    }
    return returnString;
}

bool sim_mob::isMobilityServiceDriver(const Person* person)
{
    const MobilityServiceDriver *serviceDriver = person->exportServiceDriver();
    if (serviceDriver) {
        return true;
    }
    else {
        return false;
    }
}

sim_mob::VehicleBase::VehicleType MobilityServiceDriver::getVehicleType() const
{
    return VehicleBase::CAR;
}