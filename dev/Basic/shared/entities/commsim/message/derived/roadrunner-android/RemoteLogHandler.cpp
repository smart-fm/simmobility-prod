//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "RemoteLogHandler.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;


//handler implementation
/*void sim_mob::roadrunner::RemoteLogHandler::handle(sim_mob::comm::MsgPtr message_,Broker* broker, boost::shared_ptr<sim_mob::ConnectionHandler> caller)
{
	//Just parse it manually.
	sim_mob::comm::MsgData &data = message_->getData();
	//Json::Value msgType = data.get("MESSAGE_TYPE", "UNKNOWN"); //This will always be "REMOTE_LOG"
	//Json::Value msgType = data.get("SENDER_TYPE", "UNKNOWN"); //This will always be "ANDROID_EMULATOR"
	Json::Value sender = data.get("SENDER", "UNKNOWN"); //Who sent this message.
	Json::Value logMsg = data.get("log_message", "UNDFINED"); //The message to log.

	//Now we can just log it.
	//At the moment, we are so many levels removed from Broker that we'll just put it on stdout.
	//Ideally, it would (eventually) go into out.txt.
	Print() <<"Client [" <<sender.asString() <<"] relayed remote log message: \"" <<logMsg.asString() <<"\"\n";
}


*/
