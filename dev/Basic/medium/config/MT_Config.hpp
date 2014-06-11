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

namespace sim_mob
{
namespace medium
{

class MT_Config
{
public:
	MT_Config();
	virtual ~MT_Config();

	static MT_Config& GetInstance();

	double getPedestrianWalkSpeed() const
	{
		return pedestrianWalkSpeed;
	}

	std::vector<int>& getDwellTimeParams()
	{
		return dwellTimeParams;
	}

	void setPedestrianWalkSpeed(double pedestrianWalkSpeed)
	{
		this->pedestrianWalkSpeed = pedestrianWalkSpeed;
	}

private:
	static MT_Config* instance;
	/**store parameters for dwelling time calculation*/
	std::vector<int> dwellTimeParams;
	/**store parameters for pedestrian walking speed*/
	double pedestrianWalkSpeed;
};
}
}

