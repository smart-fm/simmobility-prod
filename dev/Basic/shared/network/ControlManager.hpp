//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ControlManager.hpp
 *
 *  Created on: Nov 24, 2012
 *      Author: redheli
 */

#pragma once

#include <sys/poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <boost/thread.hpp>

namespace sim_mob {

class ConfigParams;

enum SIMSTATE
{
	IDLE=0,
	RUNNING=1,
	PAUSE=2,
	STOP=3,
	NOTREADY=4,
	READY=5,
	LOADSCENARIO=6,
	QUIT
};
class ControlManager {
public:
	void start();
	void setSimState(int s);
	int getSimState();
	void getLoadScenarioParas(std::map<std::string,std::string> &para) { para=loadScenarioParas; }
	bool handleInput(std::string& input);
	void setEndTick(int t);
	int getEndTick() { return endTick;}
private:
	ControlManager();
	struct pollfd fds;
	int simState;
	std::map<std::string,std::string> loadScenarioParas;
	boost::mutex lock;
	boost::mutex lockEndTick;
	int endTick;

	//Again, ControlManager's constructor is private for now (since we need to guarantee only one of these),
	//  but to avoid the singleton pattern, we place the global ControlManager object into ConfigParams.
	friend class sim_mob::ConfigParams;
};

}
