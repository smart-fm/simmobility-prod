/*
 * WhoAreYouProtocol.hpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#ifndef WHOAREYOUPROTOCOL_HPP_
#define WHOAREYOUPROTOCOL_HPP_
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include <entities/commsim/communicator/client-registration/base/ClientRegistration.hpp>

namespace sim_mob {
class Session;
class ConnectionServer;

class WhoAreYouProtocol
{
public:
//	sim_mob::boost::shared_ptr<sim_mob::Session> sess__;
	WhoAreYouProtocol(boost::shared_ptr<Session> sess_, ConnectionServer &);
	void start();
	bool isDone();
//	void getTypeAndID(std::string &input, std::string & out_type, std::string & out_ID);
	sim_mob::ClientRegistrationRequest getSubscriptionRequest(std::string, boost::shared_ptr<Session>);
	std::string response; //json string containing ID & type of the client
private:
	boost::shared_ptr<Session> sess;
	ConnectionServer &server;
	bool registerSuccess;
	std::map<unsigned int, boost::shared_ptr<Session> > clientRegistrationWaitingList;
	void startClientRegistration(boost::shared_ptr<Session> sess);
	void WhoAreYou_handler(const boost::system::error_code& e,boost::shared_ptr<Session> sess);
	void WhoAreYou_response_handler(const boost::system::error_code& e, boost::shared_ptr<Session> sess);
};

} /* namespace sim_mob */
#endif /* WHOAREYOUPROTOCOL_HPP_ */
