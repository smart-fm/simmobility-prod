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

const int INVALID_ID = -1;
const std::string EMPTY_STR= "";

/**
 * Events IDs. Using an enum to guarantee size.
 */
enum LongTermEventIds {
	LTEID_START =1000000,

	// Events for HousingMarket
	// housing market action (unit was ADDED, REMOVED or UPDATED )
	LTEID_HM_UNIT_ADDED,
	LTEID_HM_UNIT_REMOVED,
};


/**
 * Message Types. Using an enum to guarantee size.
 */
enum LongTermMessageIds {
	LTMID_START =2000000,

	// Messages for biding process
	LTMID_BID,
	LTMID_BID_RSP,
};


/**
 * DEFAULT VALUES
 */
const int MIN_AGE_TO_WORK =18;

/**
 * TIME UNITS
 */
const int TIME_UNIT_DAILY =1;
const int TIME_UNIT_WEEKLY =7;
const int TIME_UNIT_MONTHLY =30;
const int TIME_UNIT_YEARLY =365;

const int TIME_UNIT = TIME_UNIT_WEEKLY;

/**
 * Configs
 */
 const std::string LT_DB_CONFIG_FILE = "../private/lt-db.ini";
 const std::string HM_LUA_DIR = "../scripts/lua/long/housing-market";



