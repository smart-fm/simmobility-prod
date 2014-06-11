//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseMidTermConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include "util/LangHelpers.hpp"
#include "util/XmlParseHelper.hpp"

namespace sim_mob
{
ParseMidTermConfigFile::ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result)
: ParseConfigXmlBase(configFileName), mtCfg(result)
{
	parseXmlAndProcess();
}

void ParseMidTermConfigFile::processXmlFile(xercesc::XercesDOMParser& parser)
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	//Verify that the root node is "config"
	if (TranscodeString(rootNode->getTagName()) != "config") {
		throw std::runtime_error("xml parse error: root node must be \"config\"");
	}

	processDwellTimeElement(rootNode);
	processWalkSpeedElement(rootNode);
}

void ParseMidTermConfigFile::processDwellTimeElement(xercesc::DOMElement* node)
{
	DOMElement* subNode = GetSingleElementByName(node, "dwelling_time_parameters", false);
	if (subNode == nullptr)
	{
		throw std::runtime_error("do not find element dwelling_time_parameters in MT-config");
	}
	else
	{
		DOMElement* child = GetSingleElementByName(subNode, "parameters");
		if (child == nullptr)
		{
			throw std::runtime_error("load dwelling-time parameters errors in MT_Config");
		}
		std::string value = ParseString(GetNamedAttributeValue(child, "value"), "");
		std::vector<std::string> valArray;
		boost::split(valArray, value, boost::is_any_of(", "), boost::token_compress_on);
		std::vector<int>& dwellTimeParams = mtCfg.getDwellTimeParams();
		for (std::vector<std::string>::const_iterator it = valArray.begin(); it != valArray.end(); it++)
		{
			try
			{
				int val = boost::lexical_cast<int>(*it);
				dwellTimeParams.push_back(val);
			} catch (...)
			{
				throw std::runtime_error("load dwelling-time parameters errors in MT_Config");
			}
		}
	}
}

void ParseMidTermConfigFile::processWalkSpeedElement(xercesc::DOMElement* node)
{
	DOMElement* walkSpeedNode = GetSingleElementByName(node, "pedestrian_walk_speed", false);
	if (walkSpeedNode == nullptr)
	{
		throw std::runtime_error("missing element pedestrian_walk_speed in MT-config");
	}
	else
	{
		mtCfg.setPedestrianWalkSpeed(ParseFloat(GetNamedAttributeValue(walkSpeedNode, "value", true), nullptr));
	}
}

}


