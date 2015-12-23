//
//  ManagerMatchRebalance.cpp
//  AMODBase
//
//  Created by Harold Soh on 29/3/15.
//  Copyright (c) 2015 Harold Soh. All rights reserved.
//

#include "ManagerMatchRebalance.hpp"
#include "logging/Log.hpp"

namespace sim_mob{

namespace amod {

ManagerMatchRebalance::ManagerMatchRebalance() :
    matchMethod(ASSIGNMENT),
    distanceCostFactor(1),
    waitingTimeCostFactor(1),
    outputMoveEvents(true),
    bookingsItr(bookings.begin()),
    useBookingsFile(false),
    matchingInterval(5),
    nextMatchingTime(matchingInterval),
    eventId(0),
    rebalancingInterval(0),
    nextRebalancingTime(0),
    useCurrentQueue(false) {
}

ManagerMatchRebalance::~ManagerMatchRebalance() {
    if (outFile.is_open()) outFile.close();
}

amod::ReturnCode ManagerMatchRebalance::init(World *worldState) {

    // get number of available vehicles
    std::unordered_map<int, Vehicle>::const_iterator beginItr, endItr;
    worldState->getVehicles(&beginItr, &endItr);
    for (auto vitr=beginItr; vitr != endItr; ++vitr) {
        if (vitr->second.getStatus() == VehicleStatus::FREE ||
            vitr->second.getStatus() == VehicleStatus::PARKED) {
            availableVehs.insert(vitr->second.getId());
        }
    }

    nextMatchingTime = worldState->getCurrentTime() + matchingInterval;
    return amod::SUCCESS;
}

bool ManagerMatchRebalance::isBookingValid(amod::World *world, const amod::Booking &bk) {
    // checks if there is a path from the source to the destination
    if (simulator->getDrivingDistance(bk.source, bk.destination) < 0) {
        return false;
    }

    // checks that the source is reacheable from all stations
    if (stations.size() > 0) {
        // assumption is that we are using stations and that vehicles are all
        // initially located at the stations.

        amod::Customer *cust = world->getCustomerPtr(bk.custId);
        if (cust == nullptr) return false;
        bool pathFound = false;
        for (auto itr = stations.begin(); itr !=  stations.end(); ++itr) {
            auto *l = &(itr->second);

            double distCost = -1;
            if (cust->getLocationId()) {
                distCost = simulator->getDrivingDistance(l->getId(), cust->getLocationId());
            } else {
                distCost = simulator->getDrivingDistance(l->getPosition(), cust->getPosition());
            }

            if (distCost >= 0) {
                pathFound = true;
                break;
            }
        }

        if (!pathFound) return false; //no path from any station to this point.
    }


    return true;
}

amod::ReturnCode ManagerMatchRebalance::update(World *worldState) {
    Simulator *sim = Manager::getSimulator();
    if (!sim) {
        return amod::SIMULATOR_IS_NULLPTR;
    }

    // get simulator time
    double currentTime = worldState->getCurrentTime();

    // get events
    std::vector<Event> events;
    worldState->getEvents(&events);
    if (outFile.is_open()) outFile.precision(10);
    // respond to events
    for (auto e:events) {
        if (e.type == EVENT_ARRIVAL ||  e.type == EVENT_DROPOFF) {
            amod::Vehicle veh = worldState->getVehicle(e.entityIds[0]);

            // make this vehicle available again
            if (veh.getStatus() == VehicleStatus::FREE || veh.getStatus() == VehicleStatus::PARKED) {
                if (verbose) Print() << "Making vehicle " << veh.getId() << " available for redispatch." << std::endl;
                availableVehs.insert(e.entityIds[0]);
            }

        }
    }


    // dispatch bookings by solving the matching problem
    //if (verbose_) std::cout << "Manager Current time: " << current_time << std::endl;
    updateBookingsFromFile(currentTime); // load bookings from file (will do nothing if not using file)
    bookingsItr = bookings.begin();
    while (bookingsItr != bookings.end()) {
        // check if the time is less
        if (bookingsItr->first <= currentTime) {

            if (bookingsItr->second.id == 0) {
                // erase the booking
                bookings.erase(bookingsItr);

                // set to the earliest booking
                bookingsItr = bookings.begin();
                continue;
            }

            // issue a booking received event
            Event ev(amod::EVENT_BOOKING_RECEIVED, --eventId,
                     "BookingReceived", worldState->getCurrentTime(),
                     {bookingsItr->second.id, bookingsItr->second.custId});
            worldState->addEvent(ev);


            // check that booking is valid
            if (!isBookingValid(worldState, bookingsItr->second)) {
                // issue a booking discarded event
                Event ev(amod::EVENT_BOOKING_CANNOT_BE_SERVICED, --eventId,
                         "BookingDiscarded", worldState->getCurrentTime(),
                         {bookingsItr->second.id, NO_SUITABLE_PATH});
                worldState->addEvent(ev);

                // erase and set to earliest booking
                bookings.erase(bookingsItr);
                bookingsItr = bookings.begin();
                continue;
            }


            // ensure that the customer is available (if not, we discard the booking)
            Customer *cust = worldState->getCustomerPtr(bookingsItr->second.custId);
            if (cust->getStatus() == CustomerStatus::FREE ||
                cust->getStatus() == CustomerStatus::WAITING_FOR_ASSIGNMENT) {

                // check for teleportation
                if (bookingsItr->second.travelMode == amod::Booking::TELEPORT) {
                    simulator->teleportCustomer(worldState, bookingsItr->second.custId, bookingsItr->second.destination);
                    bookings.erase(bookingsItr);

                    // set to the earliest booking
                    bookingsItr = bookings.begin();
                    continue;
                }

                // add this to the bookings_queue for matching
                bookingsQueue[bookingsItr->second.id] = bookingsItr->second;

                // assign this customer to a station
                if (stations.size() > 0) {
                    int st_id = getClosestStationId(cust->getPosition());
                    stations[st_id].addCustomerId(cust->getId());
                }
            } else {
                // issue a booking discarded event
                Event ev(amod::EVENT_BOOKING_CANNOT_BE_SERVICED, --eventId,
                         "BookingDiscarded", worldState->getCurrentTime(),
                         {bookingsItr->second.id, CUSTOMER_NOT_FREE});
                worldState->addEvent(ev);
            }

            // erase the booking
            bookings.erase(bookingsItr);

            // set to the earliest booking
            bookingsItr = bookings.begin();
        } else {
            break;
        }
    }

    if (nextMatchingTime <= worldState->getCurrentTime()) {
        // perform matching and increase next matching time
        nextMatchingTime = worldState->getCurrentTime() + matchingInterval;
        //std::cout << next_matching_time_ << std::endl;
        //if (verbose_) std::cout << world_state->getCurrentTime() << ": Before Queue Size : " << bookings_queue_.size() << std::endl;
        //if (verbose_) std::cout << world_state->getCurrentTime() << ": Available Vehicles: " << available_vehs_.size() << std::endl;
        amod::ReturnCode rc = amod::FAILED;
        if (matchMethod == GREEDY) {
            rc = solveMatchingGreedy(worldState);
        } else if (matchMethod == ASSIGNMENT) {
            rc = solveMatching(worldState);
        } else {
            throw std::runtime_error("No such matching method");
        }
        //if (verbose_) std::cout << world_state->getCurrentTime() << ": After Queue Size  : " << bookings_queue_.size() << std::endl;
        //if (verbose_) std::cout << world_state->getCurrentTime() << ": Available Vehicles: " << available_vehs_.size() << std::endl;

        // return if we encounter a failure
        if (rc != amod::SUCCESS) {
            return rc;
        }
    }


    if (nextRebalancingTime <= worldState->getCurrentTime()) {
        amod::ReturnCode rc = solveRebalancing(worldState);
        nextRebalancingTime = worldState->getCurrentTime() + rebalancingInterval;
        // return if we encounter a failure
        if (rc != amod::SUCCESS) {
            return rc;
        }
    }

    return amod::SUCCESS;
}

amod::ReturnCode ManagerMatchRebalance::loadBookings(const std::vector<Booking> &bookings_) {
    for (auto b : bookings_) {
        bookings.emplace( b.bookingTime, b);
    }

    bookingsItr = bookings.begin();

    return amod::SUCCESS;
}

amod::ReturnCode ManagerMatchRebalance::loadBookingsFromFile(const std::string &filename) {
    bookingFile.open(filename.c_str());
    if (!bookingFile.good()) {
        if (verbose) Print() << "Cannot read: " << filename << std::endl;
        return amod::ERROR_READING_BOOKINGS_FILE;
    }

    useBookingsFile = true;
    /*
    for (auto itr = bookings_.begin(); itr != bookings_.end(); itr++) {
        auto &b = itr->second;
        if (verbose_) std::cout << b.id << ": " << b.booking_time << " " << b.cust_id << " " << b.travel_mode << std::endl;
    }
    */

    return amod::SUCCESS;
}

amod::ReturnCode ManagerMatchRebalance::updateBookingsFromFile(double currTime) {
    if (!useBookingsFile) return amod::SUCCESS;

    if (!bookingFile.good()) {
        if (verbose) Print() << "Error reading Bookings file!" << std::endl;
        return amod::ERROR_READING_BOOKINGS_FILE;
    }

    // add the last thing we read because this wouldn't have been added the last time
    if (lastBookingRead.id != 0 && lastBookingRead.bookingTime <= currTime) {
        bookings.emplace(lastBookingRead.bookingTime, lastBookingRead);
        lastBookingRead = Booking();
    } else if (lastBookingRead.id != 0) {
        // have not reached the necessary time
        return amod::SUCCESS;
    }

    while (bookingFile.good()) {
        Booking b;
        bookingFile >> b.id >> b.bookingTime >> b.custId >> b.source.x >> b.source.y >> b.destination.x >> b.destination.y >> b.travelMode;
        //if (verbose_) std::cout << b.id << " " << b.booking_time << std::endl;
        if (b.id && bookingFile.good()) {
            if (b.bookingTime <= currTime) {
                bookings.emplace(b.bookingTime, b); //only positive booking ids allowed
            } else {
                // don't emplace it just yet
                lastBookingRead = b;
            }
        }
        if (b.bookingTime > currTime) {
            break; //assumes bookings file orders bookings by time
        }
    }

    //if (verbose_) std::cout << "Loaded " << bookings_.size() << " Bookings" <<
    //    "; last time: " << last_booking_read_.booking_time << ", " << last_booking_read_.id << std::endl;

    return amod::SUCCESS;
}

void ManagerMatchRebalance::setCostFactors(double distCostFactor, double waitingTimeCostFactor_) {
    distanceCostFactor = distCostFactor;
    waitingTimeCostFactor = waitingTimeCostFactor_;

}


void ManagerMatchRebalance::setMatchingInterval(double matchingInterval_) {
    matchingInterval = matchingInterval_;
}

double ManagerMatchRebalance::getMatchingInterval() const {
    return matchingInterval;
}

void ManagerMatchRebalance::setRebalancingInterval(double rebalancingInterval_) {
    rebalancingInterval = rebalancingInterval_;
    nextRebalancingTime = 0.0;
}

double ManagerMatchRebalance::getRebalancingInterval() const {
    return rebalancingInterval;
}

void ManagerMatchRebalance::loadStations(std::vector<amod::Location> &stations_, const amod::World &worldState)
{

    if (stations_.size() <= 0) return;

    // create a map for quick lookup based on id
    for (auto l : stations_) {
        stations[l.getId()] = l;
    }

    // create a tree for quick lookup of location ids
    stationsTree.build(stations_);

    // assign vehicles to stations
    std::unordered_map<int, Vehicle>::const_iterator beginItr, endItr;
    worldState.getVehicles(&beginItr, &endItr);

    for (auto itr = beginItr; itr != endItr; ++itr) {
        int stId = getClosestStationId(itr->second.getPosition());
        stations[stId].addVehicleId(itr->second.getId()); // vehicle belongs to this station.
        vehIdToStationId[itr->second.getId()] = stId;
    }
    return;
}

void ManagerMatchRebalance::setDemandEstimator(amod::DemandEstimator *sde) {
    demandEstimator = sde;
}


// **********************************************************
// PRIVATE FUNCTIONS
// **********************************************************

int ManagerMatchRebalance::getNumWaitingCustomers(amod::World *worldState, int locId) {

    if (!worldState) return 0;

    if (locId) { //loc_id > 0?
        int num_cust = 0;
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
                    num_cust++;
                }
            } else {
                throw std::runtime_error("Customer in location does not exist!");
            }
        }

        // return number of waiting customers
        return num_cust;
    }

    // if loc_id == 0, then we want all customers from all locations.
    // get waiting customers from all locations
    // get iterators for Locations
    std::unordered_map<int, Location>::const_iterator bitr, eitr;
    worldState->getLocations(&bitr, &eitr);
    int num_cust = 0;
    for (auto itr=bitr; itr!=eitr; itr++) {
        num_cust += getNumWaitingCustomers(worldState, itr->second.getId());
    }
    return num_cust;
}



// use glpk to solve
amod::ReturnCode ManagerMatchRebalance::solveMatching(amod::World *worldState) {

    if (!worldState) {
        throw std::runtime_error("solveMatching: world_state is nullptr!");
    }

    if (availableVehs.size() == 0) return amod::SUCCESS; // no vehicles to distribute
    if (bookingsQueue.size() == 0) return amod::SUCCESS; // no bookings to service


    // create variables for solving lp
    long nbookings = bookingsQueue.size();
    long nvehs = availableVehs.size();
    int nvars = nbookings*nvehs;


    // set up the problem
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "matching");
    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_cols(lp, nvars);

    // add the structural variables (decision variables)
    std::unordered_map<int, std::pair<int,int>> indexToIds;

    int k = 1;
    for (auto vitr = availableVehs.begin(); vitr != availableVehs.end(); ++vitr){
        // loop through bookings to get bookings that can be served by this vehicle
        for (auto bitr = bookingsQueue.begin(); bitr != bookingsQueue.end(); ++bitr) {
            // store the indices so we can find them again
            indexToIds[k] = {bitr->first, *vitr};

            // get cost
            Vehicle *veh = worldState->getVehiclePtr(*vitr);
            Customer *cust = worldState->getCustomerPtr(bitr->second.custId);

            double totalInvertCost = 0;
            double dist = 0;
            if (veh->getLocationId() && cust->getLocationId()) {
                dist = simulator->getDrivingDistance(veh->getLocationId(), cust->getLocationId());
            } else {
                dist = simulator->getDrivingDistance(veh->getPosition(), cust->getPosition());
            }

            double dist_cost = distanceCostFactor*(dist);
            if (dist_cost < 0) {
                // this vehicle cannot service this booking
                totalInvertCost = -1.0;
            } else {
                double time_cost = waitingTimeCostFactor*(std::max(0.0, worldState->getCurrentTime() - bitr->second.bookingTime));
                totalInvertCost = 1.0/(1.0 + dist_cost + time_cost);
            }

            // add this variable to the solver
            std::stringstream ss;
            ss << "x " << bitr->first << " " << *vitr;
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_col_name(lp, k, cstr);
            //glp_set_col_kind(lp, k, GLP_BV); // use this if you want to run this as a MIP
            glp_set_col_bnds(lp, k, GLP_DB, 0.0, 1.0);
            glp_set_obj_coef(lp, k, totalInvertCost);

            // increment index
            ++k;
        }
    }
    // setup the constraints
    k = 1;
    int ncons = nbookings + nvehs;
    int nelems = nbookings*nvehs*2;
    std::vector<int> ia(nelems+1);// +1 because glpk starts indexing at 1 (why? I don't know)
    std::vector<int> ja(nelems+1);
    std::vector<double> ar(nelems+1);

    glp_add_rows(lp, ncons);
    for (int i=1; i<=nvehs; ++i) {
        std::stringstream ss;
        ss << "veh " << i;
        const std::string& tmp = ss.str();
        const char* cstr = tmp.c_str();
        glp_set_row_name(lp, i, cstr);
        glp_set_row_bnds(lp, i, GLP_DB, 0.0, 1.0);

        for (int j=1; j<=nbookings; ++j) {
            ia[k] = i;
            ja[k] = (i-1)*nbookings + j;
            ar[k] = 1.0;
            ++k;
        }
    }

    for (int j=1; j<=nbookings; ++j) {
        std::stringstream ss;
        ss << "booking " << j;
        const std::string& tmp = ss.str();
        const char* cstr = tmp.c_str();
        glp_set_row_name(lp, nvehs+j, cstr);
        glp_set_row_bnds(lp, nvehs+j, GLP_DB, 0.0, 1.0);

        for (int i=1; i<=nvehs; ++i) {
            ia[k] = nvehs+j;
            ja[k] = (i-1)*nbookings + j;
            ar[k] = 1.0;
            ++k;
        }
    }

    // if (verbose_) std::cout << k << " " << ncons << std::endl;

    // load the matrix
    // if (verbose_) std::cout << "Loading the matrix" << std::endl;
    glp_load_matrix(lp, k-1, ia.data(), ja.data(), ar.data());

    // solve the problem
    // if (verbose_) std::cout << "Solving the problem" << std::endl;

    // integer program
    /*
    glp_iocp parm;
    glp_init_iocp(&parm);
    parm.presolve = GLP_ON;
    glp_intopt(lp, &parm);
    */
    // linear program
    if (!verbose) glp_term_out(GLP_OFF); // suppress terminal output
    glp_simplex(lp, nullptr);

    // print out the objective value
    //double z = glp_mip_obj_val(lp);
    //if (verbose_) std::cout << z << std::endl;

    // dispatch the vehicles
    for (int k=1; k<=nvars; ++k) {
        //int opt_x = glp_mip_col_val(lp, k); //to get result for integer program
        int opt_x = glp_get_col_prim(lp,k);
        if(opt_x > 0){

            // vehicle is assigned to this booking
            auto ids = indexToIds[k];
            int bid = ids.first;
            int vehId = ids.second;

            bookingsQueue[bid].vehId = vehId;
            amod::ReturnCode rc = simulator->serviceBooking(worldState, bookingsQueue[bid]);
            if (rc!= amod::SUCCESS) {
                if (verbose) Print() << amod::kErrorStrings[rc] << std::endl;
                Event ev(amod::EVENT_BOOKING_CANNOT_BE_SERVICED, --eventId, "BookingDiscarded", worldState->getCurrentTime(), {bid, SERVICE_BOOKING_FAILURE});
                worldState->addEvent(ev);
            } else {
                if (verbose) Print() << "Assigned " << vehId << " to booking " << bid << std::endl;
                // mark the car as no longer available
                availableVehs.erase(vehId);

                // change station ownership of vehicle
                if (stations.size() > 0) {
                    int stId = vehIdToStationId[vehId]; //old station
                    stations[stId].removeVehicleId(vehId);
                    int newStId = getClosestStationId( bookingsQueue[bid].destination ); //the station at the destination
                    stations[newStId].addVehicleId(vehId);
                    vehIdToStationId[vehId] = newStId;

                    // remove this customer from the station queue
                    stations[stId].removeCustomerId(bookingsQueue[bid].custId);
                }

                // issue a booking serviced event
                Event ev(amod::EVENT_BOOKING_SERVICED, --eventId, "BookingServiced", worldState->getCurrentTime(), {bid});
                worldState->addEvent(ev);

            }

            // erase the booking
            bookingsQueue.erase(bid);
        }
    }

    //double x1 = glp_mip_col_val(mip, 1);
    // housekeeping; clear up all the dynamically allocated memory
    glp_delete_prob(lp);
    glp_free_env();

    ia.clear();
    ja.clear();
    ar.clear();

    return amod::SUCCESS;
}

amod::ReturnCode ManagerMatchRebalance::solveMatchingGreedy(amod::World *worldState) {

    if (!worldState) {
        throw std::runtime_error("solveMatching: world_state is nullptr!");
    }

    if (availableVehs.size() == 0) return amod::SUCCESS; // no vehicles to distribute
    if (bookingsQueue.size() == 0) return amod::SUCCESS; // no bookings to service


    // create variables for solving lp
    long nbookings = bookingsQueue.size();
    long nvehs = availableVehs.size();

    // for each booking, find closest vehicle
    std::vector<int> toErase;
    for (auto bitr = bookingsQueue.begin(); bitr != bookingsQueue.end(); ++bitr) {
        double minDistCost = std::numeric_limits<int>::max();
        Vehicle *closestVeh = nullptr;
        Location *closestLoc = nullptr;
        Customer *cust = worldState->getCustomerPtr(bitr->second.custId);

        // either go through available vehicles or node locations (whichever is smaller)
        if (availableVehs.size() < worldState->getNumLocations()) {
            //if (verbose_) std::cout << "Looping through vehicles" << std::endl;
            for (auto vitr = availableVehs.begin(); vitr != availableVehs.end(); ++vitr){
                // get cost
                Vehicle *veh = worldState->getVehiclePtr(*vitr);
                double distCost = -1;
                if (veh->getLocationId() && cust->getLocationId()) {
                    distCost = simulator->getDrivingDistance(veh->getLocationId(), cust->getLocationId());
                } else {
                    distCost = simulator->getDrivingDistance(veh->getPosition(), cust->getPosition());
                }
                if (distCost >= 0 && minDistCost > distCost) {
                    closestVeh = veh;
                    minDistCost = distCost;
                }
            }
        } else {
            //if (verbose_) std::cout << "Looping through locations" << std::endl;
            // check for other locations
            std::unordered_map<int, Location>::const_iterator lbitr, leitr;
            worldState->getLocations(&lbitr, &leitr);
            for (auto itr = lbitr; itr != leitr; ++itr) {
                auto *l = &(itr->second);
                if (l->getNumVehicles() > 0) {

                    double distCost = -1;
                    if (cust->getLocationId()) {
                        distCost = simulator->getDrivingDistance(l->getId(), cust->getLocationId());
                    } else {
                        distCost = simulator->getDrivingDistance(l->getPosition(), cust->getPosition());
                    }

                    if (distCost >=0 && minDistCost > distCost) {
                        closestLoc = worldState->getLocationPtr(l->getId());
                        minDistCost = distCost;
                    }
                }
            }

            if (closestLoc != nullptr) {
                //get a vehicle
                std::unordered_set<int>::const_iterator vbitr, veitr;
                closestLoc->getVehicleIds(&vbitr, &veitr);
                if (vbitr != veitr) {
                    closestVeh = worldState->getVehiclePtr(*vbitr);
//                         if (verbose_) {
//                             std::cout << closest_veh->getPosition().x << " " << closest_veh->getPosition().y << " : " <<
//                             closest_loc->getPosition().x << " " << closest_loc->getPosition().y << std::endl;
//                         }
                }
            }
        }
        if (closestVeh != nullptr) {
            // assign vehicle to booking
            int vehId = closestVeh->getId();
            int bid = bitr->second.id;
            bookingsQueue[bid].vehId = vehId;
            amod::ReturnCode rc = simulator->serviceBooking(worldState, bookingsQueue[bid]);
            if (rc!= amod::SUCCESS) {
                if (verbose) Print() << amod::kErrorStrings[rc] << std::endl;
            } else {
                if (verbose) Print() << "Assigned " << vehId << " to booking " << bid << std::endl;
                // mark the car as no longer available
                availableVehs.erase(vehId);

                // change station ownership of vehicle
                if (stations.size() > 0) {
                    int stId = vehIdToStationId[vehId]; //old station
                    stations[stId].removeVehicleId(vehId);
                    int netStId = getClosestStationId( bookingsQueue[bid].destination ); //the station at the destination
                    stations[netStId].addVehicleId(vehId);
                    vehIdToStationId[vehId] = netStId;

                    // remove this customer from the station queue
                    stations[stId].removeCustomerId(bookingsQueue[bid].custId);
                }

            }

            // issue a booking serviced event
            Event ev(amod::EVENT_BOOKING_SERVICED, --eventId, "BookingServiced", worldState->getCurrentTime(), {bid});
            worldState->addEvent(ev);

            // mark booking to be erased
            toErase.emplace_back(bid);
        }

    }

    if (verbose) Print() << "Before: " << bookingsQueue.size() << " ";


    for (auto itr=toErase.begin(); itr!= toErase.end(); ++itr) {
        bookingsQueue.erase(*itr);
    }

    if (verbose) Print() << "After: " << bookingsQueue.size() << std::endl;

    return amod::SUCCESS;
}

amod::ReturnCode ManagerMatchRebalance::solveRebalancing(amod::World *worldState) {
    if (!worldState) {
        throw std::runtime_error("solveMatching: world_state is nullptr!");
    }

    if (availableVehs.size() == 0) {
        if (verbose) Print() << "No available vehicles to rebalance." << std::endl;
        return amod::SUCCESS; // no vehicles to rebalance
    }
    if (stations.size() == 0) {
        if (verbose) Print() << "No stations loaded." << std::endl;
        return amod::SUCCESS; // nothing to rebalance
    }
    // create variables for solving lp
    int nvehs = availableVehs.size();
    int nstations = stations.size();
    int nstationsServed = 0;
    int nvars = nstations*nstations; // how many to send from one station to another

    int viTotal = availableVehs.size();
    int cexTotal = 0;
    std::unordered_map<int, int> cex;
    std::unordered_map<int, std::set<int>> vi; // free vehicles at this station

    // set up the problem
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "rebalancing");
    glp_set_obj_dir(lp, GLP_MIN);
    glp_add_cols(lp, nvars);


    // add the structural variables (decision variables)
    std::unordered_map<int, std::pair<int,int>> indexToIds;
    std::unordered_map<std::pair<int,int>, int> idsToIndex;

    int k = 1;
    for (auto sitr = stations.begin(); sitr != stations.end(); ++sitr){
        for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
            // store the indices to make lookups easier (can be optimized in future iterations)
            indexToIds[k] = {sitr->first, sitr2->first};
            idsToIndex[std::make_pair(sitr->first, sitr2->first)] = k;

            // get cost
            double cost = simulator->getDrivingDistance(sitr->second.getPosition(),
                    sitr2->second.getPosition() );

            if (cost == -1) {
                // no route possible
                cost = 1e11; //some large number
            };

            // add this variable to the solver
            std::stringstream ss;
            ss << "x " << sitr->first << " " << sitr2->first;
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_col_name(lp, k, cstr);

            glp_set_col_bnds(lp, k, GLP_LO, 0.0, 0.0); // set lower bound of zero, no upperbound
            glp_set_obj_coef(lp, k, cost);

            // increment index
            ++k;
        }

        // compute variables for the lp

        // use current demand
        // int cexi = sitr->second.getNumCustomers() - sitr->second.getNumVehicles();

        // use predicted demand
        int stid =  sitr->second.getId();
        auto curr_time = worldState->getCurrentTime();
        auto pred = demandEstimator->predict(stid, *worldState, curr_time);


        int meanPred;

        if (useCurrentQueue) {
            meanPred = sitr->second.getNumCustomers();
        } else {
            meanPred = ceil(pred.first);
        }
        /*int mean_pred = ceil(std::max(
                (double) dem_est_->predict(sitr->second.getId(), *world_state, world_state->getCurrentTime()).first,
                (double) sitr->second.getNumCustomers()));
        */
        if (verbose) Print() << "Mean prediction: " << meanPred;

        int cexi = meanPred - sitr->second.getNumVehicles();
        if (verbose) Print() << "cexi: " << cexi;
        if (verbose) Print() << "vehs: " << sitr->second.getNumVehicles();

        cex[sitr->first] = cexi; // excess customers at this station
        cexTotal += cexi; // total number of excess customers

        if (cexi > 0) {
            nstationsServed++;
        }

        if (verbose) Print() << "cex[" << sitr->first << "]: " << cex[sitr->first] << std::endl;

    }

    // set up available vehicles at each station
    for (auto vitr = availableVehs.begin(); vitr != availableVehs.end(); ++vitr) {
        // get which station this vehicle belongs
        int sid = vehIdToStationId[*vitr];
        vi[sid].insert(*vitr);
    }

    // set up constraints
    std::vector<int> ia;
    std::vector<int> ja;
    std::vector<double> ar;
    if (cexTotal <= 0) {
        // should be possible to satisfy all customers by rebalancing
        int ncons = nstations*2;
        int nelems = nstations*((nstations - 1)*2) + nstations*(nstations-1);
        ia.resize(nelems+1);
        ja.resize(nelems+1); // +1 because glpk starts indexing at 1 (why? I don't know)
        ar.resize(nelems+1);

        glp_add_rows(lp, ncons);
        int k = 1;
        int i = 1;

        // constraint for net flow to match (or exceed) excess customers
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId();
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_LO, cex[sitr->second.getId()], 0.0);


            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;
                // from i to j
                ia[k] = i;
                int st_source = sitr->second.getId();
                int st_dest   = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(st_source, st_dest)];
                ar[k] = -1.0;
                ++k;

                // from j to i
                ia[k] = i;
                ja[k] = idsToIndex[std::make_pair(st_dest, st_source)];
                ar[k] = 1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint to make sure stations don't send more vehicles than they have
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId() << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            //if (verbose_) std::cout << "vi[" << sitr->first << "]: " <<  vi[sitr->second.getId()].size() << std::endl;
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr->second.getId()].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] = 1.0;
                ++k;
            }
            ++i; // increment i
        }

        glp_load_matrix(lp, nelems, ia.data(), ja.data(), ar.data());

    } else {
        // cannot satisfy all customers, rebalance to obtain even distribution
        // should be possible to satisfy all customers by rebalancing
        int ncons = nstations*3;
        int nelems = nstations*((nstations-1)*2) + 2*nstations*(nstations-1) ;
        ia.resize(nelems+1);
        ja.resize(nelems+1); // +1 because glpk starts indexing at 1 (why? I don't know)
        ar.resize(nelems+1);

        glp_add_rows(lp, ncons);
        int k = 1;
        int i = 1;

        // if (verbose_) std::cout << "Even distribution: " <<  floor(vi_total/nstations_underserved) << std::endl;
        // constraint for net flow to match (or exceed) excess customers
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId();
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_LO,
                             std::min((double) cex[sitr->second.getId()] ,
                                      (double) floor(viTotal/nstationsServed)), 0.0);


            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  -1.0;
                ++k;

                // from j to i
                ia[k] = i;
                ja[k] = idsToIndex[std::make_pair(stDest, stSrc)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint to make sure stations don't send more vehicles than they have
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId() << " veh constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            glp_set_row_bnds(lp, i, GLP_UP, 0.0, vi[sitr->second.getId()].size());

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                // from i to j
                if (sitr2->first == sitr->first) continue;

                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest   = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        // constraint for stations to send as many vehicles as possible
        for (auto sitr = stations.begin(); sitr!= stations.end(); ++sitr) {
            std::stringstream ss;
            ss << "st " << sitr->second.getId() << " send all constraint";
            const std::string& tmp = ss.str();
            const char* cstr = tmp.c_str();
            glp_set_row_name(lp, i, cstr);
            double constr = std::min( (double) vi[sitr->first].size(), (double) std::max(0, -cex[sitr->first] ));
            glp_set_row_bnds(lp, i, GLP_LO, constr, 0.0);

            for (auto sitr2 = stations.begin(); sitr2 != stations.end(); ++sitr2) {
                if (sitr2->first == sitr->first) continue;

                // from i to j
                ia[k] = i;
                int stSrc = sitr->second.getId();
                int stDest = sitr2->second.getId();
                ja[k] = idsToIndex[std::make_pair(stSrc, stDest)];
                ar[k] =  1.0;
                ++k;
            }
            ++i; // increment i
        }

        glp_load_matrix(lp, nelems, ia.data(), ja.data(), ar.data());
    }

    // solve the lp
    if (!verbose) glp_term_out(GLP_OFF); // suppress terminal output
    glp_simplex(lp, nullptr);


    // redispatch based on lp solution
    for (int k=1; k<=nvars; k++) {
        // get the value
        int toDispatch = floor(glp_get_col_prim(lp,k));
        //if (verbose_) std::cout << k << ": " << to_dispatch << std::endl;
        if (toDispatch > 0) {
            int stSrc = indexToIds[k].first;
            int stDest = indexToIds[k].second;

            // dispatch to_dispatch vehicles form station st_source to st_dest
            amod::ReturnCode rc = interStationDispatch(stSrc, stDest, toDispatch, worldState, vi);

            Event ev(amod::EVENT_REBALANCE, --eventId,
                  "Rebalancing", worldState->getCurrentTime(),
                   {stSrc, stDest, toDispatch});
            worldState->addEvent(ev);

            if (rc != amod::SUCCESS) {
                if (verbose) Print() << amod::kErrorStrings[rc] << std::endl;

                // housekeeping
                glp_delete_prob(lp);
                glp_free_env();

                ia.clear();
                ja.clear();
                ar.clear();

                // be stringent and throw an exception: this shouldn't happen
                throw std::runtime_error("solveRebalancing: interStationDispatch failed.");
            }
        }
    }

    // housekeeping
    glp_delete_prob(lp);
    glp_free_env();

    ia.clear();
    ja.clear();
    ar.clear();

    return amod::SUCCESS;
}


amod::ReturnCode ManagerMatchRebalance::interStationDispatch(int stSrc, int stDest,
                                                             int toDispatch,
                                                             amod::World *worldState,
                                                             std::unordered_map<int, std::set<int>> &vi) {
    // check that to_dispatch is positive
    if (toDispatch <= 0) return amod::FAILED;

    // check that st_source and st_dest are valid
    auto itrSrc = stations.find(stSrc);
    auto itrDest = stations.find(stDest);

    if (itrSrc == stations.end() || itrDest == stations.end() ) {
        return amod::INVALID_STATION_ID;
    }

    // dispatch vehicles
    auto itr = vi[stSrc].begin();
    for (int i=0; i<toDispatch; i++) {
        // find a free vehicle at station st_source
        int vehId = *itr;

        // send it to station st_dest
        if (verbose) Print() << "Rebalancing " << vehId << " from " << stSrc << " to " << stDest << std::endl;
        auto rc = simulator->dispatchVehicle(worldState, vehId , itrDest->second.getPosition(),
                                        VehicleStatus::MOVING_TO_REBALANCE, VehicleStatus::FREE);

        // if dispatch is success
        if (rc != amod::SUCCESS) {
            if (verbose) Print() << kErrorStrings[rc] << std::endl;
        }; // return with error code

        // change station ownership of vehicle
        stations[stSrc].removeVehicleId(vehId);
        stations[stDest].addVehicleId(vehId);
        vehIdToStationId[vehId] = stDest;
        availableVehs.erase(vehId);

        // mark vehicle as no longer available for dispatch
        vi[stSrc].erase(vehId);

        // increment iterator
        itr = vi[stSrc].begin();
    }

    return amod::SUCCESS;
}


int ManagerMatchRebalance::getClosestStationId(const amod::Position &pos) const {
    return stationsTree.findNN({pos.x, pos.y}).getId();
}

}

}
