//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include "ST_Config.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ParseConfigXmlBase.hpp"

namespace sim_mob
{

class ParseShortTermConfigFile : public ParseConfigXmlBase, private boost::noncopyable
{
public:
    /**
     * Parse a config file into MT_Config, performing all XML parsing and some trivial semantic processing
     * @param configFileName - config file name which needs to be parsed
     * @param result - Configuration extracted from config file
     * @param cfgPrms -
     */
    ParseShortTermConfigFile(const std::string& configFileName, ST_Config& result, ConfigParams& cfgPrms);

private:
    /**
     * Virtual override
     */
    virtual void processXmlFile(xercesc::XercesDOMParser& parser);

    void processProcMapNode(xercesc::DOMElement* node);

    void processAmodControllerNode(xercesc::DOMElement* node);

    void processFmodControllerNode(xercesc::DOMElement* node);

    void processSegmentDensityNode(xercesc::DOMElement* node);

    ST_Config& cfg;

    ConfigParams& configParams;
};

}
