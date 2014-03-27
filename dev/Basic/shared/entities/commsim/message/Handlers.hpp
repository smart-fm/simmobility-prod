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

	//Retrieve a message handler for a given message type.
	const Handler* getHandler(const std::string& msgType);

private:
	//Handy lookup for handler types.
	std::map<std::string, const Handler*> HandlerMap;
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


}

