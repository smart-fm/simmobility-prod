//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

/**
 * \file Types.hpp
 * This file contains several types in the sim_mob::comm namespace which are frequently used in header definitions.
 *
 * \note
 * A typedef inside the sim_mob namespace is usually not a good idea, since so many files will #using the entire namespace.
 * Hence, I've moved these definitions to sim_mob::comm. ~Seth
 */

/*#include <boost/shared_ptr.hpp>
#include <json/json.h>

namespace sim_mob {

//Forward declaration
class Handler;

namespace comm {

//Forward declaration.
class Message;

///The type of the "data" element of a Message class.
typedef Json::Value MsgData;

///The type of a pointer to a Message class (usually a shared_ptr).
typedef boost::shared_ptr<sim_mob::comm::Message> MsgPtr;

///The type of a Handler for a Message class (usually a shared_ptr).
typedef boost::shared_ptr<sim_mob::Handler> MsgHandler;

}}*/
