/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>


namespace sim_mob {

class Config;


/**
 * Class used to validate a config file. Validation occurs directly after loading from the
 * config file. It checks simple things like system granularity compatibilty and WorkGroup
 * counts. More complex checking is done by the individual loader classes (e.g., AgentLoader)
 *
 * Typically used like a verb:
 *     Config cfg = //load cfg somehow
 *     Validate val(cfg);
 */
class Validate : private boost::noncopyable {
public:
	///Validate the given config file; throwing an error if anything fails to validate.
	Validate(Config& cfg);

private:
	//Various internal validation checks.
	void CheckAndSetGranularities() const;

private:
	//The config file we are currently dealing with.
	Config& cfg;
};

}
