/*
 * AMODSimulatorSimMobility.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#include <entities/amodController/AMODSimulatorSimMobility.hpp>

namespace{
// helper function
int getNumberFromAimsunId(std::string &aimsunid)
{
    //"aimsun-id":"69324",
    std::string number;
    boost::regex expr (".*\"aimsun-id\":\"([0-9]+)\".*$");
    boost::smatch matches;
    if (boost::regex_match(aimsunid, matches, expr))
    {
        number  = std::string(matches[1].first, matches[1].second);
    }
    else
    {
        // Warn()<<"aimsun id not correct "+aimsunid << std::endl;
    }

    return std::stoi(number);
}
}

namespace sim_mob {
namespace amod{



AMODSimulatorSimMobility::AMODSimulatorSimMobility(sim_mob::Agent *parentController_):
    stDir(nullptr),
    currentTime(0),
    eventId(0),
    tripId(0),
    updDistFile(false),
    parentController(parentController_),
    baseErrMsg("ERROR: AMODSimulatorSimMobility::")
{
    // TODO Auto-generated constructor stub

}

AMODSimulatorSimMobility::~AMODSimulatorSimMobility() {
    // TODO Auto-generated destructor stub
    if (distFile.is_open()) distFile.close();
    if(waitingTimeFile.is_open()) waitingTimeFile.close();
}


amod::ReturnCode AMODSimulatorSimMobility::init(amod::World *worldState) {
    stDir = &(sim_mob::StreetDirectory::Instance());

    const RoadNetwork& roadNetwork = *(RoadNetwork::getInstance());
    nodePool = roadNetwork.getMapOfIdvsNodes();

    // create a segment pool
//            for(auto it = road_network.getLinks().begin(), it_end(road_network.getLinks().end()); it != it_end ; it ++)
//            {
//                for(auto seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++)
//                {
//                    if (!(*seg_it)->originalDB_ID.getLogItem().empty())
//                    {
//                        std::string aimsunId = (*seg_it)->originalDB_ID.getLogItem();
//                        int seg_id = getNumberFromAimsunId(aimsunId);
//                        seg_pool_.insert(std::make_pair(seg_id,*seg_it));
//                    }
//                }
//            }

//             for(auto it=uninodes_pool.begin(); it!=uninodes_pool.end(); ++it)
//             {
//                 sim_mob::Node* n = (*it);
//                 if (!n->originalDB_ID.getLogItem().empty())
//                 {
//                     std::string aimsum_id = n->originalDB_ID.getLogItem();
//                     int id = getNumberFromAimsunId(aimsum_id);
//                     node_pool_.insert(std::make_pair(id,n));
//                 }
//             }
//             std::cout << "uninodes: " << uninodes_pool.size() << std::endl;

    // go through the nodes to ensure that they are not a source or sink node
    std::unordered_map<const Node *, bool> hasOutgoing;
    std::unordered_map<const Node *, bool> hasIncoming;

    // go through all the road segments and mark the nodes
    for(auto it : roadNetwork.getMapOfIdVsLinks())
    {
        const Node* startNode = it.second->getFromNode();
		const Node* endNode = it.second->getToNode();
		hasOutgoing[startNode] = true;
		hasIncoming[endNode] = true;

    }

    std::unordered_set<int> nodesToRemove;
    for (auto it = nodePool.begin(); it!= nodePool.end(); ++it) {
        const Node* n = it->second;
        auto id = it->first;
        auto outItr = hasOutgoing.find(n);
        auto inItr = hasIncoming.find(n);
        if (outItr == hasOutgoing.end() || inItr == hasIncoming.end()) {
            // mark node for removal
            nodesToRemove.insert(id);
        }
    }

    if (verbose) Print()  << "Removing " << nodesToRemove.size() << " source/sink nodes" << std::endl;
    for (auto it=nodesToRemove.begin(); it!= nodesToRemove.end(); ++it) {
        nodePool.erase(*it);
    }


    //TEST
    // output all nodes
    if (saveNodesFileName != "") {
        std::ofstream testout(saveNodesFileName);
        testout << std::setprecision(10);
        for (auto itr=nodePool.begin(); itr != nodePool.end(); ++itr) {
            int nodeId = itr->first;
            double xInMeters = itr->second->getLocation().getX();
            double yInMeters = itr->second->getLocation().getY();
            testout << nodeId << " " << xInMeters << " " << yInMeters<< std::endl;
        }
        testout.close();
    }
    //TEST

    Print() <<  nodePool.size() << " nodes in accessible network." << std::endl; // we always show this
    // put the nodes locations into the world state
    std::vector<amod::Location> locs;
    double minX = DBL_MAX;
    double minY = DBL_MAX;
    double maxX = -1;
    double maxY = -1;
    for (auto it = nodePool.begin(); it!= nodePool.end(); ++it) {

        amod::Location l;
        // id
        l.setId(it->first);

        // position
        double xInMeters = it->second->getLocation().getX();
        double yInMeters = it->second->getLocation().getY();
        l.setPosition(amod::Position(xInMeters, yInMeters));

        // name
        std::stringstream ss;
        ss << it->first;
        l.setName(ss.str());

        // push onto locations
        worldState->addLocation(l);
        locs.push_back(l);

        // get maximum and minimum ranges
        minX = std::min(minX, xInMeters);
        maxX = std::max(maxX, yInMeters);
        minY  = std::min(minY , yInMeters);
        maxY  = std::max(minY , yInMeters);
    }

    //if (verbose) std::cout .precision(10);
    if (verbose) Print()  << "X Range: " << minX << " " << maxX << std::endl;
    if (verbose) Print()  << "Y Range: " << minY  << " " << maxY  << std::endl;


    // build KDTree
    locTree.build(locs);

    // for each node,


    // put all the vehicles and customers into valid locations
    // set the customer positions to be in valid locations
    std::unordered_map<int, amod::Customer>::const_iterator bitr, eitr;
    worldState->getCustomers(&bitr, &eitr);
    for (auto itr = bitr; itr!=eitr; ++itr) {

        amod::Customer * pcust = worldState->getCustomerPtr(itr->first);
        amod::Location loc = locTree.findNN({pcust->getPosition().x, pcust->getPosition().y});
        pcust->setPosition(loc.getPosition());

        // add this customer to the location
        amod::Location *ploc = worldState->getLocationPtr(loc.getId());
        ploc->addCustomerId(pcust->getId());
        pcust->setLocationId(loc.getId());
    }

    // set the vehicle positions to be in valid locations
    std::unordered_map<int, amod::Vehicle>::const_iterator vbitr, veitr;
    worldState->getVehicles(&vbitr, &veitr);
    for (auto itr = vbitr; itr!=veitr; ++itr) {

        amod::Vehicle * pveh = worldState->getVehiclePtr(itr->first);
        amod::Location loc = locTree.findNN({pveh->getPosition().x, pveh->getPosition().y});
        pveh->setPosition(loc.getPosition());

        // add this vehicle to the location
        amod::Location *ploc = worldState->getLocationPtr(loc.getId());
        ploc->addVehicleId(pveh->getId());
        pveh->setLocationId(loc.getId());
    }

    return amod::SUCCESS;
}

amod::ReturnCode AMODSimulatorSimMobility::update(amod::World *worldState) {
    // Simulate all the individual components
    worldState->setCurrentTime( getCurrentTime() );

    simulateVehicles(worldState);
    simulatePickups(worldState);
    simulateDropoffs(worldState);
    simulateTeleports(worldState);
    simulateCustomers(worldState);

    return amod::SUCCESS;
}

amod::ReturnCode AMODSimulatorSimMobility::dispatchVehicle(amod::World *worldState,
                                                 int vehId,
                                                 const amod::Position &to,
                                                 amod::VehicleStatus vehStartStatus,
                                                 amod::VehicleStatus vehEndStatus
                                                 ) {
    return dispatchVehicle(worldState, vehId, to, vehStartStatus, vehEndStatus, 0);
}

amod::ReturnCode AMODSimulatorSimMobility::pickupCustomer(amod::World *worldState,
                                                int vehId, int custId,
                                                amod::VehicleStatus startStatus,
                                                amod::VehicleStatus endStatus
                                                ) {
    return pickupCustomer(worldState, vehId, custId, startStatus, endStatus, 0);
}

amod::ReturnCode AMODSimulatorSimMobility::dropoffCustomer(amod::World *worldState,
                                                 int vehId, int custId,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus
                                                 ) {
    return dropoffCustomer(worldState, vehId, custId, startStatus, endStatus, 0);
}




amod::ReturnCode AMODSimulatorSimMobility::serviceBooking(amod::World *worldState, const amod::Booking &booking) {
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

        // check that customer is at source position
        if (custPos != booking.source) {
            rc = amod::CUSTOMER_NOT_AT_SOURCE;
        }


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
        std::vector<int> entities = {booking.id, booking.custId, rc};
        amod::Event ev(amod::EVENT_BOOKING_CANNOT_BE_SERVICED, ++eventId, "BookingDiscarded", getCurrentTime(), entities);
        worldState->addEvent(ev);

    }
    return rc;
}

   // DEPRECATED
//         void AMODSimulatorSimMobility::preComputeDrivingDistances() {
//             // for each location
//             int k = 0;
//             std::ofstream  fout("ecbd_shortest_distances.txt", std::ios::binary);
//             
//             int ns = node_pool_.size();
//             fout.write(reinterpret_cast<const char*>(&ns), sizeof ns);
//             
//             for (auto itr=node_pool_.begin(); itr!=node_pool_.end(); ++itr) {
//                 int source = itr->first;
//                 fout.write(reinterpret_cast<const char*>(&source), sizeof source);
//             }
//             
//             for (auto itr=node_pool_.begin(); itr!=node_pool_.end(); ++itr){
//                 // compute distance to all other locations
//                 std::cout << ++k << " of " << node_pool_.size() << std::endl;
//                 for (auto itr2=node_pool_.begin(); itr2!= node_pool_.end(); ++itr2) {
//                     //fout << itr->first << " " << itr2->first << " " << getDrivingDistance(itr->first, itr2->first) << std::endl;
//                     double dist = getDrivingDistance(itr->first, itr2->first);
//                     fout.write(reinterpret_cast<const char*>(&dist), sizeof dist);
//                 }
//             }
//             
//             fout.close();
//             
//             return;
//         }

void AMODSimulatorSimMobility::loadPreComputedDistances(const std::string &filename, bool updtDistances) {
    distFile.open(filename.c_str(), std::fstream::in);
    if (!distFile.is_open()) {
        Print() << "Cannot load precomputed distances" << std::endl;
        return;
    }

    distFile.precision(10);
    while (distFile.good()) {
        int source, dest;
        double distance;
        distFile >> source >> dest >> distance;
        if (distFile.good() && source) {
            drivingDistances[source][dest] = distance;
        }
    }

    if (updtDistances) {
        distFile.close();
        distFile.open(filename.c_str(), std::fstream::out | std::fstream::app );
        distFile.seekg(std::ios_base::end);
        updDistFile = updtDistances;
        distFile.flush();
    } else {
        distFile.close();
    }
}

double AMODSimulatorSimMobility::getDrivingDistance(const amod::Position &from, const amod::Position &to) {
    // get closest node from
    amod::Location fromLoc = locTree.findNN({from.x, from.y});
    // get closest node to
    amod::Location toLoc = locTree.findNN({to.x, to.y});
    // return distance
    return getDrivingDistance(fromLoc.getId(), toLoc.getId());

}

double AMODSimulatorSimMobility::getDrivingDistance(int fromLocId, int toLocId) {
    if (fromLocId == toLocId) return 0;

    // create waypoints
    auto fromItr = nodePool.find(fromLocId);
    auto toItr = nodePool.find(toLocId);

    // check that iterators are valid
    if (fromItr == nodePool.end() || toItr == nodePool.end()) return -1;

    // check to see if our map has this cached
    auto fromCacheItr = drivingDistances.find(fromLocId);
    if (fromCacheItr != drivingDistances.end()) {
        auto toCacheItr = fromCacheItr->second.find(toLocId);
        if (toCacheItr != fromCacheItr->second.end()) {
            return toCacheItr->second; //return the pre-computed distance
        }
    }

    // if we're here, we have not computed this distance before
    std::vector<const sim_mob::Link*> blacklist;

    std::vector < WayPoint > wp = stDir->SearchShortestDrivingPath(*fromItr->second,
               *toItr->second,
               blacklist);

    double distance = 0;
    if (wp.size() == 0) {
        // cannot generate such a path // no path to destination
        distance = -1;
    } else {
        for(int i=0;i<wp.size();++i)
        {
            if(wp[i].type == WayPoint::ROAD_SEGMENT )
            {
                const sim_mob::RoadSegment* rs = wp[i].roadSegment;
                distance += rs->getLength();
                //Print() <<"from node: "<<rs->getStart()->originalDB_ID.getLogItem()<<" to node: "<<rs->getEnd()->originalDB_ID.getLogItem()<<std::endl;
            }
        }
    }
    drivingDistances[fromLocId][toLocId] = distance; //cache this value for the future

    // save this distance to disk
    if (updDistFile) {
         if (distFile.is_open()) distFile << fromLocId << " " << toLocId << " " << distance << std::endl;
    }

    return distance;
}


double AMODSimulatorSimMobility::getDistance(const amod::Position &from, const amod::Position &to) {
    return sqrt(pow( from.x - to.x ,2.0) + pow( from.y - to.y ,2.0));
}


/*
 * INTERNAL SIMULATION FUNCTIONS
 */

void AMODSimulatorSimMobility::simulateVehicles(amod::World *worldState) {
    // TODO
    // loop through all arrivals and set the vehicle positions
    arrivalsMtx.lock();
    for (auto it=arrivals.begin(); it != arrivals.end(); ++it) {
        // update status and position of vehicles internally
        int vehId = it->first;
        double arrivalTime = it->second;

        // get relevant dispatch
        Dispatch dp = dispatches[vehId];

        // get the vehicle and any associated customer
        amod::Vehicle *veh = worldState->getVehiclePtr(vehId);
        if (!veh) {
            if (verbose) Print() << "Vehicle ptr is nullptr" << std::endl;
            throw std::runtime_error("simulateVehicles: veh is nullptr");
        }

        int custId = veh->getCustomerId();
        amod::Customer *cust = nullptr;
        if (custId) {
            cust = worldState->getCustomerPtr(custId);
            if (!cust) {
                if (verbose) Print() << "Customer ptr is nullptr" << std::endl;
                throw std::runtime_error("simulateVehicles: cust is nullptr");
            }
        }

        if (verbose) if (verbose) Print()  << veh->getId() << " has arrived at " << dp.to.x << " " << dp.to.y <<
                " at time "  << getCurrentTime() << std::endl;

        // set the vehicle position to the final destination
        veh->setPosition(dp.to);
        if (cust && cust->isInVehicle()) cust->setPosition(dp.to);

        // trigger arrival event
        int bid = dp.bookingId;
        std::vector<int> entities = {vehId};
        if (bid) entities.push_back(bid);

        amod::Event ev(amod::EVENT_ARRIVAL, ++eventId, "VehicleArrival", getCurrentTime(), entities);
        worldState->addEvent(ev);

        // set the vehicle status
        veh->setStatus(dp.vehEndStatus);

        // update the location to indicate the vehicle is here.
        if (!bid && ((veh->getStatus() == amod::Vehicle::FREE) ||
            (veh->getStatus() == amod::Vehicle::PARKED))
        ) { //only if the vehicle is free
            int locId = dp.toLocId;
            amod::Location *ploc = worldState->getLocationPtr(locId);
            if (ploc == nullptr) {
                if (verbose) Print() << "Location " << locId << " is nullptr!" << std::endl;
                throw std::runtime_error("AMODSimulatorSimMobility: Location Pointer is nullptr");
            }

            ploc->addVehicleId(vehId);
            veh->setLocationId(locId);
            amod::Event ev(amod::EVENT_LOCATION_VEHS_SIZE_CHANGE, ++eventId,
                    "LocationVehSizeChange", getCurrentTime(),
                    {locId});
            worldState->addEvent(ev);

            // update the customer and trigger event if necessary
            if (cust && cust->isInVehicle()) {
                amod::Location *ploc = worldState->getLocationPtr(locId);
                ploc->addCustomerId(custId);
                cust->setLocationId(locId);
                amod::Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                        "LocationCustSizeChange", getCurrentTime(),
                        {locId});
                worldState->addEvent(ev);
            }
        }

        // get the booking id


        // erase the dispatch since the vehicle is no longer on the road
        auto itr = dispatches.find(vehId);
        dispatches.erase(itr);

        // if dispatch has non-zero booking id
        if (bid) {
            amod::Customer *cust = worldState->getCustomerPtr(bookings[bid].custId);
            if (cust && cust->isInVehicle()) {
                //dropoff
                //if (verbose_) std::cout  << "Car has customer - dropping off" << cust.getId() << std::endl;
                auto rc = dropoffCustomer(worldState, vehId, custId,
                        amod::VehicleStatus::DROPPING_OFF,
                        amod::VehicleStatus::FREE, bid);
                if (rc != amod::SUCCESS) {
                    throw std::runtime_error("Could not drop off customer");
                }
            } else {
                // pickup the customer
                //if (verbose_) std::cout  << "Car is empty - picking up " << cust.getId() << std::endl;
                pickupCustomer(worldState, bookings[bid].vehId,
                        bookings[bid].custId,
                        amod::VehicleStatus::PICKING_UP,
                        amod::VehicleStatus::FREE, bid);
            }
        }

    }

    arrivals.clear(); // clear the arrivals
    arrivalsMtx.unlock();
    // loop through all dispatches
    for (auto it=dispatches.begin(); it!= dispatches.end(); ++it) {

        // update status and position of vehicle internally
        // get the position of the vehicle within simmobility
        int vehId = it->first;
        Dispatch &dp = (it->second);

        amod::Vehicle *veh = worldState->getVehiclePtr(vehId);
        if (!veh) {
            throw std::runtime_error("simulateVehicles: veh is nullptr");
        }

        // check that vehicle is alive in simmobility
        bool isalive = checkVehicleAlive(vehId);
        if (!isalive && !dp.arrived) {
            //if (verbose_) {
            // fow now, we always want to print this
                //auto oldPrecision = std::cout.precision(10);
                Print() << "update: Simmobility is destroying my vehicle!" << std::endl;
                Print() << "-----------------------------------------------------" << std::endl;
                Print() << "Current time: " << getCurrentTime() << " seconds" << std::endl;
                Print() << "Vehicle ID: " << vehId << std::endl;
                Print() << "Last known position: " << veh->getPosition().x << " " << veh->getPosition().y << std::endl;
                Print() << "Dispatch ids from: " << dp.fromLocId << " to " << dp.toLocId << std::endl;
                Print() << "Source pos: " << dp.from.x << ", " << dp.from.y << std::endl;
                Print() << "Destination pos: " << dp.to.x << ", " << dp.to.y << std::endl;
                Print() << "-----------------------------------------------------" << std::endl;

                //std::cout.precision(oldPrecision);
            //}

            // Gracefully recover from the error
            // make as though the vehicle has arrived
            // this can skew results so, we create a special event error saying this occurred.
            setArrival(vehId, getCurrentTime());
            // create an event saying this error occured
            std::vector<int> entities = {vehId};
            if (dp.bookingId) entities.push_back(dp.bookingId);
            amod::Event ev(amod::EVENT_VEH_SIMULATION_ERROR, ++eventId, "VehicleDestroyed", getCurrentTime(), entities);
            worldState->addEvent(ev);

            //throw std::runtime_error("checkVehicleAlive: Simmobility is destroying my vehicle!");
        }

        amod::Position vehPos = getPositionInSimmobility(worldState, vehId);

        if (vehPos.x == 0 && vehPos.y == 0) {
            // vehicle still not yet moved
            continue;
        }

        // update status and position of customer internally

        int custId = veh->getCustomerId();
        amod::Customer *cust = nullptr;
        if (custId) {
            cust = worldState->getCustomerPtr(custId);
            if (!cust) {
                throw std::runtime_error("simulateVehicles: cust is nullptr");
            }
        }

        // DEBUGGING
        // CHECK IF THE VEH IS STATIONARY FOR A LONG TIME

        if (vehPos == veh->getPosition()) {
            auto ppers = vehIdToPersonMap.find(vehId)->second;

            if (ppers && !(ppers->isToBeRemoved()) && !(ppers->isVehicleInLoadingQueue)) {
                veh->stationaryCount++;
            }
        } else {
            veh->stationaryCount = 0;
        }

        if (veh->stationaryCount > 10000) {
            auto ppers = vehIdToPersonMap.find(vehId)->second;
            //auto oldPrecision = std::cout.precision(10);
            Print() << "Veh is stuck! " << veh->getId() << " at " << veh->getPosition().x << " " << veh->getPosition().y << std::endl;

            // check the departure node of the vehicle
            Dispatch dp = dispatches[vehId];

            Print() << "Current time: " << getCurrentTime() << std::endl;
            Print() << "Dispatch ids from: " << dp.fromLocId << " to " << dp.toLocId << std::endl;
            Print() << "Source pos: " << dp.from.x << ", " << dp.from.y << std::endl;
            Print() << "Destination pos: " << dp.to.x << ", " << dp.to.y << std::endl;
            //std::cout.precision(oldPrecision);

            Print() << "Person: isToBeRemoved? " << ppers->isToBeRemoved() << ", isinit? " << ppers->isInitialized()
                    /*<< ", isQueuing? " << ppers->isQueuing*/ << ", " << ppers->getId() <<std::endl;

            veh->stationaryCount = 0;

            /*if(!logEnabled)
            {
                ConfigManager::GetInstanceRW().FullConfig().logInterval = 1;
                logEnabled = true;
            }*/

        }

        // END DEBUGGING

        veh->setPosition(vehPos);
        if (cust && cust->isInVehicle()) {
            cust->setPosition(vehPos);
        }

        // trigger move event
        std::vector<int> entities = {vehId};
        if (cust && cust->isInVehicle()) entities.push_back(custId);
        amod::Event ev(amod::EVENT_MOVE, ++eventId, "VehicleMoved", getCurrentTime(), entities);
        worldState->addEvent(ev);
    }
}

void AMODSimulatorSimMobility::simulateCustomers(amod::World *worldState) {
    // TODO
    // nothing to do here since it is handled by simulate vehicles
}


void AMODSimulatorSimMobility::simulatePickups(amod::World *worldState) {
    auto it = pickups.begin();
     while (it != pickups.end()) {
         if (it->first <= getCurrentTime()) {

             if (verbose) if (verbose) Print()  << it->second.vehId << " has picked up " << it->second.custId << " at time " << it->first << std::endl;

             // create pickup event
             int bid = it->second.bookingId;
             std::vector<int> entityIds = {it->second.vehId, it->second.custId};
             if (bid) entityIds.push_back(bid);
             amod::Event ev(amod::EVENT_PICKUP, ++eventId, "CustomerPickup", it->first, entityIds);
             worldState->addEvent(ev);

             amod::Vehicle veh = worldState->getVehicle(it->second.vehId);
             veh.setCustomerId(it->second.custId);
             amod::Customer cust = worldState->getCustomer(it->second.custId);
             cust.setAssignedVehicleId(it->second.vehId);
             cust.setInVehicle();
             cust.setLocationId(it->second.locId);

             // sets vehicle state
             veh.setStatus(it->second.vehEndStatus);

             // update the world state and internal state
             worldState->setCustomer(cust);
             worldState->setVehicle(veh);


             if (bid) {
                 amod::ReturnCode rc = dispatchVehicle(worldState, veh.getId(), bookings[bid].destination,
                        amod::VehicleStatus::MOVING_TO_DROPOFF, amod::VehicleStatus::HIRED,
                                                 bid);
                 if (rc != amod::SUCCESS) {
                    if (verbose) Print()  << bookings[bid].destination.x << " " <<
                            bookings[bid].destination.y << std::endl;
                     if (verbose) if (verbose) Print()  << amod::kErrorStrings[rc] << std::endl;
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

void AMODSimulatorSimMobility::simulateDropoffs(amod::World *worldState) {
    auto it = dropoffs.begin();
      while (it != dropoffs.end()) {
          if (it->first <= getCurrentTime()) {
                // create dropoff event
                if (verbose) if (verbose) Print()  << it->second.vehId << " has dropped off " << it->second.custId << " at time " << it->first << std::endl;
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
                        "LocationVehSizeChange", getCurrentTime(),
                        {locId});
                    worldState->addEvent(ev);

                    // update the customer and trigger event if necessary
                    if (cust && cust->isInVehicle()) {
                        amod::Location *ploc = worldState->getLocationPtr(locId);
                        ploc->addCustomerId(cust->getId());
                        cust->setLocationId(locId);
                        amod::Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                            "LocationCustSizeChange", getCurrentTime(),
                            {locId});
                        worldState->addEvent(ev);
                    }
                }

          } else {
              break;
          }
      }
}


void AMODSimulatorSimMobility::simulateTeleports(amod::World *worldState) {
    auto it = teleports.begin();
    while (it != teleports.end()) {
        if (it->first <= getCurrentTime()) {
            // create teleportation arrival event
            if (verbose) if (verbose) Print()  << it->second.custId << " has teleported to location " << it->second.locId << " at time " << it->first << std::endl;

            std::vector<int> entityIds = {it->second.custId};
            amod::Event ev(amod::EVENT_TELEPORT_ARRIVAL, ++eventId, "CustomerTeleportArrival", it->first, entityIds);
            worldState->addEvent(ev);

            amod::Customer cust = worldState->getCustomer(it->second.custId);
            cust.setStatus(it->second.custEndStatus);
            cust.setPosition(worldState->getLocationPtr(it->second.locId)->getPosition());


            // update the customer and trigger event if necessary
            int locId = it->second.locId;
            amod::Location *ploc = worldState->getLocationPtr(locId);
            ploc->addCustomerId(it->second.custId);
            cust.setLocationId(locId);
            amod::Event ev2(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                     "LocationCustSizeChange", getCurrentTime(),
                     {locId});
            worldState->addEvent(ev2);

            // update the external world and internal state
            worldState->setCustomer(cust);


            // erase item
            teleports.erase(it);
            it = teleports.begin();
        } else {
            break;
        }
    }
}


void AMODSimulatorSimMobility::setPickupDistributionParams(double mean, double sd, double min, double max) {
    std::normal_distribution<>::param_type par{mean, sd};
    pickupParams.par = par;
    pickupParams.max = max;
    pickupParams.min = min;
}

void AMODSimulatorSimMobility::setDropoffDistributionParams(double mean, double sd, double min, double max) {
    std::normal_distribution<>::param_type par{mean, sd};
    dropoffParams.par = par;
    dropoffParams.max = max;
    dropoffParams.min = min;
}

void AMODSimulatorSimMobility::setTeleportDistributionParams(double mean, double sd, double min, double max) {

    std::normal_distribution<>::param_type par{mean, sd};
    teleportParams.par = par;
    teleportParams.max = max;
    teleportParams.min = min;
}

double AMODSimulatorSimMobility::getCurrentTime() {
    return currentTime;
}

void AMODSimulatorSimMobility::setController(sim_mob::Agent *parentController_) {
    if (!parentController_) {
        throw std::runtime_error(baseErrMsg + "setController: parent_controller is nullptr");
    }
    parentController = parentController_;
}

void AMODSimulatorSimMobility::setCurrentTime(double currTime) {
    currentTime = currTime;
}

void AMODSimulatorSimMobility::setCurrentTimeMs(int currTimeMS) {
    currentTime = ((double) currTimeMS)/1000.0;
}

void AMODSimulatorSimMobility::setArrival(int amodId, int currTimeSecs) {

    arrivalsMtx.lock();
    dispatches[amodId].arrived = true;
    vehIdToPersonMap[amodId] = nullptr; // the Person will become invalid
    arrivals.emplace_back(amodId, ((double) currTimeSecs)/1000.0); //set in seconds
    arrivalsMtx.unlock();
}


double AMODSimulatorSimMobility::genRandTruncNormal(TruncatedNormalParams &params) {
    normalDist.param(params.par);
    double r = normalDist(eng);
    if (r > params.max) r = params.max;
    if (r < params.min) r = params.min;
    return r;
}

int AMODSimulatorSimMobility::findClosestNode(const amod::Position &to) {
    // Search using KDTree
    auto closestLoc = locTree.findNN({to.x, to.y});
    return closestLoc.getId();
}

amod::Position AMODSimulatorSimMobility::getNodePosition(int nodeId) {
    double xInMeters = nodePool[nodeId]->getLocation().getX();
    double yInMeters = nodePool[nodeId]->getLocation().getY();
    return amod::Position(xInMeters, yInMeters);
}




/*
 * PRIVATE FUNCTIONS
 *
 */
amod::ReturnCode AMODSimulatorSimMobility::dispatchVehicle(amod::World *worldState,
                                                 int vehId,
                                                 const amod::Position &to,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus,
                                                 int bookingId) {
    // dispatches parked vehicles from one position (node in simmobility) to another
    // TODO: add facility to reroute free/in-route vehicles

    // error checks
    if (!parentController) {
        if (verbose) Print()  << "Parent controller is null" << std::endl;
        throw std::runtime_error(baseErrMsg + "dispatchVehicle: parent_controller_ is null");
    }


    // check if vehicle already exists in the dispatch map
    auto it = dispatches.find(vehId);
    if (it != dispatches.end()) {
        if (verbose) Print()  << "Vehicle ID found (it is currently being dispatched!): " << vehId << std::endl;
        return amod::VEHICLE_CANNOT_BE_DISPATCHED;
    }

    // create a new dispatch
    amod::Vehicle *veh = worldState->getVehiclePtr(vehId);
    if (!veh) {
        if (verbose) Print() << "Can't get vehicle from world_state" << std::endl;
        return amod::CANNOT_GET_VEHICLE;
    }

    // create a new dispatch
    Dispatch dp;
    dp.bookingId = bookingId;
    dp.vehId = vehId;
    dp.from = veh->getPosition();
    dp.arrived = false;
    // first get where we are going
    amod::Location des = locTree.findNN({to.x, to.y});
    dp.to = des.getPosition(); // find the closest location to be the destination position
    dp.toLocId = des.getId();
    if (dp.toLocId == 0) {
        Print() << " Destination id is zero!" << std::endl;
        throw std::runtime_error("Destination id is zero!");

    }

    // then where we are from
    des = locTree.findNN({dp.from.x, dp.from.y}); // in case the vehicle is not at a node
    dp.from = des.getPosition(); // find the closest location to be the destination position
    dp.fromLocId = des.getId();
    if (verbose) Print()  << "Dispatching from: " << dp.fromLocId << std::endl;
    if (verbose) Print()  << "Dispatching to: " << dp.toLocId << std::endl;

    dp.curr = dp.from;
    dp.vehEndStatus = endStatus;

    // DEBUGGING
    veh->stationaryCount = 0;


    // check if the origin and destination is the same
    if (dp.toLocId == dp.fromLocId) {
        if (verbose) Print() << "Start and end location is the same" << std::endl;
        // the starting and end positions are exactly the same
        // we do not really dispatch but simply trigger an arrival
        arrivalsMtx.lock();
        dp.arrived = true;
        vehIdToPersonMap[vehId] = nullptr; // the Person will become invalid
        arrivals.emplace_back(vehId, getCurrentTime()); //set in seconds
        arrivalsMtx.unlock();

    } else {

        // when some travelling is needed
        // dispatch vehicle from current location to n
        // Simmobility specific code for dispatching a vehicle
        int currentTimeMS = round(getCurrentTime()*1000);
        if (verbose) Print()  << ConfigManager::GetInstance().FullConfig().simStartTime().getValue() << ": " << currentTimeMS << std::endl;
        DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + currentTimeMS);

        // create a trip chain
        SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(),
                nodePool[dp.fromLocId], "node",
                nodePool[dp.toLocId], "node", "Car");


        // use path set manager to get way points
        std::vector < WayPoint > waypoints;
        bool useShortestPath = true;

        if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
        	PrivateTrafficRouteChoice* pmgr = PrivateTrafficRouteChoice::getInstance();

            if (pmgr != nullptr && (dp.fromLocId != dp.toLocId)) {
                //std::cout << "Pathset manager" << std::endl;
                try {
                    waypoints = pmgr->getPath(subTrip, false, nullptr);
                    useShortestPath = false;
                } catch (std::exception e) {
                    //std::cout << e.what() << std::endl;
                    if (verbose) Print() << "Path set manager failed: " << e.what() << std::endl;
                }
            }
        }

        if (waypoints.empty() || useShortestPath) {
            //std::cout << "Shortest Path" << std::endl;
            std::vector<const sim_mob::Link*> blacklist;
            waypoints = stDir->SearchShortestDrivingPath(*nodePool[dp.fromLocId],*nodePool[dp.toLocId],blacklist);
        }


        std::vector < WayPoint > filteredWaypoints;
        for(std::vector<WayPoint>::iterator it = waypoints.begin(); it != waypoints.end(); ++it)
        {
            if (it->type == WayPoint::LINK)
            {
                filteredWaypoints.push_back(*it);
            }
        }

        if (filteredWaypoints.size() == 0) {
            // cannot generate such a path
            return amod::NO_PATH_TO_DESTINATION;
        }

    	//The path containing the links and turning groups
    	vector<WayPoint> path;

    	//Add the road segments and turning groups that lie along the links in the path
    	for (vector<WayPoint>::iterator itWayPts = filteredWaypoints.begin(); itWayPts != filteredWaypoints.end(); ++itWayPts)
    	{
    		//The segments in the link
    		const vector<RoadSegment *> &segments = itWayPts->link->getRoadSegments();

    		//Create a way point for every segment and insert it into the path
    		for (vector<RoadSegment *>::const_iterator itSegments = segments.begin(); itSegments != segments.end(); ++itSegments)
    		{
    			path.push_back(WayPoint(*itSegments));
    		}

    		if((itWayPts + 1) != filteredWaypoints.end())
    		{
    			unsigned int currLink = itWayPts->link->getLinkId();
    			unsigned int nextLink = (itWayPts + 1)->link->getLinkId();

    			//Get the turning group between this link and the next link and add it to the path

    			const TurningGroup *turningGroup = itWayPts->link->getToNode()->getTurningGroup(currLink, nextLink);

    			if (turningGroup)
    			{
    				path.push_back(WayPoint(turningGroup));
    			}
    			else
    			{
    				stringstream msg;
    				msg << "No turning between the links " << currLink << " and " << nextLink << "!\nInvalid Path!!!";
    				throw std::runtime_error(msg.str());
    			}
    		}
    	}

        sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(),
                "",
                nodePool[dp.fromLocId], "node",
                nodePool[dp.toLocId], "node");
        ((Trip*)tc)->addSubTrip(subTrip);
        std::vector<sim_mob::TripChainItem*>  tcs;
        tcs.push_back(tc);

        // create new Person and scheduled for simulation
        sim_mob::Person_ST* smVeh = new sim_mob::Person_ST("AMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);

        //sm_veh->setTripChain(tcs); //add trip chain; no longer necessary
        smVeh->setPath(path);
        smVeh->amodId = veh->getName();

        smVeh->amodSegmLength = 0.0;
        smVeh->amodPickUpSegmentStr = "-1";

        smVeh->amodSegmLength2 = 0.0;
        smVeh->amodDropOffSegmentStr = "-1";

        std::stringstream ss;
        if (bookingId) {
            ss << bookingId;
        } else {
            ss << ++tripId;
        }
        smVeh->amodTripId = ss.str();
        parentController->currWorkerProvider->scheduleForBred(smVeh); //worker provider is a public member

        // insert into our map so we can find this Person
        vehIdToPersonMap[veh->getId()] = smVeh;
        // add record of this dispatch

    }
    dispatches[veh->getId()] = dp;


    // set the vehicle and customer status
    veh->setStatus(startStatus);
    if (bookingId) {
        amod::Customer *cust = worldState->getCustomerPtr(bookings[bookingId].custId);
        if (!cust) {
            throw std::runtime_error("dispatchVehicle: cannot get associated customer!");
        }
        if (!cust->isInVehicle()) {
            //if (verbose_) if (verbose_) std::cout  << "Set customer waiting for pickup" << std::endl;
            cust->setStatus(amod::CustomerStatus::WAITING_FOR_PICKUP);
        } else {
            //if (verbose_) if (verbose_) std::cout  << "Set customer in vehicle" << std::endl;
            cust->setStatus(amod::CustomerStatus::IN_VEHICLE);

            // log the waiting time
            waitingTimeFile << vehId << "," << bookings[bookingId].source.x << "," <<
                               bookings[bookingId].source.y << "," << bookings[bookingId].bookingTime << "," <<
                               getCurrentTime() << "," << veh->getLocationId() <<std::endl;
        }
    }


    // create a dispatch event
    std::vector<int> entityIds = {veh->getId(), bookingId};
    amod::Event dev(amod::EVENT_DISPATCH, ++eventId, "VehicleDispatch", getCurrentTime(), entityIds);
    worldState->addEvent(dev);


    // update the locations
    amod::Location * ploc = worldState->getLocationPtr(dp.fromLocId);
    ploc->removeVehicleId(veh->getId());
    veh->setLocationId(0);
    // trigger event
    amod::Event ev(amod::EVENT_LOCATION_VEHS_SIZE_CHANGE, ++eventId,
            "LocationVehSizeChange", getCurrentTime(),
            {dp.fromLocId});
    worldState->addEvent(ev);

    amod::Customer *cust = worldState->getCustomerPtr(bookings[bookingId].custId);

    if (cust && cust->isInVehicle()) {
        ploc->removeCustomerId(cust->getId());
        cust->setLocationId(0);
        // trigger event
        amod::Event ev(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
                "LocationCustSizeChange", getCurrentTime(),
                {dp.fromLocId});
        worldState->addEvent(ev);
    }

    return amod::SUCCESS;
}



amod::ReturnCode AMODSimulatorSimMobility::pickupCustomer(amod::World *worldState,
                                                    int vehId, int custId,
                                                    amod::VehicleStatus startStatus,
                                                    amod::VehicleStatus endStatus,
                                                    int bookingId) {
    // check that vehicle is at the same location as the customer
    amod::Vehicle* veh = worldState->getVehiclePtr(vehId);
    if (!veh) {
        Print() << "pickupCustomer: Veh is nullptr" << std::endl;
        throw std::runtime_error("pickupCustomer: veh is nullptr");
        return amod::CANNOT_GET_VEHICLE;
    }

    amod::Customer* cust = worldState->getCustomerPtr(custId);
    if (!cust) {
        Print() << "Failed to get customer: " << custId << std::endl;
        throw std::runtime_error("pickupCustomer: cust is nullptr");
        return amod::CANNOT_GET_CUSTOMER;
    }

    // add a pickup to simulate
    double pickupTime = getCurrentTime() + genRandTruncNormal(pickupParams);
    if (verbose) if (verbose) Print()  << "Future Pickup time : " << pickupTime << std::endl;

    int locId = 0;
    // get the pickup location
    amod::Location pickupLoc = locTree.findNN({cust->getPosition().x, cust->getPosition().y});
    locId = pickupLoc.getId();

    Pickup p{bookingId, vehId, custId, locId, pickupTime, endStatus};
    pickups.emplace(pickupTime, p);

    // set the vehicle's start status in the world state
    veh->setStatus(startStatus);
    cust->setStatus(amod::CustomerStatus::WAITING_FOR_PICKUP);

    return amod::SUCCESS;
}

amod::ReturnCode AMODSimulatorSimMobility::dropoffCustomer(amod::World *worldState,
                                                 int vehId, int custId,
                                                 amod::VehicleStatus startStatus,
                                                 amod::VehicleStatus endStatus,
                                                 int bookingId) {
    // check that vehicle is at the same location as the customer
    amod::Vehicle* veh = worldState->getVehiclePtr(vehId);
    if (!veh) {
        throw std::runtime_error("pickupCustomer: veh is nullptr");
        return amod::CANNOT_GET_VEHICLE;
    }


    if (veh->getCustomerId() != custId) {
        return amod::VEHICLE_DOES_NOT_HAVE_CUSTOMER;
    }

    amod::Customer* cust = worldState->getCustomerPtr(custId);
    if (!cust) {
        throw std::runtime_error("pickupCustomer: cust is nullptr");
        return amod::CANNOT_GET_CUSTOMER;
    }

    // add a dropoff to simulate
    double dropoffTime = getCurrentTime() + genRandTruncNormal(dropoffParams);

    int locId = 0;
    // get the dropoff location
    amod::Location dropoffLoc = locTree.findNN({cust->getPosition().x, cust->getPosition().y});
    locId = dropoffLoc.getId();
    if (locId == 0) {
        Print() << "Location ID should not be zero" << std::endl;
    }

    //if (verbose_) if (verbose_) std::cout  << "Future Dropoff time : " << dropoff_time << std::endl;
    Dropoff doff{bookingId, vehId, custId, locId, dropoffTime, endStatus};
    dropoffs.emplace(dropoffTime, doff);

    // sets the status of the vehicle
    veh->setStatus(startStatus);
    cust->setStatus(amod::CustomerStatus::WAITING_FOR_DROPOFF);


    // return success
    return amod::SUCCESS;
}



amod::Position AMODSimulatorSimMobility::getPositionInSimmobility(amod::World *worldState, int vehId) {
    if (!vehId) {
        throw std::runtime_error(baseErrMsg + "updateObjectPosition: invalid veh_id");
    }

    auto itr = vehIdToPersonMap.find(vehId);
    if (itr == vehIdToPersonMap.end()) {
        throw std::runtime_error(baseErrMsg + "updateObjectPosition: no such entityid in map");
    }
    amod::Vehicle *veh = worldState->getVehiclePtr(vehId);
    if (itr->second == nullptr) {
        // object no longer exists in the simulation
        // do not update, simply return existing position

        if (!veh) {
            throw std::runtime_error("getPositionInSimmobility: No such veh_id in world!");
        }
        return veh->getPosition();
    }


    try {
        double x = itr->second->xPos.get();
        double y = itr->second->yPos.get();
        return amod::Position( x, y);
    } catch (std::exception &e) {
        amod::Vehicle *veh = worldState->getVehiclePtr(vehId);
        if (!veh) {
            throw std::runtime_error("getPositionInSimmobility: No such veh_id in world!");
        }
        return veh->getPosition();
    }
}


amod::ReturnCode AMODSimulatorSimMobility::teleportCustomer(amod::World *worldState,
                                         int custId,
                                         const amod::Position &to,
                                         amod::CustomerStatus custStartStatus,
                                         amod::CustomerStatus custEndStatus
                                         ) {
    amod::Customer *cust = worldState->getCustomerPtr(custId);
    if (!cust) {
        return amod::CANNOT_GET_CUSTOMER;
    }

    if (cust->getStatus() != amod::CustomerStatus::FREE) {
        return amod::CUSTOMER_IS_NOT_FREE;
    }

    // set a teleporation arrival time
    double teleportTime = getCurrentTime() + genRandTruncNormal(teleportParams);

    int fromLocId = 0;
    // get the teleport location
    amod::Location telportLoc = locTree.findNN({cust->getPosition().x, cust->getPosition().y});
    fromLocId = telportLoc.getId();

    int toLocId = 0;
    // get the teleport location
    telportLoc = locTree.findNN({to.x, to.y});
    toLocId = telportLoc.getId();


    Teleport tport{custId, toLocId, teleportTime, custEndStatus};

    teleports.emplace(teleportTime, tport);

    // sets the status of the vehicle
    cust->setStatus(custStartStatus);

    // create a teleportation event
    std::vector<int> entityIds = {custId, fromLocId};
    amod::Event ev(amod::EVENT_TELEPORT, ++eventId, "CustomerTeleport", getCurrentTime(), entityIds);
    worldState->addEvent(ev);

    // adjust locations
    // location specific changes
    amod::Location * ploc = worldState->getLocationPtr(fromLocId);
    ploc->removeCustomerId(custId);
    cust->setLocationId(0);

    // trigger event
    amod::Event ev2(amod::EVENT_LOCATION_CUSTS_SIZE_CHANGE, ++eventId,
             "LocationCustSizeChange", getCurrentTime(),
             {fromLocId});
    worldState->addEvent(ev2);

    return amod::SUCCESS;
}

void AMODSimulatorSimMobility::setCustomerStatus(amod::World *worldState, int custId, amod::CustomerStatus status) {
    amod::Customer cust = worldState->getCustomer(custId);
    cust.setStatus(status);
    worldState->setCustomer(cust);
}


//        AMODBase::ReturnCode AMODSimulatorSimMobility::createObjectsInWorld(const std::vector<AMODBase::AMODObject*> &objects) {
//            // iterate through and create a map of all objects and their positions in the world
//            /*for (auto op : objects) {
//                // error check
//                if (!op) {
//                    throw std::runtime_error(base_error_msg + "createObjectsInWorld: object ptr is nullptr");
//                }
//
//                int node_id = findClosestNode(op->getPosition());
//            }*/
//
//            return AMODBase::SUCCESS;
//        }


bool AMODSimulatorSimMobility::checkVehicleAlive(int vehId) {
    if (!vehId) {
        return false;
    }
    auto itr = vehIdToPersonMap.find(vehId);
    if (itr == vehIdToPersonMap.end()) {
        //if (verbose_) std::cout << "End of Person Map" << std::endl;
        return false;
    }

    if (itr->second == nullptr) {
        //if (verbose_) std::cout << "Person is nullptr" << std::endl;
        return false;
    }

    if (itr->second->isToBeRemoved()) {
        //if (verbose_) std::cout << "Person is about to be removed" << std::endl;
        return false;
    }
    return true;
}

void AMODSimulatorSimMobility::initWaitingTimeFile(const std::string &fileName)
{
    if(!fileName.empty())
    {
        waitingTimeFile.open(fileName);
    }
}


} /* namespace AMOD */
} /* namespace simmob */
