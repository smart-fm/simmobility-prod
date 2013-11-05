//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/shared_ptr.hpp>
#include <json/json.h>

namespace sim_mob {

//Forward Declaration
class Broker;

namespace comm {

//Forward declaration.
class Message;

} //End namespace comm


//todo do something here. the following std::string is spoiling messge's templatization benefits
//TODO: Need to get rid of these header-defined typedefs.
//typedef sim_mob::comm::Message msg_t;
//NOTE: A forward declaration inside of a shared_ptr is fine for headers (as long as you don't dereference it).
typedef boost::shared_ptr<sim_mob::comm::Message> msg_ptr; //putting std::string here is c++ limitation(old standard). don't blame me!-vahid

///A message handler (no documentation provided).
class Handler {
public:
	virtual ~Handler() {}

	///Handle a given message.
	virtual void handle(msg_ptr message_, sim_mob::Broker*) = 0;
};


}
