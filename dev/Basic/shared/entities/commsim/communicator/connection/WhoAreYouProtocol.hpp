/*
 * WhoAreYouProtocol.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef WHOAREYOUPROTOCOL_HPP_
#define WHOAREYOUPROTOCOL_HPP_
#include <map>
#include <entities/commsim/communicator/client-registration/base/ClientRegistration.hpp>
class Session;
class ConnectionServer;

namespace sim_mob {

class WhoAreYouProtocol
{
public:
	WhoAreYouProtocol(session_ptr sess_, boost::shared_ptr<ConnectionServer> server__);
	void start();
	bool isDone();
	void getTypeAndID(std::string &input, unsigned int & out_type, unsigned int & out_ID);
	sim_mob::ClientRegistrationRequest getSubscriptionRequest(std::string, session_ptr);
	std::string response; //json string containing ID & type of the client
private:
	session_ptr sess;
	ConnectionServer *server_;
	bool registerSuccess;
	std::map<unsigned int, session_ptr> clientRegistrationWaitingList;
	void startClientRegistration(session_ptr sess);
	void WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess);
	void WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess);
};

} /* namespace sim_mob */
#endif /* WHOAREYOUPROTOCOL_HPP_ */
