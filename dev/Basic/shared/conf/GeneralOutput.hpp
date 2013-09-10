#if 0
/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "conf/Config.hpp"


namespace sim_mob {


/**
 * Class used to print general output after loading a config file.
 * Typically used like a verb:
 *     Config cfg = //load cfg somehow
 *     GeneralOutput print(cfg);
 */
class GeneralOutput : private boost::noncopyable {
public:
	///Print general output for the config file.
	GeneralOutput(const Config& cfg);

protected:
	///Main function.
	void LogRelevantOutput() const;

private:
	//Helper functions

private:
	//The config file we are currently printing.
	const Config& cfg;
};

}
#endif
