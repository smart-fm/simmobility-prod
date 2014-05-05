//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace sim_mob {

//New base message class; contains everything a Message is guranteed to have. 
//Does NOT support dynamic inheritance; never store a subclass using this class as a pointer.
struct MessageBase {
	///The "type" of this message. Used to identify the subclass.
	///TODO: We will eventually be using both integers and strings to define message types, so this
	///      class will eventually be folded into a "message type" class.
	std::string msg_type;
};


}

