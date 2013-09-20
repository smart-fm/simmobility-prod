//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Message.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 8, 2013, 6:16 PM
 */

#include "Message.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob::messaging;

Message::Message() : priority(5), sender(NULL) {
}

Message::Message(const Message& source) {
    this->priority = source.priority;
    this->sender = source.sender;
}

Message::~Message() {
}

Message& Message::operator=(const Message& source) {
    this->priority = source.priority;
    this->sender = source.sender;
    return *this;
}

int Message::GetPriority() const {
    return priority;
}

MessageHandler* Message::GetSender() const {
    return sender;
}

void Message::SetSender(MessageHandler* sender) {
    this->sender = sender;
}
