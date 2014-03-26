//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/shared_ptr.hpp>

namespace sim_mob {

class ConnectionHandler;
class MessageConglomerate;
class Broker;

///A base class for anything that can "handle" a message. Note that the message is passed in 
/// as a Json object, so handlers should proceed to parse the type of message they expect in the 
/// handle() method. (The serialize.h functions can help with this.)
class Handler {
public:
	virtual ~Handler() {}
	virtual void handle(boost::shared_ptr<ConnectionHandler> handler, const MessageConglomerate& messages, int msgNumber, Broker* broker) const = 0;
};

}

