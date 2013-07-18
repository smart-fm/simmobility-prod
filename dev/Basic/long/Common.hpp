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
#include "util/OutputUtil.hpp"

#define INVALID_ID -1
#define EMPTY_STR ""

/**
 * Events IDs
 */
#define LTEID_START 1000000

// Events for HousingMarket
// housing market action (unit was ADDED, REMOVED or UPDATED ) 
const int LTEID_HM_UNIT_ADDED           = (LTEID_START + 1);
const int LTEID_HM_UNIT_REMOVED         = (LTEID_START + 2);


/**
 * Message Types
 */
#define LTMID_START 2000000

// Messages for biding process
const int  LTMID_BID            = (LTMID_START + 1);
const int  LTMID_BID_RSP        = (LTMID_START + 2);

/**
 * DEFAULT VALUES
 */
#define MIN_AGE_TO_WORK 18

/**
 * TIME UNITS
 */
#define TIME_UNIT_DAILY 1 
#define TIME_UNIT_WEEKLY 7
#define TIME_UNIT_MONTHLY 30 
#define TIME_UNIT_YEARLY 365 

#define TIME_UNIT TIME_UNIT_WEEKLY




