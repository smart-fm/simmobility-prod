/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageReceiver.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on April 6, 2013, 3:30 PM
 */
#include "MessageReceiver.hpp"
#include "util/LangHelpers.hpp"

using namespace sim_mob;

MessageReceiver::MessageReceiver() {
}

MessageReceiver::~MessageReceiver() {
    while (ContainsMessages()) {
        MessageEntry* entry = messages.front();
        messages.pop();
        DeleteEntry(entry);
    }
}

bool MessageReceiver::ReadMessage() {
    MessageEntry* entry = NULL;
    {
    	boost::upgrade_lock<boost::shared_mutex> up_lock(queueMutex);
    	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
        if (ContainsMessages()) {
            entry = messages.front();
            messages.pop();
        }
    }
    if (entry) {
        HandleMessage(entry->first, *(entry->second->first),
                *(entry->second->second));
        DeleteEntry(entry);
        return true;
    }
    return false;
}

void MessageReceiver::Post(MessageType type, MessageReceiver* sender,
        Message* message) {
	boost::upgrade_lock<boost::shared_mutex> up_lock(queueMutex);
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
    SendMessage(type, sender, message, true);
}

bool MessageReceiver::Send(MessageType type, MessageReceiver& sender, const Message& message) {
    SendMessage(type, &sender, const_cast<Message*> (&message), false);
    return true;
}

bool MessageReceiver::HasMessages() {
	boost::shared_lock<boost::shared_mutex> lock(queueMutex);
    return ContainsMessages();
}

bool MessageReceiver::SendMessage(MessageType type, MessageReceiver* sender, Message* message, bool async) {
    if (sender && message) {
        if (async) {
            messages.push(new MessageEntry(type, new MessageData(sender, message)));
        } else {
            HandleMessage(type, *sender, *message);
        }
    }
    return true;
}

bool MessageReceiver::ContainsMessages(){
    
        return !messages.empty();
}

void MessageReceiver::DeleteEntry(MessageEntry* entry) {
    if (entry) {
        safe_delete_item((entry->second)->second);
        safe_delete_item(entry->second);
        safe_delete_item(entry);
    }
}
