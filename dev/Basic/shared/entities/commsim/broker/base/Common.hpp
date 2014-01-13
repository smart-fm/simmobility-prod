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

/**
 * Events IDs
 * Uses an enum to guarantee uniqueness.
 */
enum {
	COMMEID_START = 9000000,

	// Events for HousingMarket (???)
	COMMEID_TIME,
	COMMEID_LOCATION,
	COMMEID_ALL_LOCATIONS,
	COMMEID_REGIONS_AND_PATH,

	//special context ids
	//good for situations like NS3 location subscription where it
	//requires a special context for publishing the location of
	//all agents
	COMMCID_ALL_LOCATIONS,
};


