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

#include <string>
#include <vector>
#include "stddef.h"
#include "util/LangHelpers.hpp"
#include "Common.hpp"

namespace sim_mob {
    namespace long_term {
        
        typedef long long BigSerial;
        typedef std::vector<BigSerial> IdVector;
        
        enum Sex {
            UNKNOWN_SEX = 0,
            MASCULINE = 1,
            FEMININE = 2
        };

        enum Race {
            UNKNOWN_RACE = 0,
            CHINISE = 1,
            MALAY = 2,
            INDIAN = 3,
            OTHER = 4,
        };

        enum EmploymentStatus {
            UNKNOWN_STATUS = -1,
            UNEMPLOYED = 0,
            EMPLOYED = 1
        };

        enum TimeUnit {
            DAILY = 0,
            WEEKLY = 1,
            MONTHLY = 2,
            YEARLY = 3
        };

        static Race ToRace(int value) {
            switch (value) {
                case CHINISE: return CHINISE;
                case MALAY: return MALAY;
                case INDIAN: return INDIAN;
                case OTHER: return OTHER;
                default: return UNKNOWN_RACE;
            }
        }

        static Sex ToSex(int value) {
            switch (value) {
                case MASCULINE: return MASCULINE;
                case FEMININE: return FEMININE;
                default: return UNKNOWN_SEX;
            }
        }

        static EmploymentStatus ToEmploymentStatus(int value) {
            switch (value) {
                case UNEMPLOYED: return UNEMPLOYED;
                case EMPLOYED: return EMPLOYED;
                default: return UNKNOWN_STATUS;
            }
        }

        static TimeUnit ToTimeUnit(int value) {
            switch (value) {
                case DAILY: return DAILY;
                case WEEKLY: return WEEKLY;
                case MONTHLY: return MONTHLY;
                case YEARLY: return YEARLY;
                default: return DAILY;
            }
        }

        /**
         * Structs
         */
        typedef struct ExpectationEntry_ {
            ExpectationEntry_() : price(0), expectation(0){}
            double price;
            double expectation;
        } ExpectationEntry;
    }
}