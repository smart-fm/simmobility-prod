//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/commsim/message/Types.hpp"
#include "entities/commsim/message/base/Message.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob {
namespace roadrunner {

///TODO: This class doesn't seem to do much; do we need it? ~Seth
class ClientDoneMessage : public sim_mob::comm::Message {
public:
	ClientDoneMessage(const sim_mob::comm::MsgData& data_) : Message(data_) {}

	Handler* newHandler() { return nullptr; }
};

}}
