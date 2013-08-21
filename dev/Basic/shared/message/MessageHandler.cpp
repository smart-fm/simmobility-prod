/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageBus.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Aug 15, l2013, 9:30 PM
 */

#include "MessageHandler.hpp"

using namespace sim_mob::messaging;

MessageHandler::MessageHandler(int id) : id(id) {
}

MessageHandler::~MessageHandler() {
}

int MessageHandler::GetId() {
    return id;
}