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

/**
 * Types
 */
typedef int UnitId;

/**
 * Events IDs
 */
#define LTID_START 1000000

// Events for HousingMarket
#define LTID_BID         LTID_START + 1
#define LTID_BID_RSP     LTID_START + 2

