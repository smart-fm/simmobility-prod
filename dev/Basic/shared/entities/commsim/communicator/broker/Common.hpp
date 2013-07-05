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
 */
#define COMMEID_START 9000000

// Events for HousingMarket
#define COMMEID_TIME     COMMEID_START + 1
#define COMMEID_LOCATION   COMMEID_START + 2
