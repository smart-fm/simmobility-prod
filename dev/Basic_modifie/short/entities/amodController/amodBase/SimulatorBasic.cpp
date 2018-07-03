/*
 * SimulatorBasic.cpp
 *
 *  Created on: Mar 27, 2015
 *      Author: haroldsoh
 */

#include "SimulatorBasic.hpp"
#include "logging/Log.hpp"

namespace sim_mob{

namespace amod {
    
SimulatorBasic::SimulatorBasic(double resolution_):
    resolution(resolution_), eventId(0),
    usingLocs(false)
{
    // just return
}

SimulatorBasic::~SimulatorBasic() {
    // just return
}

amod::ReturnCode  SimulatorBasic::init(amod::World *worldState) {

    // initialize the locations
    if (worldState->getNumLocations() > 0) {
        std::vector<Location> locs;
        worldState->getLocations(&locs);
        locTree.build(locs);
        usingLocs = true;

        // set the customer positions to be in valid locations
        std::unordered_map<int, Customer>::const_iterator bitr, eitr;
        worldState->getCustomers(&bitr, &eitr);
        for (auto itr = bitr; itr!=eitr; ++itr) {

            Customer * pcust = worldState->getCustomerPtr(itr->first);
            Location loc = locTree.findNN({pcust->getPosition().x, pcust->getPosition().y});
            pcust->setPosition(loc.getPosition());
            pcust->setLocationId(loc.getId());

            // add this customer to the location
            Location *ploc = worldState->getLocationPtr(loc.getId());
            ploc->addCustomerId(pcust->getId());
        }

        // set the vehicle positions to be in valid locations
        std::unordered_map<int, Vehicle>::const_iterator vbitr, veitr;
        worldState->getVehicles(&vbitr, &veitr);
        for (auto itr = vbitr; itr!=veitr; ++itr) {

            Vehicle * pveh = worldState->getVehiclePtr(itr->first);
            Location loc = locTree.findNN({pveh->getPosition().x, pveh->getPosition().y});
            pveh->setPosition(loc.getPosition());
            pveh->setLocationId(loc.getId());

            // add this vehicle to the location
            Location *ploc = worldState->getLocationPtr(loc.getId());
            ploc->addVehicleId(pveh->getId());
        }

    }

    // copy over the state
    state = *worldState;
    return amod::SUCCESS;
}

amod::ReturnCode  SimulatorBasic::update(amod::World *worldState) {
    // updates the world state
    state.setCurrentTime( worldState->getCurrentTime() + resolution );

    // simulate the vehicles
    simulateVehicles(worldState);

    // simulate the pickups
    simulatePickups(worldState);

    // simulate the dropoffs
    simulateDropoffs(worldState);

    // simulate the teleports
    simulateTeleports(worldState);

    // simulate the customers (this is currently a placeholder for future expansion)
    simulateCustomers(worldState);

    worldState->setCurrentTime( state.getCurrentTime() );

    return amod::SUCCESS;
}

amod::ReturnCode SimulatorBasic::dispatchVehicle(amod::World *worldState,
                                                 int vehId,
                                                 const amod::Position &to,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus
                                                 ) {
    return dispatchVehicle(worldState, vehId, to, startStatus, endStatus, 0);
}

amod::ReturnCode SimulatorBasic::pickupCustomer(amod::World *worldState,
                                                int vehId, int custId,
                                                amod::VehicleStatus startStatus,
                                                amod::VehicleStatus endStatus
                                                ) {
    return pickupCustomer(worldState, vehId, custId, startStatus, endStatus, 0);
}

amod::ReturnCode SimulatorBasic::dropoffCustomer(amod::World *worldState,
                                                 int vehId, int custId,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus
                                                 ) {
    return dropoffCustomer(worldState, vehId, custId, startStatus, endStatus, 0);
}


// internal helper functions

amod::ReturnCode SimulatorBasic::dispatchVehicle(amod::World *worldState,
                                                 int vehId,
                                                 const amod::Position &to,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus,
                                                 int bookingId) {
    // check if vehicle already exists in the dispatch map
    auto it = dispatches.find(vehId);
    if (it != dispatches.end()) {
        if (getVerbose()) Print() << "Vehicle ID found (it is currently being dispatched!): " << vehId << std::endl;
        return amod::VEHICLE_CANNOT_BE_DISPATCHED;
    }

    // create a new dispatch
    Vehicle veh = state.getVehicle(vehId);
    if (!veh.getId()) {
        if (getVerbose()) Print() << "Can't get vehicle from world_state" << std::endl;
        return amod::CANNOT_GET_VEHICLE;
    }

    Dispatch dp;
    dp.bookingId = bookingId;
    dp.vehId = vehId;
    dp.from = veh.getPosition();
    if (usingLocs) {
        Location des = locTree.findNN({to.x, to.y});
        dp.to = des.getPosition(); // find the closest location to be the destination position
        dp.toLocId = des.getId();
    } else {
        dp.to = to;
    }
    if (usingLocs) {
        Location des = locTree.findNN({dp.from.x, dp.from.y});
        dp.from = des.getPosition(); // find the closest location to be the destination position
        dp.fromLocId = des.getId();
    }

    dp.curr = dp.from;
    double dx = dp.to.x - dp.from.x;
    double dy = dp.to.y - dp.from.y;
    double rd = sqrt( dx*dx + dy*dy);
    if (rd == 0) {
        dp.grad = Position(1.0, 1.0); // just travel someplace (same location, will arrive at next timestep)
    } else {
        dp.grad = Position( dx/rd , dy/rd); // travel in the direction of the destination
    }
    dp.vehEndStatus = endStatus;

//        if (dp.from == dp.to) {
//            return amod::SOURCE_EQUALS_DESTINATION;
//        }
//

    // add it to the dispatch qmap
    dispatches[vehId] = dp;

    // update the vehicle status
    veh.setStatus(startStatus);
    worldState->setVehicle(veh);
    state.setVehicle(veh);
    if (bookingId) {
        //get the booking customer
        Customer cust = worldState->getCustomer(bookings[bookingId].custId);

        if (!cust.isInVehicle()) {
            //if (getVerbose()) std::cout << "Set customer waiting for pickup" << std::endl;
            cust.setStatus(CustomerStatus::WAITING_FOR_PICKUP);
            worldState->setCustomer(cust);
            state.setCustomer(cust);
        } else {
            //if (getVerbose()) std::cout << "Set customer in vehicle" << std::endl;
            cust.setStatus(CustomerStatus::IN_VEHICLE);
            worldState->setCustomer(cust);
            state.setCustomer(cust);
        }
    }

    // location specific changes
    if (usingLocs) {
        Location * ploc = worldState->getLocationPtr(dp.fromLocId);
        ploc->removeVehicleId(veh.getId());
        veh.setLocationId(0);
        worldState->setVehicle(veh);

        // trigger event
        Event ev(amod::EVENT_LOCATION_VEHS_SIZE_CHANGE, ++eventId,
                "LocationVehSizeChange", state.getCurrentTime(),
                {dp.fromLocId});
        worldState->addEvent(ev);

        Customer cust;
        if (bookingId) {
            cust = worldState->getCustomer(bookings[bookingId].custId);

            if (cust.isInVehicle()) {
                ploc->removeCustomerId(cust.getId());
                cust.setLocationId(0);
                worldState->setCustomer(cust);
                // trigger event
                Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                        "LocationCustSizeChange", state.getCurrentTime(),
                        {dp.fromLocId});
                worldState->addEvent(ev);

            }
        }

        // update internal state
        ploc = state.getLocationPtr(dp.fromLocId);
        ploc->removeVehicleId(veh.getId());
        if (bookingId && cust.isInVehicle()) {
            ploc->removeCustomerId(cust.getId());
        }
    }


    // create a dispatch event
    std::vector<int> entityIds = {vehId, bookingId};
    Event ev(amod::EVENT_DISPATCH, ++eventId, "VehicleDispatch", state.getCurrentTime(), entityIds);
    worldState->addEvent(ev);

    return amod::SUCCESS;
}

amod::ReturnCode SimulatorBasic::pickupCustomer(amod::World *worldState,
                                                int vehId, int custId,
                                                amod::VehicleStatus startStatus,
                                                amod::VehicleStatus endStatus,
                                                int bookingId) {
    // check that vehicle is at the same location as the customer
    Vehicle veh = state.getVehicle(vehId);
    if (!veh.getId()) {
        return amod::CANNOT_GET_VEHICLE;
    }

    Customer cust = state.getCustomer(custId);
    if (!custId) {
        return amod::CANNOT_GET_CUSTOMER;
    }

//        if (getDistance(veh.getPosition(), cust.getPosition()) > 1e-3 ) {
//            return amod::VEHICLE_NOT_AT_CUSTOMER_LOCATION;
//        }

    // add a pickup to simulate
    double pickupTime = state.getCurrentTime() + genRandTruncNormal(pickupParams);
    //if (getVerbose()) std::cout << "Future Pickup time : " << pickup_time << std::endl;

    int loc_id = 0;
    if (usingLocs) {
        // get the pickup location
        Location pickupLoc = locTree.findNN({cust.getPosition().x, cust.getPosition().y});
        loc_id = pickupLoc.getId();
    }
    Pickup p{bookingId, vehId, custId, loc_id, pickupTime, endStatus};
    pickups.emplace(pickupTime, p);

    // set the vehicle's start status
    veh.setStatus(startStatus);
    cust.setStatus(CustomerStatus::WAITING_FOR_PICKUP);
    worldState->setVehicle(veh);
    worldState->setCustomer(cust);

    state.setVehicle(veh);
    state.setCustomer(cust);

    return amod::SUCCESS;
}

amod::ReturnCode SimulatorBasic::dropoffCustomer(amod::World *worldState,
                                                 int vehID, int custId,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus,
                                                 int bookingId) {
    // check that vehicle is at the same location as the customer
    Vehicle veh = worldState->getVehicle(vehID);
    if (!veh.getId()) {
        return amod::CANNOT_GET_VEHICLE;
    }

    if (veh.getCustomerId() != custId) {
        return amod::VEHICLE_DOES_NOT_HAVE_CUSTOMER;
    }

    Customer cust = state.getCustomer(custId);
    if (!custId) {
        return amod::CANNOT_GET_CUSTOMER;
    }

//        if (getDistance(veh.getPosition(), cust.getPosition()) > 1e-3 ) {
//            return amod::VEHICLE_NOT_AT_CUSTOMER_LOCATION;
//        }

    // add a dropoff to simulate
    double dropoffTime = state.getCurrentTime() + genRandTruncNormal(dropoffParams);

    int locId = 0;
    if (usingLocs) {
        // get the dropoff location
        Location dropoffLoc = locTree.findNN({cust.getPosition().x, cust.getPosition().y});
        locId = dropoffLoc.getId();
    }

    //if (getVerbose()) std::cout << "Future Dropoff time : " << dropoff_time << std::endl;
    Dropoff doff{bookingId, vehID, custId, locId, dropoffTime, endStatus};
    dropoffs.emplace(dropoffTime, doff);

    // sets the status of the vehicle
    veh.setStatus(startStatus);
    worldState->setVehicle(veh);
    cust.setStatus(CustomerStatus::WAITING_FOR_DROPOFF);
    worldState->setCustomer(cust);

    state.setVehicle(veh);
    state.setCustomer(cust);

    // return success
    return amod::SUCCESS;
}

amod::ReturnCode SimulatorBasic::teleportCustomer(amod::World *worldState,
                                         int custId,
                                         const amod::Position &to,
                                         amod::CustomerStatus custStartStatus,
                                         amod::CustomerStatus custEndStatus
                                         ) {
    Customer *cust = state.getCustomerPtr(custId);
    if (!cust) {
        return amod::CANNOT_GET_CUSTOMER;
    }

    if (cust->getStatus() != CustomerStatus::FREE) {
        return amod::CUSTOMER_IS_NOT_FREE;
    }

    // set a teleporation arrival time
    double teleportTime = state.getCurrentTime() + genRandTruncNormal(teleportParams);

    int fromLocId = 0;

    if (usingLocs) {
        // get the teleport location
        Location teleportLoc = locTree.findNN({cust->getPosition().x, cust->getPosition().y});
        fromLocId = teleportLoc.getId();
    }

    int toLocId = 0;
    if (usingLocs) {
        // get the teleport location
        Location teleportLoc = locTree.findNN({to.x, to.y});
        toLocId = teleportLoc.getId();
    }



    Teleport tport{custId, toLocId, teleportTime, custEndStatus};

    teleports.emplace(teleportTime, tport);

    // sets the status of the vehicle
    cust->setStatus(custStartStatus);
    cust->setLocationId(0);

    // create a teleportation event
    std::vector<int> entityIds = {custId, fromLocId};
    Event ev(amod::EVENT_TELEPORT, ++eventId, "CustomerTeleport", state.getCurrentTime(), entityIds);
    worldState->addEvent(ev);

    // adjust locations
    // location specific changes

    if (usingLocs) {
        Location * ploc = worldState->getLocationPtr(fromLocId);

        ploc->removeCustomerId(custId);
        // trigger event
        Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                 "LocationCustSizeChange", state.getCurrentTime(),
                 {fromLocId});
        worldState->addEvent(ev);

        // update internal state;
        ploc = state.getLocationPtr(fromLocId);
        ploc->removeCustomerId(custId);
    }

    // set the internal state
    state.getCustomerPtr(custId)->setStatus(custStartStatus);



    return amod::SUCCESS;
}


void SimulatorBasic::setCustomerStatus(amod::World *worldState, int custId, CustomerStatus status) {
    Customer cust = worldState->getCustomer(custId);
    cust.setStatus(status);
    worldState->setCustomer(cust);
}


amod::ReturnCode SimulatorBasic::serviceBooking(amod::World *worldState, const amod::Booking &booking) {
    // add booking to internal structure
    bookings[booking.id] = booking;

    amod::ReturnCode rc;

    // make sure the customer is valid
    amod::Customer cust = worldState->getCustomer(booking.custId);
    if (!(cust.getStatus() == amod::CustomerStatus::FREE || cust.getStatus() == amod::CustomerStatus::WAITING_FOR_ASSIGNMENT)) {
        rc = amod::ReturnCode::CUSTOMER_IS_NOT_FREE;
    } else {

        // dispatch the vehicle to the customer's position
        amod::Position custPos = cust.getPosition();

        double distToDropoff = getDrivingDistance(custPos, booking.destination);
        if (distToDropoff < 0) {
            rc = amod::NO_PATH_TO_DESTINATION;
        } else {

            // dispatch the vehicle
            rc = dispatchVehicle(worldState, booking.vehId, custPos,
                    amod::VehicleStatus::MOVING_TO_PICKUP, amod::VehicleStatus::HIRED, booking.id);
        }
    }

    // if the return code was not successful, then we raise an event that the booking could not be serviced
    if (rc!= amod::SUCCESS) {
        // raise an event that this booking was dropped
        std::vector<int> entities = {booking.id, booking.custId};
        amod::Event ev(amod::EVENT_BOOKING_CANNOT_BE_SERVICED, ++eventId, "BookingDropped", worldState->getCurrentTime(), entities);
        worldState->addEvent(ev);
    }
    return rc;
}

double SimulatorBasic::getDrivingDistance(const amod::Position &from, const amod::Position &to) {
    return getDistance(from, to);
}

double SimulatorBasic::getDrivingDistance(int fromLocId, int toLocId) {
    if (fromLocId && toLocId) {
        return getDistance(state.getLocation(fromLocId).getPosition(), state.getLocation(toLocId).getPosition());
    }
}

double SimulatorBasic::getDistance(const amod::Position &from, const amod::Position &to) {
    return sqrt(pow( from.x - to.x ,2.0) + pow( from.y - to.y ,2.0));
}

void SimulatorBasic::simulateVehicles(amod::World *worldState) {
    // for all vehicles in dispatches_
    for (auto it = dispatches.begin(); it != dispatches.end(); ) {
        // move vehicle
        Vehicle veh = worldState->getVehicle(it->second.vehId);

        double dist = genRandTruncNormal(speedParams);

        double xDist = dist*(it->second.grad.x)*resolution;
        double yDist = dist*(it->second.grad.y)*resolution;

        it->second.curr.x += xDist;
        it->second.curr.y += yDist;

        veh.setPosition(it->second.curr);

        // set associated customer id and update
        int custId = veh.getCustomerId();
        Customer cust = Customer();
        if (custId) cust = worldState->getCustomer(custId);
        if (custId && cust.isInVehicle()) {
            cust.setPosition(it->second.curr);
        }

        // if vehicle (specified by the dispatch) has arrived
        if (hasArrived(it->second)) {
            // vehicle has arrived
            if (getVerbose()) Print() << veh.getId() << " has arrived at " <<
                                           it->second.to.x << " " << it->second.to.y <<
                                            " at time "  << worldState->getCurrentTime() << std::endl;
            it->second.curr = it->second.to;
            veh.setPosition(it->second.curr);

            // set associated customer position to the final destination
            if (custId && cust.isInVehicle()) {
                cust.setPosition(it->second.curr);
            }

            // trigger arrival event
            int bid = it->second.bookingId;
            std::vector<int> entities = {veh.getId()};

            if (custId && cust.isInVehicle()) {
                entities.push_back(custId);
            }

            if (bid) {
                entities.push_back(bid);
            }


            Event ev(amod::EVENT_ARRIVAL, ++eventId, "VehicleArrival", state.getCurrentTime(), entities);
            worldState->addEvent(ev);

            // set the vehicle status
            veh.setStatus(it->second.vehEndStatus);


            // update the location to indicate the vehicle is here.
            if (!bid && ((veh.getStatus() == amod::Vehicle::FREE) ||
                (veh.getStatus() == amod::Vehicle::PARKED))
            ) { //only if the vehicle is free
                int vehId = veh.getId();
                Dispatch dp = dispatches[vehId];
                int locId = dp.toLocId;
                amod::Location *ploc = worldState->getLocationPtr(locId);
                if (ploc == nullptr) {
                    if (verbose) Print() << "Location " << locId << " is nullptr!" << std::endl;
                    throw std::runtime_error("AMODSimulatorSimMobility: Location Pointer is nullptr");
                }

                ploc->addVehicleId(vehId);
                veh.setLocationId(locId);
                amod::Event ev(amod::EVENT_LOCATION_VEHS_SIZE_CHANGE, ++eventId,
                        "LocationVehSizeChange", state.getCurrentTime(),
                        {locId});
                worldState->addEvent(ev);

                // update the customer and trigger event if necessary
                if (cust.getId() && cust.isInVehicle()) {
                    amod::Location *ploc = worldState->getLocationPtr(locId);
                    ploc->addCustomerId(custId);
                    cust.setLocationId(locId);
                    amod::Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                            "LocationCustSizeChange", state.getCurrentTime(),
                            {locId});
                    worldState->addEvent(ev);
                }
            }

            // update the world state
            worldState->setVehicle(veh); //update the vehicle in the world state
            state.setVehicle(veh); // update the internal state

            worldState->setCustomer(cust);
            state.setCustomer(cust);

            dispatches.erase(it++);

            // if dispatch has non-zero booking id
            if (bid) {
                if (cust.isInVehicle()) {
                    //dropoff
                    //if (getVerbose()) std::cout << "Car has customer - dropping off" << cust.getId() << std::endl;
                    auto rc = dropoffCustomer(worldState, veh.getId(), cust.getId(),
                                              VehicleStatus::DROPPING_OFF, VehicleStatus::FREE, bid);
                    if (rc != amod::SUCCESS) {
                        throw std::runtime_error("Could not drop off customer");
                    }
                } else {
                    // pickup the customer
                    //if (getVerbose()) std::cout << "Car is empty - picking up " << cust.getId() << std::endl;
                    pickupCustomer(worldState, bookings[bid].vehId, bookings[bid].custId,
                                   VehicleStatus::PICKING_UP, VehicleStatus::FREE, bid);
                }
            }
        } else {
            ++it;
            //if (getVerbose()) std::cout << "Vehicle pos:" << veh.getPosition().x << " " << veh.getPosition().y << std::endl;
            std::vector<int> entities = {veh.getId()};

            // check if the customer is in the vehicle
            if (custId && cust.isInVehicle()) {
                entities.push_back(custId);
            }

            // trigger move event
            Event ev(amod::EVENT_MOVE, ++eventId, "VehicleMoved", state.getCurrentTime(), entities);
            worldState->addEvent(ev);

            // update the world_state and internal state
            worldState->setVehicle(veh); //update the vehicle in the world state
            state.setVehicle(veh);
            worldState->setCustomer(cust);
            state.setCustomer(cust);

        }
    }
}

void SimulatorBasic::simulateCustomers(amod::World *worldState) {
    // this is not necessary (at the moment) as performed using by simulateVehicles.
    return;
}

bool SimulatorBasic::hasArrived(const Dispatch &d) {
    //Position diff(d.to.x - d.curr.x, d.to.y - d.curr.y);
    //return !((sign(diff.x) == sign(d.grad.x)) && (sign(diff.x) == sign(d.grad.x)));
    double distToDest = getDistance(d.from, d.to);
    double distToCurr = getDistance(d.from, d.curr);
    return (distToCurr >= distToDest); // distance travelled larger or equal to distance to destination
}

void SimulatorBasic::simulatePickups(amod::World *worldState) {
    auto it = pickups.begin();
    while (it != pickups.end()) {
        if (it->first <= state.getCurrentTime()) {
            if (getVerbose()) Print() << it->second.vehId << " has picked up " <<
                                           it->second.custId << " at time " << it->first << std::endl;

            // create pickup event
            std::vector<int> entityIds = {it->second.vehId, it->second.custId};
            int bid = it->second.bookingId;
            if (bid) {
                entityIds.push_back(bid);
            }
            Event ev(amod::EVENT_PICKUP, ++eventId, "CustomerPickup", it->first, entityIds);
            worldState->addEvent(ev);

            Vehicle veh = worldState->getVehicle(it->second.vehId);
            veh.setCustomerId(it->second.custId);
            Customer cust = worldState->getCustomer(it->second.custId);
            cust.setAssignedVehicleId(it->second.vehId);
            cust.setInVehicle();
            cust.setLocationId(0);

            // sets vehicle state
            veh.setStatus(it->second.vehEndStatus);

            // update the world state and internal state
            worldState->setCustomer(cust);
            worldState->setVehicle(veh);
            state.setCustomer(cust);
            state.setVehicle(veh);


            if (bid) {
                ReturnCode rc = dispatchVehicle(worldState, veh.getId(), bookings[bid].destination,
                        VehicleStatus::MOVING_TO_DROPOFF, VehicleStatus::HIRED,
                                                bid);
                if (rc != amod::SUCCESS) {
                    if (getVerbose()) Print() << bookings[bid].destination.x << " " <<
                            bookings[bid].destination.y << std::endl;
                    if (getVerbose()) Print() << kErrorStrings[rc] << std::endl;
                    throw std::runtime_error("redispatch failed!");
                }
            }

            // erase item
            pickups.erase(it);
            it = pickups.begin();
        } else {
            break;
        }
    }
}

void SimulatorBasic::simulateDropoffs(amod::World *worldState) {
    auto it = dropoffs.begin();
    while (it != dropoffs.end()) {
        if (it->first <= state.getCurrentTime()) {
            // create dropoff event
            if (verbose) if (verbose) Print()  << it->second.vehId << " has dropped off " <<
                                                    it->second.custId << " at time " << it->first << std::endl;
            int locId = it->second.locId;
            int bid = it->second.bookingId;
            std::vector<int> entityIds = {it->second.vehId, it->second.custId};
            if (bid) entityIds.push_back(bid);
            amod::Event ev(amod::EVENT_DROPOFF, ++eventId, "CustomerDropoff", it->first, entityIds);
            worldState->addEvent(ev);

            amod::Vehicle *veh = worldState->getVehiclePtr(it->second.vehId);
            veh->clearCustomerId();
            veh->setStatus(it->second.vehEndStatus);
            veh->setLocationId(it->second.locId);

            amod::Customer *cust = worldState->getCustomerPtr(it->second.custId);
            cust->clearAssignedVehicleId();
            cust->setStatus(amod::CustomerStatus::FREE);
            cust->setLocationId(it->second.locId);

            // if is part of a booking, clear it since the vehicle has fropped off the custmer

            if (bid) {
              bookings.erase(bid);
            }

            // erase item
            dropoffs.erase(it);
            it = dropoffs.begin();

            // update locations
            // update the location to indicate the vehicle is here.
            {
                amod::Location *ploc = worldState->getLocationPtr(locId);
                ploc->addVehicleId(veh->getId());
                veh->setLocationId(locId);
                amod::Event ev(amod::EVENT_LOCATION_VEHS_SIZE_CHANGE, ++eventId,
                    "LocationVehSizeChange", state.getCurrentTime(),
                    {locId});
                worldState->addEvent(ev);

                // update the customer and trigger event if necessary
                if (cust && cust->isInVehicle()) {
                    amod::Location *ploc = worldState->getLocationPtr(locId);
                    ploc->addCustomerId(cust->getId());
                    cust->setLocationId(locId);
                    amod::Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                        "LocationCustSizeChange", state.getCurrentTime(),
                        {locId});
                    worldState->addEvent(ev);
                }
            }
        } else {
            break;
        }
    }
}


void SimulatorBasic::simulateTeleports(amod::World *worldState) {
    auto it = teleports.begin();
    while (it != teleports.end()) {
        if (it->first <= state.getCurrentTime()) {
            // create teleportation arrival event
            if (getVerbose()) Print() << it->second.custId << " has teleported to location " <<
                                           it->second.locId << " at time " << it->first << std::endl;

            std::vector<int> entityIds = {it->second.custId};
            Event ev(amod::EVENT_TELEPORT_ARRIVAL, ++eventId, "CustomerTeleportArrival", it->first, entityIds);
            worldState->addEvent(ev);

            Customer cust = worldState->getCustomer(it->second.custId);
            cust.setStatus(it->second.custEndStatus);
            cust.setPosition(worldState->getLocationPtr(it->second.locId)->getPosition());

            // update the customer and trigger event if necessary
            if (usingLocs) {
                int locId = it->second.locId;
                Location *ploc = worldState->getLocationPtr(locId);
                ploc->addCustomerId(it->second.custId);
                Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                         "LocationCustSizeChange", state.getCurrentTime(),
                         {locId});
                worldState->addEvent(ev);

                // update internal state
                ploc = state.getLocationPtr(locId);
                ploc->addCustomerId(it->second.custId);

                cust.setLocationId(locId);
            }
            // update the external world and internal state
            worldState->setCustomer(cust);
            state.setCustomer(cust);

            // erase item
            teleports.erase(it);
            it = teleports.begin();
        } else {
            break;
        }
    }
}


void SimulatorBasic::setPickupDistributionParams(double mean, double sd, double min, double max) {
    std::normal_distribution<>::param_type par{mean, sd};
    pickupParams.par = par;
    pickupParams.max = max;
    pickupParams.min = min;
}

void SimulatorBasic::setDropoffDistributionParams(double mean, double sd, double min, double max) {
    std::normal_distribution<>::param_type par{mean, sd};
    dropoffParams.par = par;
    dropoffParams.max = max;
    dropoffParams.min = min;
}

void SimulatorBasic::setVehicleSpeedParams(double mean, double sd, double min, double max) {

    std::normal_distribution<>::param_type par{mean, sd};
    speedParams.par = par;
    speedParams.max = max;
    speedParams.min = min;
}

void SimulatorBasic::setTeleportDistributionParams(double mean, double sd, double min, double max) {

    std::normal_distribution<>::param_type par{mean, sd};
    teleportParams.par = par;
    teleportParams.max = max;
    teleportParams.min = min;
}

double SimulatorBasic::genRandTruncNormal(TruncatedNormalParams &params) {
    normalDist.param(params.par);
    double r = normalDist(eng);
    if (r > params.max) r = params.max;
    if (r < params.min) r = params.min;
    return r;
}
}
}
