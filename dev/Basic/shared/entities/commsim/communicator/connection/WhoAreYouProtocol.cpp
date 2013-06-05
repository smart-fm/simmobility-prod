/*
 * WhoAreYouProtocol.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "WhoAreYouProtocol.hpp"
#include "ConnectionServer.hpp"
#include "Session.hpp"
#include "entities/commsim/communicator/serialization/Serialization.hpp"
#include "logging/Log.hpp"

namespace sim_mob {

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr sess_, ConnectionServer &server_):sess(sess_),server(server_),registerSuccess(false){
	}
	void WhoAreYouProtocol::start()
	{
		startClientRegistration(sess);
	}
	bool WhoAreYouProtocol::isDone()
	{
		return registerSuccess;
	}

//	void WhoAreYouProtocol::getTypeAndID(std::string& input, std::string & type, std::string & ID)
//	{
//		 JsonParser::get_WHOAMI(input, type, ID);
//	}
	sim_mob::ClientRegistrationRequest WhoAreYouProtocol::getSubscriptionRequest(std::string input, session_ptr sess)
	{
//		Print() << "Inside getSubscriptionRequest" << std::endl;
		sim_mob::ClientRegistrationRequest candidate;
		JsonParser::get_WHOAMI(input, candidate.client_type, candidate.clientID,candidate.requiredServices);
//		Print() << "Inside getSubscriptionRequest, after get_WHOAMI" << std::endl;
		candidate.session_ = sess;
//		JsonParser::get_WHOAMI_Services(input,candidate.requiredServices);
//		 Print() << "Inside getSubscriptionRequest, after getServices" << std::endl;
//		Print() << "WhoAreYouProtocol::getSubscriptionRequest=> candidate.client_type = " << candidate.client_type << " candidate.clientID = " <<  candidate.clientID << std::endl;
//		Print() << "WhoAreYouProtocol::getSubscriptionRequest=> candidate.requiredServices.size() = " << candidate.requiredServices.size() << std::endl;
		return candidate;
	}

	void WhoAreYouProtocol::startClientRegistration(session_ptr sess) {
		std::string str = JsonParser::makeWhoAreYou();
//		std::cout<< " WhoAreYou protocol Sending [" << str << "]" <<  std::endl;
		sess->async_write(str,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_handler, this,
						boost::asio::placeholders::error, sess));
	}
	void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess) {
//		std::cout<< " WhoAreYou_handler readring" << std::endl;
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
//			std::cout<< " WhoAreYou_handler read Done [" << response << "]"  << std::endl;
			//response string is successfully populated
			unsigned int id = -1 , type = -1;

			pckt_header pHeader;
			msg_header mHeader;
			Json::Value root;
			if(sim_mob::JsonParser::parsePacketHeader(response,pHeader,root))
			{
				if(sim_mob::JsonParser::getPacketMessages(response,root))
				{
					unsigned int i = 0;
					std::string str = root[i].toStyledString();
//					Print()<< "Message is: '" << str << "'" << std::endl;
					if(sim_mob::JsonParser::parseMessageHeader(str,mHeader))
					{
//				        getTypeAndID(response, type, id);
//				        std::cout << "response = " << id << " " << type << std::endl;
				        sim_mob::ClientRegistrationRequest request = getSubscriptionRequest(str, sess);
//				        Print() << "sim_mob::ClientRegistrationRequest.reqServ.size() = " << request.requiredServices.size() << std::endl;
				        server.RequestClientRegistration(request);
//						std::cout << "registering the client Done" << std::endl;
						delete this; //first time in my life! people say it is ok.

					}
				}
			}


		}
	}
} /* namespace sim_mob */
