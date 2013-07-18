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

sim_mob::messaging::Message::Message() {
}

sim_mob::messaging::Message::Message(const sim_mob::messaging::Message& source) {
}

sim_mob::messaging::Message::~Message() {
}

sim_mob::messaging::Message& sim_mob::messaging::Message::operator=(const sim_mob::messaging::Message& source) {
    return *this;
}
