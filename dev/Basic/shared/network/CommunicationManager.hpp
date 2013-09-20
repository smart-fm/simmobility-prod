//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CommunicationManager.hpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace sim_mob {

class CommunicationDataManager;
class ControlManager;


class CommunicationManager {

public:
	CommunicationManager(int port, CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr);
	void start();
	bool isCommDone() { return CommDone; }
	void setCommDone(bool b) { CommDone = b; }
	void setSimulationDone(bool b) { simulationDone = b; }
	bool isSimulationDone() { return simulationDone; }

private:
	CommunicationDataManager* comDataMgr;
	ControlManager* ctrlMgr;
	boost::asio::io_service io_service;
	int listenPort;

private:
	bool simulationDone;
	bool CommDone;
};

}
