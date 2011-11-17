/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <sstream>
#include <string>

namespace sim_mob {


/**
 * Very simple class which holds a value that is only used in the output (log) file.
 * Should not be accessed by any C++ code at all.
 */
class OpaqueProperty
{
private:
	std::string key;
	int value;

public:
	void initProperty(std::string key, int value) {
		/*std::stringstream res;
		res <<"\"" <<key.c_str() <<"\"" <<":";
		res <<"\"" <<value <<"\"" <<",";
		res >>output_str;*/
		//this->key = key;
		//this->value = value;
	}
	const std::string& getString() const {
		return key;
	}
	bool isSet() const {
		return !key.empty();
	}
};


}
