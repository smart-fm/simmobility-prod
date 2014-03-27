//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "json/json.h"

namespace sim_mob {
namespace comm {

/**
 * Inherit from this class to indicate that your subclass can serialize itself to JSON.
 * This is currently only used by commsim events; we might want to expand on this functionality
 *   and make it available through, e.g., the "util" folder.
 */
/*class JsonSerializable {
public:
	virtual ~JsonSerializable() {}
	virtual Json::Value toJSON() const = 0;
};*/

}}
