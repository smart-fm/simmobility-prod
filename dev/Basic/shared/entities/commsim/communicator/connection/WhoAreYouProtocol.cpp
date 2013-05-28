/*
 * WhoAreYouProtocol.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "WhoAreYouProtocol.hpp"
#include "ConnectionServer.hpp"
#include "Session.hpp"
#include <json/json.h>

namespace sim_mob {

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr sess_, boost::shaerd_ptr<ConnectionServer>server__):sess(sess_),server_(server__),registerSuccess(false){
	}
	void WhoAreYouProtocol::start()
	{
		startClientRegistration(sess);
	}
	bool WhoAreYouProtocol::isDone()
	{
		return registerSuccess;
	}

	void WhoAreYouProtocol::getTypeAndID(std::string& input, unsigned int & type, unsigned int & ID)
	{
		 JsonParser::getTypeAndID(input, type, ID);
	}
	sim_mob::ClientRegistrationRequest WhoAreYouProtocol::getSubscriptionRequest(std::string input, session_ptr sess)
	{

		sim_mob::ClientRegistrationRequest candidate;
		 JsonParser::getTypeAndID(input, candidate.client_type, candidate.clientID);
		candidate.session_ = sess;
		JsonParser::getServices(input,candidate.requiredServices);

	}

	void WhoAreYouProtocol::startClientRegistration(session_ptr sess) {
		std::string str = JsonParser::makeWhoAreYou();
		std::cout<< " WhoAreYou protocol Sending [" << str << "]" <<  std::endl;
		sess->async_write(str,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_handler, this,
						boost::asio::placeholders::error, sess));
	}
	void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess) {
		std::cout<< " WhoAreYou_handler readring" << std::endl;
		sess->async_read(response,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
						boost::asio::placeholders::error, sess));
	}

	void WhoAreYouProtocol::WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess) {
		if(e)
		{
			std::cerr << "WhoAreYou_response_handler Fail [" << e.message() << "]" << std::endl;
		}
		else
		{
			std::cout<< " WhoAreYou_handler read Done [" << response << "]"  << std::endl;
			//response string is successfully populated
			unsigned int id = -1 , type = -1;
			sim_mob::ClientRegistrationRequest request = getSubscriptionRequest(response, sess);
	        getTypeAndID(response, type, id);
	        std::cout << "response = " << id << " " << type << std::endl;
			server_->RequestClientRegistration(request);
			std::cout << "registering the client Done" << std::endl;
			delete this; //first time in my life! people say it is ok.


		}
	}
} /* namespace sim_mob */
