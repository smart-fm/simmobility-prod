//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <jsoncpp/json/json.h>

#include "entities/commsim/message/HandlerBase.hpp"

namespace sim_mob {

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

class AllLocationHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class AgentsInfoHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class UnicastHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

class MulticastHandler : public Handler {
public:
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const;
};

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


}

