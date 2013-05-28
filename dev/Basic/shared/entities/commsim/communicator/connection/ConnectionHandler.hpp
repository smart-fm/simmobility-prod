/*
 * ConnectionHandler.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef CONNECTIONHANDLER_HPP_
#define CONNECTIONHANDLER_HPP_

namespace sim_mob {

//Macro used for callbacks
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))
//Forward Declaration
class Broker;
class Session;

class ConnectionHandler
{
	session_ptr mySession;
	//metadata
	unsigned int clientID, agentPtr, clientType;//some of such data is duplicated in the broker client list entries
	typedef void (Broker::*messageReceiveCallback)(ConnectionHandler&,std::string);
	messageReceiveCallback receiveCallBack;
	Broker &theBroker;
public:
	//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
	//      For now I've made both work by value; you may need to modify this. ~Seth
	ConnectionHandler(
			session_ptr session_ ,
			Broker& broker,
			messageReceiveCallback callback,
			unsigned int clientID_ = 0,
			unsigned int ClienType_ = 0,
			unsigned long agentPtr_ = 0
			);
	void start();
	void readyHandler(const boost::system::error_code &e, std::string str);
	void readHandler(const boost::system::error_code& e);
	void send(std::string str);
	void sendHandler(const boost::system::error_code& e) ;
};//ConnectionHandler

} /* namespace sim_mob */
#endif /* CONNECTIONHANDLER_HPP_ */
