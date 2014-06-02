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
class MT_Config : public RawConfigFile {
public:
	MT_Config();
	virtual ~MT_Config();

	static const MT_Config& GetInstance();

protected:
	/**
	 * process each node included in xml file.
	 * @param name is node's name
	 * @param node is a element inside xml file
	 */
	virtual void processElement(xercesc::DOMElement* node, const std::string& name);
private:
	static MT_Config* instance;
};
}
}


