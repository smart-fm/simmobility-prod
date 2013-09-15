//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MessageFactory.hpp
 *
 *  Created on: May 13, 2013
 *      Author: vahid
 */

#pragma once

namespace sim_mob {
template <class RET,class MSG>
class MessageFactory {
public:
	virtual bool createMessage(MSG,RET ) = 0;
};

} /* namespace sim_mob */
