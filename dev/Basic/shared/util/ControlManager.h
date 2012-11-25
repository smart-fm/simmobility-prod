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

namespace sim_mob {
enum SIMSTATE
{
	IDLE=0,
	RUNNING=1,
	PAUSE=2,
	STOP=3,
	NOTREADY=4,
	READY=5
};
class ControlManager {
public:
	static ControlManager* GetInstance();
	void start();
	~ControlManager();
	void setSimState(int s) { simState = s;}
	int getSimState() { return simState; }
private:
	ControlManager();
	static ControlManager *instance;
	struct pollfd fds;
	int simState;
};

}
#endif /* CONTROLMANAGER_H_ */
