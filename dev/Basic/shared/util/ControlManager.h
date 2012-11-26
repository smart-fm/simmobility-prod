/*
 * ControlManager.h
 *
 *  Created on: Nov 24, 2012
 *      Author: redheli
 */

#ifndef CONTROLMANAGER_H_
#define CONTROLMANAGER_H_

#include <sys/poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <boost/thread.hpp>

namespace sim_mob {
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
	static ControlManager* GetInstance();
	void start();
	~ControlManager();
	void setSimState(int s) { boost::mutex::scoped_lock local_lock(lock); simState = s;}
	int getSimState() { return simState; }
	void getLoadScenarioParas(std::map<std::string,std::string> &para) { para=loadScenarioParas; }
	bool handleInput(std::string& input);
private:
	ControlManager();
	static ControlManager *instance;
	struct pollfd fds;
	int simState;
	std::map<std::string,std::string> loadScenarioParas;
	boost::mutex lock;
};

}
#endif /* CONTROLMANAGER_H_ */
