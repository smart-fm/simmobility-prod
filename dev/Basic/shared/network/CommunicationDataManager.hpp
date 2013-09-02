/*
 * CommunicationManager.hpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <queue>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>

namespace sim_mob {

class ConfigParams;
class ControlManager;

class CommunicationDataManager {
public:
	void sendTrafficData(std::string &s);
	void sendRoadNetworkData(const std::string &s);
	bool getTrafficData(std::string &s);
	bool getCmdData(std::string &s);
	bool getRoadNetworkData(std::string &s);
	bool isAllTrafficDataOut() { return trafficDataQueue.empty(); }

private:
	CommunicationDataManager() {}
	std::queue<std::string> trafficDataQueue;
	std::queue<std::string> cmdDataQueue;
	std::queue<std::string> roadNetworkDataQueue;
	boost::mutex trafficDataGuard;
	boost::mutex cmdDataGuard;
	boost::mutex roadNetworkDataGuard;

	//The CommunicationDataManager should be shared for *all* communication, but we want to avoid
	// making it a singleton. For now, that means we put it in ConfigParams, so ConfigParams needs
	// friend access.
	friend class sim_mob::ConfigParams;
};

}
