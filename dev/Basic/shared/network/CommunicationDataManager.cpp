/*
 * CommunicationManager.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include "CommunicationManager.hpp"

#include <boost/thread.hpp>

#include "CommunicationDataManager.hpp"
#include "ControlManager.hpp"

using boost::asio::ip::tcp;
using namespace sim_mob;


void sim_mob::CommunicationDataManager::sendTrafficData(std::string &s)
{
		boost::mutex::scoped_lock lock(trafficDataGuard);
		trafficDataQueue.push(s);
}
void sim_mob::CommunicationDataManager::sendRoadNetworkData(std::string &s)
{
		boost::mutex::scoped_lock lock(roadNetworkDataGuard);
		roadNetworkDataQueue.push(s);
}
bool sim_mob::CommunicationDataManager::getTrafficData(std::string &s) {
		if(!trafficDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(trafficDataGuard);
			s = trafficDataQueue.front();
			trafficDataQueue.pop();
			return true;
		}
		return false;
}
bool sim_mob::CommunicationDataManager::getCmdData(std::string &s) {
		if(!cmdDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(cmdDataGuard);
			s = cmdDataQueue.front();
			cmdDataQueue.pop();
			return true;
		}
		return false;
}
bool sim_mob::CommunicationDataManager::getRoadNetworkData(std::string &s) {
		if(!roadNetworkDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(roadNetworkDataGuard);
			s = roadNetworkDataQueue.front();
			roadNetworkDataQueue.pop();
			return true;
		}
		return false;
}
