/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <list>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>


namespace sim_mob {

//Forward declarations
class Config;


/**
 * Class used to validate, load, and initialize the Road Network..
 * Typically used like a verb:
 *     Config cfg = //load cfg somehow
 *     LoadNetwork load(cfg);
 */
class LoadNetwork : private boost::noncopyable {
public:
	///Load the road network for a given Config file.
	LoadNetwork(sim_mob::Config& cfg);

protected:

private:
	//Internal helper functions


private:
	//The config file we are currently using as a reference.
	Config& cfg;
};

}
