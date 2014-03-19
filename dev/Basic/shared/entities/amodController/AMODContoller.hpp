//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODContoller.hpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#ifndef AMODCONTOLLER_HPP_
#define AMODCONTOLLER_HPP_

#include "AMODClient.hpp"

namespace sim_mob {

namespace AMOD {

class AMODContoller {
public:
	AMODContoller();
	virtual ~AMODContoller();

	/**
	  * connect to AMOD simulator
	  * @return true if successfully .
	  */
	bool connectAmodService();

private:
	//identify whether or not communication is created
	bool isConnectFmodServer;
private:
	AmodClientPtr connectPoint;
	std::string ipAddress;
	std::string mapFile;
	int port;
	int updateTiming;
	int frameTicks;
	int waitingseconds;
private:
	static AMODContoller* pInstance;
	static boost::asio::io_service ioService;
};

} /* namespace AMOD */
} /* namespace sim_mob */
#endif /* AMODCONTOLLER_HPP_ */
