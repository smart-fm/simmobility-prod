//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Types.h
 * Author: Pedro Gandola
 *
 * Created on April 4, 2013, 2:04 PM
 */

#pragma once

#include <iostream>
#include "stddef.h"
#include "util/LangHelpers.hpp"
#include "logging/Log.hpp"
#include "event/SystemEvents.hpp"

namespace sim_mob {
    namespace long_term {

        /**
         * Events IDs. Using an enum to guarantee size.
         */
        enum LT_EventId {
            LTEID_START = event::EVT_LONG_START,
            
            // Model life cycle events.
            LTEID_MODEL_STARTED,
            LTEID_MODEL_STOPPED,

            // Events for HousingMarket
            // housing market action (unit was ADDED, REMOVED or UPDATED )
            LTEID_HM_UNIT_ADDED,
            LTEID_HM_UNIT_REMOVED,
            
            // External Events
            LTEID_EXT_NEW_JOB,
            LTEID_EXT_LOST_JOB,
            LTEID_EXT_NEW_JOB_LOCATION,
            LTEID_EXT_NEW_CHILD,
            LTEID_EXT_NEW_SCHOOL_LOCATION,
        };

        /**
         * Message Types. Using an enum to guarantee size.
         */
        enum LT_MessageId {
            LTMID_START = 2000000,

            // HM internal messages.
            LTMID_HMI_ADD_ENTRY,
            LTMID_HMI_RM_ENTRY,
            
            // Messages for biding process
            LTMID_BID,
            LTMID_BID_RSP,
            LTMID_LOG,
        };

        const int INTERNAL_MESSAGE_PRIORITY = 5;
        const double INVALID_DOUBLE = -1;
        const int INVALID_ID = -1;
        const std::string EMPTY_STR = std::string();

        /**
         * DEFAULT VALUES
         */
        const int MIN_AGE_TO_WORK = 18;
        
        /**
         * TIME UNITS
         */
        const int TIME_UNIT_DAILY = 1;
        const int TIME_UNIT_WEEKLY = 7;
        const int TIME_UNIT_MONTHLY = 30;
        const int TIME_UNIT_YEARLY = 365;

        const int TIME_UNIT = TIME_UNIT_WEEKLY;

        /**
         * Configs
         */
        const std::string LT_CONFIG_FILE = "../data/long/lt-config.ini";
        const std::string LT_DB_CONFIG_FILE = "../private/lt-db.ini";
        const std::string HM_LUA_DIR = "../scripts/lua/long/housing-market";
        const std::string EX_EV_LUA_DIR = "../scripts/lua/long/external-events";
        const std::string DEV_LUA_DIR = "../scripts/lua/long/developer";
    }
}



