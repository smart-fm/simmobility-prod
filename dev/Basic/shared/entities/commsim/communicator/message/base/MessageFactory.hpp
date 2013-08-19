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
