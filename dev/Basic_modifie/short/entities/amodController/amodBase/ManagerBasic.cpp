//
//  ManagerBasic.cpp
//  AMODBase
//
//  Created by Harold Soh on 29/3/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#include "ManagerBasic.hpp"
#include "logging/Log.hpp"

namespace sim_mob{

namespace amod {
ManagerBasic::ManagerBasic() {
    return;
}

ManagerBasic::~ManagerBasic() {
    if (outFile.is_open()) outFile.close();
    return;
}

amod::ReturnCode ManagerBasic::init(World *worldState) {

    // get number of available vehicles
    numAvailVeh = 0;
    std::unordered_map<int, Vehicle>::const_iterator beginItr, endItr;
    worldState->getVehicles(&beginItr, &endItr);
    for (auto vitr=beginItr; vitr != endItr; ++vitr) {
        if (vitr->second.getStatus() == VehicleStatus::FREE ||
            vitr->second.getStatus() == VehicleStatus::PARKED) {
            ++numAvailVeh;
        }
    }


    return amod::SUCCESS;
}


amod::ReturnCode ManagerBasic::update(World *worldState) {
    Simulator *sim = Manager::getSimulator();
    if (!sim) {
        return amod::SIMULATOR_IS_NULLPTR;
    }

    // get simulator time
    double currTime = worldState->getCurrentTime();

    // get events
    // std::clock_t start = std::clock();
    // double duration = 0;

    std::vector<Event> events;
    worldState->getEvents(&events);
    if (outFile.is_open()) outFile.precision(10);
    // respond to events
    for (auto e:events) {

        if (e.type == EVENT_DROPOFF) {
            ++numAvailVeh;
        }

    }
    // clear events
    worldState->clearEvents();

    // dispatch bookings
    auto itr = bookings.begin();
    int numSkipped = 0;
    int numProcessed = 0;
    std::list<std::list<Booking>::iterator> toErase;
    while (itr != bookings.end()) {

        Booking bk = *itr;

        // check if the time is less
        if (bk.bookingTime <= currTime) {
            ++numProcessed;
            // Get the relevant customer
            Customer cust = worldState->getCustomer(bk.custId);
            if (!cust.getId()) {
                // invalid customer id
                // delete this booking
                Print() << "Invalid customer id. Deleting booking " << bk.id << std::endl;
                toErase.emplace_back(itr);
                ++itr;
                continue;
            }

            // check that customer is free
            if (!(cust.getStatus() == CustomerStatus::FREE ||
                  cust.getStatus() == CustomerStatus::WAITING_FOR_ASSIGNMENT)) {
                // customer is not free or not waiting for assignment
                // we skip this booking
                //std::cout << "Booking type: " << itr->second.travel_mode << std::endl;
                //std::cout << "Cust " << cust.getId() << " is not free, status " << cust.getStatus() << std::endl;
                ++itr;
                ++numSkipped;
                continue;
            } else {
                //std::cout << "Cust " << cust.getId() << " is free. Proceeding to assign. ";
            }

            // check for teleportation
            if (bk.travelMode == amod::Booking::TELEPORT) {
                simulator->teleportCustomer(worldState, bk.custId, bk.destination);
                toErase.emplace_back(itr);
                ++itr;
                continue;
            }



            // check if we have vehicles to dispatch
            //std::cout << world_state->getCurrentTime() << ": Num Available Veh: " << num_avail_veh_ << std::endl;
            if (numAvailVeh == 0) {
                simulator->setCustomerStatus(worldState, bk.custId,
                                        amod::CustomerStatus::WAITING_FOR_ASSIGNMENT);
                itr++;
                continue;
            }


            // assign a vehicle to this customer booking

            // find closest free vehicle
            // simple iterative method


            double minDist = -1;
            int bestVehId = 0;
            std::unordered_map<int, Vehicle>::const_iterator beginItr, endItr;
            worldState->getVehicles(&beginItr, &endItr);
            for (auto vitr=beginItr; vitr != endItr; ++vitr) {
                double dist = sim->getDistance(vitr->second.getPosition(), cust.getPosition());
                if (minDist < 0 || dist < minDist) {
                    amod::VehicleStatus status = vitr->second.getStatus();
                    if (status == VehicleStatus::PARKED || status == VehicleStatus::FREE ) {
                        minDist = dist;
                        bestVehId = vitr->second.getId();
                    }
                }
            }

            if (bestVehId) {
                bk.vehId = bestVehId;

                // tell the simulator to service this booking
                //std::cout << "... Assign bk " << bk.id << " car " << best_veh_id << std::endl;
                amod::ReturnCode rc = sim->serviceBooking(worldState, bk);
                if (rc!= amod::SUCCESS) {
                    // destroy booking and print error code
                    std::cout << amod::kErrorStrings[rc] << std::endl;
                } else {
                    --numAvailVeh;
                }

                // erase the booking
                toErase.emplace_back(itr);
                ++itr;
            } else {
                // cannot find a proper vehicle, move to next booking
                //std::cout << "... no car found " << std::endl;
                simulator->setCustomerStatus(worldState, bk.custId,
                                        amod::CustomerStatus::WAITING_FOR_ASSIGNMENT);
                ++itr;
            }



        } else {
            // exceeded the current time
            break;
        }

    }

    // erase the bookings that were serviced/removed
    for (auto vitr=toErase.begin(); vitr!=toErase.end(); ++vitr) {
        bookings.erase(*vitr);
    }

//        duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//        std::cout<<"Booking processing (" << num_processed <<   ") : "<< duration <<'\n';
//        std::cout<<" processing : "<< duration << ",   ";

    // output
    if (int numWaitingCost = getNumWaitingCustomers(worldState)) {
        Print() << worldState->getCurrentTime() << ": " << "Queue Size: " << numWaitingCost <<
                " Num Skipped: " << numSkipped << std::endl;
    }
    return amod::SUCCESS;
}

amod::ReturnCode ManagerBasic::loadBookings(const std::vector<Booking> &bookings_) {
    for (auto b : bookings_) {
        bookings.emplace_back(b);
    }

    bookings.sort();

    return amod::SUCCESS;
}

amod::ReturnCode ManagerBasic::loadBookingsFromFile(const std::string &filename) {
    std::ifstream in(filename.c_str());
    if (!in.good()) {
        Print() << "Cannot read: " << filename << std::endl;
        return amod::ERROR_READING_BOOKINGS_FILE;
    }

    while (in.good()) {
        Booking b;
        in >> b.id >> b.bookingTime >> b.custId >> b.source.x >> b.source.y >> b.destination.x >> b.destination.y >> b.travelMode;
        if (b.id && in.good()) bookings.emplace_back( b); //only positive booking ids allowed
    }
    /*
     for (auto itr = bookings_.begin(); itr != bookings_.end(); itr++) {
     auto &b = itr->second;
     std::cout << b.id << ": " << b.booking_time << " " << b.cust_id << " " << b.travel_mode << std::endl;
     }
     */
    bookings.sort();
    return amod::SUCCESS;
}


int ManagerBasic::getNumWaitingCustomers(amod::World *worldState, int locId) {

    if (!worldState) return 0;

    if (locId) { //loc_id > 0?
        int numCust = 0;
        // get iterators
        std::unordered_set<int>::const_iterator bitr, eitr;
        Location *ploc = worldState->getLocationPtr(locId);
        if (ploc) {
            ploc->getCustomerIds(&bitr, &eitr);
        } else {
            // incorrect loc_id
            return 0;
        }

        // loop through all customers at location specific by loc_id
        for (auto itr = bitr; itr != eitr; ++itr) {
            int custId = *itr; // get customer id
            // get customer pointer and check status
            Customer *cust = worldState->getCustomerPtr(custId);
            if (cust) {
                if (cust->getStatus() == CustomerStatus::WAITING_FOR_ASSIGNMENT) {
                    numCust++;
                }
            } else {
                throw std::runtime_error("Customer in location does not exist!");
            }
        }

        // return number of waiting customers
        return numCust;
    }

    // if loc_id == 0, then we want all customers from all locations.
    // get waiting customers from all locations
    // get iterators for Locations
    std::unordered_map<int, Location>::const_iterator bitr, eitr;
    worldState->getLocations(&bitr, &eitr);
    int numCust = 0;
    for (auto itr=bitr; itr!=eitr; itr++) {
        numCust += getNumWaitingCustomers(worldState, itr->second.getId());
    }
    return numCust;
}
    
}

}
