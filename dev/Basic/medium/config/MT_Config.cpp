/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"

#include <string>
#include "util/XmlParseHelper.hpp"

namespace sim_mob
{
namespace medium
{
MT_Config::MT_Config() : pedestrianWalkSpeed(0) {}

MT_Config::~MT_Config() {}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::GetInstance()
{
	if (!instance)
	{
		instance = new MT_Config();
	}
	return *instance;
}

void MT_Config::processElement(xercesc::DOMElement* root)
{
	processDwellTimeElement(root);
	processWalkSpeedElement(root);
}

void MT_Config::processDwellTimeElement(xercesc::DOMElement* node)
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

void MT_Config::processWalkSpeedElement(xercesc::DOMElement* node)
{
	DOMElement* walkSpeedNode = GetSingleElementByName(node, "pedestrian_walk_speed", false);
	if (walkSpeedNode == nullptr)
	{
		throw std::runtime_error("missing element pedestrian_walk_speed in MT-config");
	}
	else
	{
		pedestrianWalkSpeed = ParseFloat(GetNamedAttributeValue(walkSpeedNode, "value", true), nullptr);
	}
}

}
}

