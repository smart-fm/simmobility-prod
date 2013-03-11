/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Util.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:13 PM
 */
#pragma once

#include "conf/settings/DisableMPI.h"

/**
 * Long-term Events IDs
 */
#define LTID_START 10000

// Events for HousingMarket
#define LTID_HM_START LTID_START + 100
#define LTID_HM_UNIT_STATE_CHANGED LTID_HM_START + 1
