/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   MT_Config.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once
#include "conf/RawConfigFile.hpp"
namespace sim_mob {
namespace medium {
const std::string MT_CONFIG_FILE = "data/medium/mt-config.xml";
const unsigned int NUM_PARAMS_DWELLTIME = 5;
class MT_Config : public RawConfigFile {
public:
	MT_Config();
	virtual ~MT_Config();

	static MT_Config& GetInstance();

	const std::vector<int>& getParamsDwellingTime() const;
	const std::vector<double>& getParamsWalkSpeed() const;

protected:
	/**
	 * process each node included in xml file.
	 * @param name is node's name
	 * @param node is a element inside xml file
	 */
	virtual void processElement(xercesc::DOMElement* node, const std::string& name);
private:
	static MT_Config* instance;
	/**store parameters for dwelling time calculation*/
	std::vector<int> paramsDwellingTime;
	/**store parameters for pedestrian walking speed*/
	std::vector<double> paramsWalkSpeed;
};
}
}


