//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <stdexcept>
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
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const {}
};

///A handler that throws an exception
class ObsoleteHandler : public Handler {
	virtual ~ObsoleteHandler() {}
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const {
		throw std::runtime_error("MULTICAST/UNICAST messages are obsolete (or, encountered another obsolete message type). Use OPAQUE_SEND and OPAQUE_RECEIVE instead.");
	}
};

///A handler that informs us the Broker has made a mistake.
class BrokerErrorHandler : public Handler {
	virtual ~BrokerErrorHandler() {}
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const {
		throw std::runtime_error("ERROR: Broker-specific message was passed on to the Handlers array (these messages must not be pended).");
	}
};

class RemoteLogHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const;
};

class RerouteRequestHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const;
};


//OpaqueSend/Receive are more complex; we might move them later.
class OpaqueSendHandler : public sim_mob::Handler {
public:
	OpaqueSendHandler(bool useNs3) : useNs3(useNs3) {}

	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const;

private:
	const bool useNs3;
};


class OpaqueReceiveHandler : public sim_mob::Handler {
public:
	OpaqueReceiveHandler(bool useNs3) : useNs3(useNs3) {}

	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, BrokerBase* broker) const;

private:
	const bool useNs3;
};



}

