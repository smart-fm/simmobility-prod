/*
 * ControlManager.cpp
 *
 *  Created on: Nov 24, 2012
 *      Author: redheli
 */

#include "ControlManager.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
sim_mob::ControlManager* sim_mob::ControlManager::instance = NULL;

sim_mob::ControlManager* sim_mob::ControlManager::GetInstance() {
     if (!instance) {
          instance = new ControlManager();
     }
     return instance;
}

sim_mob::ControlManager::ControlManager()
	:simState(NOTREADY)
{
	 int flags;
	 fds.fd = 0; /* this is STDIN */
	 fds.events = POLLIN;
	 if ((flags = fcntl(fds.fd, F_GETFL)) == -1)
		  std::cout<<"first fcntl() failed"<<std::endl;
	else if (fcntl(fds.fd, F_SETFL, flags | O_NONBLOCK) == -1)
		std::cout<<"second fcntl() failed"<<std::endl;
//	 while(1)
//	 {
//		ret = poll(&fds, 1, 0);
//		if(ret == 1)
//		{
//				printf("Yep get input: ");
//				char buff[255] = "\0";
//				read(fds.fd, buff, 255);
//				std::cout<<buff;
//				cout<<"simmob"<<">"<<flush;
//		}
//	 } // end of while

}
void sim_mob::ControlManager::start()
{
	int ret;
	while(1)
	 {
		ret = poll(&fds, 1, 0);
		if(ret == 1)
		{
			std::cout<<"Yep get input: ";
			char buff[255] = "\0";
			read(fds.fd, buff, 255);
//			std::cout<<buff;
			std::string cmd = buff;
			std::cout<<cmd;
			boost::erase_all(cmd, "\n");
			if(cmd == "pause")
			{
				simState=PAUSE;
			}
			if(cmd == "run")
				simState=RUNNING;
			std::cout<<"simmob"<<">"<<std::flush;
		}
		sleep(0.01);
	 } // end of while
}
sim_mob::ControlManager::~ControlManager() {
	if(instance)
		delete instance;
}

