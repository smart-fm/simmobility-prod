/* 
 * File:   Common.hpp
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

/**
 * Events IDs
 * Kindly observe the uniqueness of all ids before adding a new one
 */
#define COMMEID_START 9000000

// Events for HousingMarket
#define COMMEID_TIME     (COMMEID_START + 1)
#define COMMEID_LOCATION   (COMMEID_START + 2)

/*special context ids
 * good for situations like NS3 location subscription where it
 * requires a special context for publishing the location of
 * all agents
 */
#define COMMCID_ALL_LOCATIONS (COMMEID_START + 3)
