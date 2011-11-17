/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>

namespace sim_mob
{


/**
 * Class to represent data we wish to carry along but not react to. Agents shouldn't ever check these values.
 */
class OpaqueProperty {
private:
	std::string key;
	std::string value;

public:
	void setProps(const std::string& key, const std::string& value) {
		this->key = key;
		this->value = value;
	}

	std::string getLogItem() const {
		return "\"" + key + "\"" + ":" + "\"" + value + "\"" + ",";
	}

};


}


