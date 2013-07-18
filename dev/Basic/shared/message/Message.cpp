/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Message.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 8, 2013, 6:16 PM
 */

#include "Message.hpp"

using namespace sim_mob::messaging;

Message::Message() {
}

Message::Message(const Message& source) {
}

Message::~Message() {
}

Message& Message::operator=(const Message& source) {
    return *this;
}