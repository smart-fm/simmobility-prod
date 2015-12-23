/*
 * Logger.cpp
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#include "Logger.hpp"

namespace sim_mob{

namespace amod {

Logger::Logger() {
	// TODO Auto-generated constructor stub
    moveEvntInterval = 0;
    nextMoveEvntLogTime = 0;
}

Logger::Logger(const std::string& filename) {
    openLogFile(filename);
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

amod::ReturnCode Logger::openLogFile(const std::string& filename) {
    logFile.open(filename.c_str());
    if (!logFile) return amod::CANNOT_OPEN_LOGFILE;
    logFile.precision(10);
    return amod::SUCCESS;
}

amod::ReturnCode Logger::closeLogFile() {
    logFile.close();
    return amod::SUCCESS;
}

amod::ReturnCode Logger::logEvents(amod::World *worldState, bool outMoveEvnts, bool clearEvnts) {
    
    // first version, log events differently depending on type of events
    // future version would streamline this
    std::vector<Event> events;
    worldState->getEvents(&events);
    
    // check if we output move events now
    if (outMoveEvnts) {
        outMoveEvnts = outMoveEvnts && (worldState->getCurrentTime() > nextMoveEvntLogTime);
        if (outMoveEvnts) {
            nextMoveEvntLogTime = worldState->getCurrentTime() + moveEvntInterval;
        }
    }
    
    // respond to events
    for (auto e:events) {
        if (logFile.is_open()) {
            if ((outMoveEvnts && e.type == EVENT_MOVE) || (e.type != EVENT_MOVE)) {
                logFile << e.t << " Event " << e.id << " " << e.type << " " << e.name << " Entities: ";
                for (auto ent: e.entityIds) {
                    logFile << ent << ",";
                }
                logFile << " ";
            }
        }
        
        if (e.type == EVENT_MOVE || e.type == EVENT_ARRIVAL || e.type == EVENT_PICKUP || e.type == EVENT_DROPOFF) {
            amod::Vehicle veh = worldState->getVehicle(e.entityIds[0]);
            
            if (logFile.is_open()) {
                if ((outMoveEvnts && e.type == EVENT_MOVE) || (e.type != EVENT_MOVE)) {
                    logFile << veh.getPosition().x << " " << veh.getPosition().y << " " << veh.getStatus();
                }
            }
        }
        
        // teleportation event
        if (e.type == EVENT_TELEPORT || e.type == EVENT_TELEPORT_ARRIVAL) {
            amod::Customer cust = worldState->getCustomer(e.entityIds[0]);
            if (logFile.is_open()) logFile << cust.getPosition().x << " " << cust.getPosition().y << " " << cust.getStatus();
        }

        // output the location sizes
        if (e.type == EVENT_LOCATION_CUSTS_SIZE_CHANGE ||
                e.type == EVENT_LOCATION_VEHS_SIZE_CHANGE) {
            amod::Location * ploc = worldState->getLocationPtr(e.entityIds[0]);
            int curr_size = (e.type == EVENT_LOCATION_VEHS_SIZE_CHANGE)? ploc->getNumVehicles(): ploc->getNumCustomers();
            if (logFile.is_open()) logFile << ploc->getPosition().x << " " << ploc->getPosition().y << " " << curr_size;
        }
        
        if (e.type == EVENT_DISPATCH) {
             amod::Vehicle veh = worldState->getVehicle(e.entityIds[0]);
            if (logFile.is_open()) logFile << veh.getStatus();
        }
        
        if ((outMoveEvnts && e.type == EVENT_MOVE) || (e.type != EVENT_MOVE)) {
            if (logFile.is_open()) logFile << std::endl;
        }

    }
    // clear events
    if (clearEvnts) worldState->clearEvents();
    
    return amod::SUCCESS;
}
    
}
}
