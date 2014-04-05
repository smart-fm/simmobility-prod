//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>

namespace sim_mob {

//New base message class; contains everything a Message is guranteed to have. 
//Does NOT support dynamic inheritance; never store a subclass using this class as a pointer.
struct MessageBase {
	//std::string sender_id;   ///<Who sent this message (from a communication point-of-view; see Multicast)
	//std::string sender_type; ///<The "type" of this sender (to be removed).
	std::string msg_type;    ///<The "type" of this message. Used to identify the subclass.
	//std::string msg_cat;     ///<The "category" of this message (to be removed).
};


}

