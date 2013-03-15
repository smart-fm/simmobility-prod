/*
 * ControlManager.cpp
 *
 *  Created on: Nov 24, 2012
 *      Author: redheli
 */

#include "ControlManager.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

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
	endTick=-1;
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
	std::cout<<"simmob"<<">"<<std::flush;
	while(1)
	 {
		ret = poll(&fds, 1, 0);
		if(ret == 1)
		{
//			std::cout<<"Yep get input: ";
			char buff[255] = "\0";
			read(fds.fd, buff, 255);
			std::string cmd = buff;
			handleInput(cmd);
//			std::cout<<cmd;
//			boost::erase_all(cmd, "\n");
//			if(cmd == "pause")
//			{
//				simState=PAUSE;
//			}
//			if(cmd == "run")
//				simState=RUNNING;
			std::cout<<"simmob"<<">"<<std::flush;
		}
		sleep(0.01);
	 } // end of while
}
bool sim_mob::ControlManager::handleInput(std::string& input)
{
	boost::erase_all(input, "\n");
	if(input.size()<3)
	{
		return false;
	}
	std::cout<<input<<std::endl;
	boost::char_separator<char> sep(" ");
	boost::tokenizer<boost::char_separator<char> > tokens(input, sep);
	//for(boost::tokenizer<boost::char_separator<char> >::iterator beg=tokens.begin(); beg!=tokens.end();++beg)
//	{
	boost::tokenizer<boost::char_separator<char> >::iterator beg=tokens.begin();
	std::string cmd = *beg;
	std::cout<<cmd<< std::endl;
	if(cmd == "pause")
	{
//		simState=PAUSE;
		setSimState(PAUSE);
		return true;
	}
	else if(cmd == "run")
	{
//		simState=RUNNING;
		setSimState(RUNNING);
		return true;
	}
	else if(cmd == "load")
	{
		beg++;
		if(beg == tokens.end())
		{
			std::cout<<"no scenario file name "<<std::endl;
			return true;
		}
		std::string configFileName = *beg;
		loadScenarioParas.clear();
		loadScenarioParas["configFileName"] = configFileName;
//		simState=LOADSCENARIO;
		setSimState(LOADSCENARIO);
		return true;
	}
	else if(cmd == "stop")
	{
		setSimState(STOP);
		return true;
	}
	else if(cmd == "quit")
	{
		setSimState(QUIT);
		return true;
	}
	else
	{
		std::cout<<"unknow command"<<std::endl;
		return false;
	}
//	}

//	std::cout<<"simmob"<<">"<<std::flush;
	return true;
}
sim_mob::ControlManager::~ControlManager() {
	if(instance)
		delete instance;
}

