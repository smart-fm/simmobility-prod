/*
 * Logger.h
 *
 *  Created on: Mar 23, 2015
 *      Author: haroldsoh
 */

#pragma once

#include "Event.hpp"
#include "Types.hpp"
#include "World.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

namespace sim_mob{

namespace amod {

class Logger {
public:
    /**
     * Constructor
     */
	Logger();

    /**
     * Constructor
     * @param filename file where the logging to be done
     */
    Logger(const std::string& filename);

    /**
     * Destructor
     */
	virtual ~Logger();
    
    /**
     * openLogFile
     * Attempts to open the log file
     * @param filename - log file to be opened
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode openLogFile(const std::string& filename);

    /**
     * closeLogFile
     * Attempts to close the log file
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode closeLogFile();

    /**
     * logEvents
     * Log the events in the amod world
     * @param worldState - Pointer to the amod world
     * @param outMoveEvnts - flag to specify logging move events (default is true)
     * @param clearEvnts - flag to specify whether to clear the events (default is true)
     * @return if the call is successful, it returns amod::SUCESSS. Otherwise, it returns
     * one of the amod::ReturnCode error codes.
     */
    virtual amod::ReturnCode logEvents( amod::World *worldState, bool outMoveEvnts = true, bool clearEvnts=true);
    
    /**
     * setMoveEventLogInterval
     * sets tje move event log interval
     * @param interval
     */
    virtual void setMoveEventLogInterval(double interval) {
        if (interval >= 0) {
            moveEvntInterval = interval;
        } else {
            throw std::runtime_error("Interval cannnot be negative");
        }
    }
    
    /**
     * getMoveEventLogInterval
     * @return Move event log interval
     */
    virtual double getMoveEventLogInterval() { return moveEvntInterval; }
    
private:
    /// file where the logging is done
    std::ofstream logFile;

    /// move event interval
    double moveEvntInterval;

    /// next move event log time
    double nextMoveEvntLogTime;
};

}

}
