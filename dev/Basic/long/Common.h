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

using std::cout;
using std::endl;
using std::ostream;

#define INVALID_ID -1

/**
 * Events IDs
 */
#define LTEID_START 1000000

// Events for HousingMarket
// housing market action (unit was ADDED, REMOVED or UPDATED ) 
#define LTEID_HM_UNIT_ADDED     LTEID_START + 1
#define LTEID_HM_UNIT_REMOVED   LTEID_START + 2


/**
 * Message Types
 */
#define LTMID_START 2000000

// Messages for biding process
#define LTMID_BID         LTMID_START + 1
#define LTMID_BID_RSP     LTMID_START + 2

/**
 * DEFAULT VALUES
 */
#define MIN_AGE_TO_WORK 18



