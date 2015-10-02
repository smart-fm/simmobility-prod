#include "ParseShortTermConfigFile.hpp"

using namespace xercesc;

namespace sim_mob
{

ParseShortTermConfigFile::ParseShortTermConfigFile(const std::string &configFileName,
						   ST_Config &result, ConfigParams &cfgPrms):
    ParseConfigXmlBase(configFileName), cfg(result), configParams(cfgPrms)
{
    parseXmlAndProcess();
}

void ParseShortTermConfigFile::processXmlFile(XercesDOMParser& parser)
{
    DOMElement* rootNode = parser.getDocument()->getDocumentElement();
    //Verify that the root node is "config"
    if (TranscodeString(rootNode->getTagName()) != "config")
    {
        throw std::runtime_error("xml parse error: root node must be \"config\"");
    }
}

void ParseShortTermConfigFile::processProcMapNode(DOMElement* node)
{

}

void ParseShortTermConfigFile::processAmodControllerNode(DOMElement* node)
{
    if(node)
    {
        ///Read the attribute value indicating whether AMOD is enabled or disabled
        cfg.amod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
    }
}

void ParseShortTermConfigFile::processFmodControllerNode(DOMElement* node)
{
    if(node)
    {
        ///The fmod tag has an attribute
        cfg.fmod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

        ///Now set the rest.
        cfg.fmod.ipAddress = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "ip_address"), "value"), "");

        cfg.fmod.port = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "port"), "value"),
                                              static_cast<unsigned int>(0));

        cfg.fmod.mapfile = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "map_file"), "value"), "");

        cfg.fmod.blockingTimeSec = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "blocking_time_Sec"),
                                                                                "value"), static_cast<unsigned int>(0));
    }
}

void ParseShortTermConfigFile::processSegmentDensityNode(DOMElement* node)
{
    if(node)
    {
        cfg.segDensityMap.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "outputEnabled"), "false");
        if(cfg.segDensityMap.outputEnabled)
        {
            cfg.segDensityMap.updateInterval = ParseUnsignedInt(GetNamedAttributeValue(node, "updateInterval"), 1000);
            cfg.segDensityMap.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "private/DensityMap.csv");

            if(cfg.segDensityMap.updateInterval == 0)
            {
                throw std::runtime_error("ParseConfigFile::ProcessShortDensityMapNode - Update interval for aggregating density is 0");
            }

            if(cfg.segDensityMap.fileName.empty())
            {
                throw std::runtime_error("ParseConfigFile::ProcessShortDensityMapNode - File name is empty");
            }
        }
    }
}

}
