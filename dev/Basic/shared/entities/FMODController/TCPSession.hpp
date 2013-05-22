/*
 * TCPSession.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef TCPSESSION_HPP_
#define TCPSESSION_HPP_
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

namespace sim_mob {

namespace FMOD
{

class TCPSession : public boost::enable_shared_from_this<TCPSession> {
public:
	TCPSession(boost::asio::io_service& io_service);

	virtual ~TCPSession();

	boost::asio::ip::tcp::socket& socket();
private:

	boost::asio::ip::tcp::socket socket_;

};

}

} /* namespace sim_mob */
#endif /* TCPSESSION_HPP_ */
