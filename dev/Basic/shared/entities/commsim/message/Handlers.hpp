//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <jsoncpp/json/json.h>

#include "entities/commsim/message/HandlerBase.hpp"

namespace sim_mob {

class OpaqueSendMessage;
class OpaqueReceiveMessage;

///Handy lookup class for handlers.
class HandlerLookup {
public:
	HandlerLookup();
	~HandlerLookup();

	///Retrieve a message handler for a given message type.
	///If any custom handlers are defined, the default type is ignored and the custom type is returned instead.
	const Handler* getHandler(const std::string& msgType) const;

	///Add an override for a known handler type. Fails if an override has already been specified.
	void addHandlerOverride(const std::string& mType, const Handler* handler);

private:
	///Handy lookup for default handler types.
	std::map<std::string, const Handler*> defaultHandlerMap;

	///Lookup for application-defined (custom) types.These override the default types.
	std::map<std::string, const Handler*> customHandlerMap;
};


///A handler that does nothing.
class NullHandler : public Handler {
	virtual ~NullHandler() {}
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const {}
};

///A handler that throws an exception
class ObsoleteHandler : public Handler {
	virtual ~ObsoleteHandler() {}
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const {
		throw std::runtime_error("MULTICAST/UNICAST messages are obsolete (or, encountered another obsolete message type). Use OPAQUE_SEND and OPAQUE_RECEIVE instead.");
	}
};

class AllLocationHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class AgentsInfoHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

/*class OpaqueSendHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class OpaqueReceiveHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};*/

class RemoteLogHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class RerouteRequestHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class NewClientHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};


//OpaqueSend/Receive are more complex; we might move them later.
class OpaqueSendHandler : public sim_mob::Handler {
public:
	OpaqueSendHandler(bool useNs3) : useNs3(useNs3) {}

	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;

private:
	//Called whenever a client is found that we must dispatch a message to.
	//Behavior differs for ns3 versus android-only.
	//TODO: The client handler can't really be const, since we are expecting to respond to this messsage at some point (which may modify the client).
	void handleClient(const sim_mob::ClientHandler& clientHdlr, std::vector<std::string>& receiveAgentIds, sim_mob::Broker& broker, const OpaqueSendMessage& currMsg) const;

	//Called when all client have been processed and messages may now be sent.
	//Behavior only exists for ns-3 (where messages are delayed).
	void postPendingMessages(sim_mob::Broker& broker, const sim_mob::Agent& agent, const std::vector<std::string>& receiveAgentIds, const OpaqueSendMessage& currMsg) const;

private:
	const bool useNs3;
};


class OpaqueReceiveHandler : public sim_mob::Handler {
public:
	OpaqueReceiveHandler(bool useNs3) : useNs3(useNs3) {}

	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;

private:
	const bool useNs3;
};



}

