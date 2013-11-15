//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Message.hpp"

sim_mob::comm::Message::Message(const sim_mob::comm::MsgData& data_) : data(data_)
{}

sim_mob::comm::MsgHandler sim_mob::comm::Message::supplyHandler()
{
	return handler;
}

void sim_mob::comm::Message::setHandler(sim_mob::comm::MsgHandler handler_)
{
	handler = handler_;
}

sim_mob::comm::MsgData& sim_mob::comm::Message::getData()
{
	return data;
}
