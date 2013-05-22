/*
 * TCPSession.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "TCPSession.hpp"

namespace sim_mob {

namespace FMOD
{

TCPSession::TCPSession(boost::asio::io_service& io_service);: socket_(io_service) {
	// TODO Auto-generated constructor stub

}

TCPSession::~TCPSession() {
	// TODO Auto-generated destructor stub
}

}

} /* namespace sim_mob */
