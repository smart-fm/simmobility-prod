/*
 * MTConfig.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "MT_Config.hpp"

namespace sim_mob {
namespace medium {
MT_Config::MT_Config() {
	// TODO Auto-generated constructor stub

}

MT_Config::~MT_Config() {
	// TODO Auto-generated destructor stub
}

MT_Config* MT_Config::instance(nullptr);

MT_Config& MT_Config::GetInstance()
{
	if (!instance) {
		instance = new MT_Config();
	}
	return *instance;
}

void MT_Config::processElement(xercesc::DOMElement* node,
		const std::string& name) {
	if (name == "dwelling_time_parameters") {
		DOMElement* subNode = GetSingleElementByName(node, "parameters");
		if (subNode == nullptr) {
			return;
		}
		std::string value = ParseString(
				GetNamedAttributeValue(subNode, "value"), "");
		std::vector<std::string> valArray;
		boost::split(valArray, value, boost::is_any_of(", "),
				boost::token_compress_on);
		for (std::vector<std::string>::const_iterator it = valArray.begin();
				it != valArray.end(); it++) {
			try {
				double val = boost::lexical_cast<int>(*it);
				paramsDwellingTime.push_back(val);
			} catch (...) {
				Print() << "load parameters errors in MT_Config\n";
			}
		}
	}
	else if(name == "pedestrian_walk_speed"){
		DOMElement* subNode = GetSingleElementByName(node, "parameters");
		if (subNode == nullptr) {
			return;
		}
		double val = ParseFloat(
				GetNamedAttributeValue(subNode, "value"), nullptr);
		paramsWalkSpeed.push_back(val);
	}
}

const std::vector<int>& MT_Config::getParamsDwellingTime() const
{
	return paramsDwellingTime;
}

const std::vector<double>& MT_Config::getParamsWalkSpeed() const
{
	return paramsWalkSpeed;
}


}
}

