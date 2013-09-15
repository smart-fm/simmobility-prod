//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include "logging/Log.hpp"

/**
 * Events IDs. Using an enum guarantees size.
 */
enum CommunicationIds {
	COMMEID_START = 9000000,
	COMMEID_TIME,
	COMMEID_LOCATION,
};

