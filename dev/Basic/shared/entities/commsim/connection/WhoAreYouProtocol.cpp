//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WhoAreYouProtocol.cpp
 *
 *  Created on: May 29, 2013
 *      Author: vahid
 */

#include "WhoAreYouProtocol.hpp"
#include "ConnectionServer.hpp"
#include "Session.hpp"
#include "entities/commsim/serialization/JsonParser.hpp"
#include "logging/Log.hpp"

namespace sim_mob {

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr &sess_, ConnectionServer &server_):sess(sess_),server(server_),registerSuccess(false)
{
}

void WhoAreYouProtocol::start()
{
	std::string str = JsonParser::makeWhoAreYouPacket();
	sess->async_write(str,
			boost::bind(&WhoAreYouProtocol::WhoAreYou_handler/*temp_handler_1*/, this,
					boost::asio::placeholders::error));
}

bool WhoAreYouProtocol::isDone()
{
	return registerSuccess;
}

sim_mob::ClientRegistrationRequest WhoAreYouProtocol::getSubscriptionRequest(std::string input)
{
	sim_mob::ClientRegistrationRequest candidate;
	JsonParser::get_WHOAMI(input, candidate.client_type, candidate.clientID,candidate.requiredServices);
	candidate.session_ = sess;
	return candidate;
}

void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e) {
	if(e) {
		WarnOut( "Failed to send WHOAREYOU message: " <<e.category().name() <<" : " <<e.value() << " : " <<e.message() <<std::endl);
	} else {
		sess->async_read(response,
			boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
			boost::asio::placeholders::error));
	}
}

void WhoAreYouProtocol::WhoAreYou_response_handler(const boost::system::error_code& e) {
	//Warn() <<"WHO_ARE_YOU responded: " <<response <<"\n";

	if(e) {
		std::cerr << "WhoAreYou_response_handler Fail [" << e.message() << "]" << std::endl;
	} else {
		//response string is successfully populated
		unsigned int id = -1 , type = -1;

		pckt_header pHeader;
		msg_header mHeader;
		Json::Value root;
		if(sim_mob::JsonParser::parsePacketHeader(response,pHeader,root)) {
			if(sim_mob::JsonParser::getPacketMessages(response,root)) {
				unsigned int i = 0;
				std::string str = root[i].toStyledString();
				if((sim_mob::JsonParser::parseMessageHeader(str,mHeader))&&(mHeader.msg_type == "WHOAMI")) {
					sim_mob::ClientRegistrationRequest request = getSubscriptionRequest(str);
					server.RequestClientRegistration(request);
					delete this; //first time in my life! people say it is ok.
				}
			}
		}
	}
}

} /* namespace sim_mob */
