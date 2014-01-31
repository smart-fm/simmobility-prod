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
#include "entities/commsim/Broker.hpp"
#include "logging/Log.hpp"

namespace sim_mob {

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr &sess_, ConnectionServer &server_, Broker& broker, boost::shared_ptr<sim_mob::ConnectionHandler> existingConn)
	: sess(sess_), server(server_), broker(broker), existingConn(existingConn)
{
}

void WhoAreYouProtocol::queryAgentAsync()
{
	//We'll need to make a new ConnectionHandler if we don't have an existing one.
	if (!existingConn) {
		existingConn.reset(new ConnectionHandler(sess, broker.getMessageReceiveCallBack()));
	}

	//Inform the Broker about this session.
	broker.insertIntoWaitingOnWHOAMI(existingConn);

	//At this point, we have a ConnectionHandler that can at least receive messages. So send the "WHOAREYOU" request.
	//This will be received by the Broker, and added to the messageReceived() callback, which should then be filtered as expected.
	existingConn->forwardMessage(JsonParser::makeWhoAreYouPacket());
}

/*sim_mob::ClientRegistrationRequest WhoAreYouProtocol::getSubscriptionRequest(std::string input)
{
	sim_mob::ClientRegistrationRequest candidate;
	JsonParser::get_WHOAMI(input, candidate.client_type, candidate.clientID,candidate.requiredServices);
	candidate.session_ = sess;
	return candidate;
}*/

//void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e) {
	/*if(e) {
		Warn() <<"Failed to send WHOAREYOU message: " <<e.category().name() <<" : " <<e.value() << " : " <<e.message() <<std::endl;
		return;
	}*/

	//Save this session on the Broker for later.
	//broker.insertIntoWaitingOnWHOAMI(sess);

	//Start listening to this socket if it's new.
	//TODO: It might make more sense to migrate this behavior to the Broker anyway.
	//NOTE: The wait-for-read will happen anyway in the ConnectionHandler.
/*	if (!existingConn) {
		sess->async_read(response,
			boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
			boost::asio::placeholders::error));
	}*/


	//NOTE: This code assumes that only one client is sending on the socket, which is incorrect.
	//      Fortunately, we can handle WHOAMI in the normal message loop.
	/*else {
		sess->async_read(response,
			boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
			boost::asio::placeholders::error));
	}*/
//}

/*void WhoAreYouProtocol::WhoAreYou_response_handler(const boost::system::error_code& e) {
	//TODO: This type of functionality needs to go into Broker.
	if(e) {
		Warn() <<"WhoAreYou_response_handler Fail: " <<e.category().name() <<" : " <<e.value() << " : " <<e.message() <<std::endl;
		return;
	}*/

	/*pckt_header pHeader;
	msg_header mHeader;
	Json::Value root;
	if(sim_mob::JsonParser::parsePacketHeader(response,pHeader,root)) {
		if(sim_mob::JsonParser::getPacketMessages(response,root)) {
			std::string str = root[0].toStyledString();
			if((sim_mob::JsonParser::parseMessageHeader(str,mHeader))&&(mHeader.msg_type == "WHOAMI")) {
				sim_mob::ClientRegistrationRequest request = getSubscriptionRequest(str);
				//server.RequestClientRegistration(request, existingConn); //TODO: Elsewhere?
				delete this; //first time in my life! people say it is ok.
			}
		}
	}*/
//}

} /* namespace sim_mob */
