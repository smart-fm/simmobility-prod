/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>



namespace sim_mob {

class RawConfigParams;


/**
 * Class used to parse a config file into a RawConfigParams object.
 * Typically used like a verb:
 *     RawConfigParams& cfg = ConfigParams::getInstanceRW();
 *     ParseConfigFile print(cfg);
 *
 * \note
 * This class is actually USED by the old config format (simpleconf). Don't delete it if you are cleaning
 * up the remains of the new config format (which doesn't work at the moment). ~Seth
 */
class ParseConfigFile : private boost::noncopyable {
public:
	///Parse a config file into RawConfigParams, performing all XML parsing and some trivial semantic processing.
	ParseConfigFile(const std::string& configFileName, RawConfigParams& result);

protected:
	///Does all the work.
	void ParseXmlAndProcess();

private:
	//These functions are called by ParseXmlAndProcess()
	std::string ParseXmlFile(XercesDOMParser* parser); //Returns "" or an error message.
	void ProcessXmlFile(const XercesDOMParser* parser);



private:
	//The path of the file we are loading our exact configuration from.
	std::string inFilePath;

	//The config file we are currently loading
	RawConfigParams& cfg;
};

}
