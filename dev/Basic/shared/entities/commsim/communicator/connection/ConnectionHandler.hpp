/*
 * ConnectionHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CONNECTIONHANDLER_HPP_
#define CONNECTIONHANDLER_HPP_
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
namespace sim_mob {

//Macro used for callbacks
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))
//Forward Declaration
class Broker;
class Session;
class ConnectionHandler;
class ConnectionHandler: public boost::enable_shared_from_this<ConnectionHandler>
{
	boost::shared_ptr<Session> mySession;
	typedef void (Broker::*messageReceiveCallback)(boost::shared_ptr<ConnectionHandler>,std::string);
	messageReceiveCallback receiveCallBack;
	Broker &theBroker;
	std::string incomingMessage;
public:
	//metadata
	//some of such data is duplicated in the broker client list entries
	unsigned int agentPtr,clientType;
	std::string clientID;
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(
			boost::shared_ptr<Session> session_ ,
			Broker& broker,
			messageReceiveCallback callback,
			std::string clientID_ = 0,
			unsigned int ClienType_ = 0,
			unsigned long int agentPtr_ = 0l
			);
	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
};//ConnectionHandler

} /* namespace sim_mob */
#endif /* CONNECTIONHANDLER_HPP_ */
