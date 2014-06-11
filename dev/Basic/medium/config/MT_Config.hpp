/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   MT_Config.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once
#include <vector>
#include <xercesc/dom/DOMElement.hpp>

#include "conf/RawConfigFile.hpp"

namespace sim_mob
{
namespace medium
{

class MT_Config: public RawConfigFile
{
public:
	MT_Config();
	virtual ~MT_Config();

	static MT_Config& GetInstance();

	double getPedestrianWalkSpeed() const
	{
		return pedestrianWalkSpeed;
	}

	const std::vector<int>& getDwellTimeParams() const
	{
		return dwellTimeParams;
	}

protected:
	/**
	 * processes a node included in xml file.
	 * @param root root node inside xml file
	 */
	virtual void processElement(xercesc::DOMElement* root);

private:
	/**
	 * processes dwell time element included in xml file.
	 * @param node node corresponding to the dwell time element inside xml file
	 */
	void processDwellTimeElement(xercesc::DOMElement* node);

	/**
	 * processes pedestrian walk speed included in xml file.
	 * @param node node corresponding to pedestrian walk speed element inside xml file
	 */
	void processWalkSpeedElement(xercesc::DOMElement* node);

	static MT_Config* instance;
	/**store parameters for dwelling time calculation*/
	std::vector<int> dwellTimeParams;
	/**store parameters for pedestrian walking speed*/
	double pedestrianWalkSpeed;
};
}
}

