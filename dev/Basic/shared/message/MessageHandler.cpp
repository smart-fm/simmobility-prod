//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   MessageBus.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Aug 15, 2013, 9:30 PM
 */

#include "util/LangHelpers.hpp"
#include "MessageHandler.hpp"

using namespace sim_mob::messaging;
std::set<unsigned int> MessageHandler::used = std::set<unsigned int>();
unsigned int MessageHandler::freeCounter = 1;
MessageHandler::MessageHandler(unsigned int id) : id(id), context(NULL) {
	add(id);
}

MessageHandler::MessageHandler() : id(getNewId()), context(NULL) {
	add(id);
}

MessageHandler::~MessageHandler() {
    context = NULL;
}

unsigned int MessageHandler::GetId() const {
    return id;
}


void MessageHandler::add(unsigned int i)
{
    used.insert(i);
}

unsigned int MessageHandler::getNewId()
{
	std::set<unsigned int>::iterator iter = used.lower_bound(freeCounter);
    while (iter != used.end() && *iter == freeCounter)
    {
        ++iter;
        ++freeCounter;
    }
    return freeCounter++;
}
