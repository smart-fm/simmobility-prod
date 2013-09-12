//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>


sim_mob::ControlManager::ControlManager()
	:simState(NOTREADY)
{
	endTick=-1;
	 int flags;
	 fds.fd = 0; /* this is STDIN */
	 fds.events = POLLIN;
	 if ((flags = fcntl(fds.fd, F_GETFL)) == -1) {
		  std::cout<<"first fcntl() failed"<<std::endl;
	 } else if (fcntl(fds.fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cout<<"second fcntl() failed"<<std::endl;
	 }
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
			char buff[255] = "\0";
			read(fds.fd, buff, 255);
			std::string cmd = buff;
			handleInput(cmd);
			std::cout<<"simmob"<<">"<<std::flush;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	 } // end of while
}

void sim_mob::ControlManager::setSimState(int s)
{
	boost::mutex::scoped_lock local_lock(lock);
	simState = s;
	std::cout<<"simmob"<<">"<<std::flush;
}

int sim_mob::ControlManager::getSimState()
{
	boost::mutex::scoped_lock local_lock(lock);
	return simState;
}


void sim_mob::ControlManager::setEndTick(int t)
{
	boost::mutex::scoped_lock local_lock(lockEndTick);
	if (endTick<0) {
		endTick = t;
	}
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
	boost::tokenizer<boost::char_separator<char> >::iterator beg=tokens.begin();
	std::string cmd = *beg;
	std::cout<<cmd<< std::endl;
	if(cmd == "pause")
	{
		setSimState(PAUSE);
		return true;
	}
	else if(cmd == "run")
	{
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

	return true;
}


