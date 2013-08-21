/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   MessageBus.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Aug 15, l2013, 9:30 PM
 */
#include "MessageBus.hpp"
#include "util/LangHelpers.hpp"
#include "entities/Entity.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <queue>
#include <list>
#include <iostream>

using namespace sim_mob::messaging;
using std::priority_queue;
using std::runtime_error;
using std::list;
using std::pair;
using boost::unordered_map;
using std::endl;
using std::cout;

using boost::shared_mutex;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::upgrade_to_unique_lock;

namespace {
    const int MIN_CUSTOM_PRIORITY = 5;
    const int MSGI_ADD_HANDLER = 1;
    boost::shared_mutex contextsMutex;

    struct MessageEntry {
        bool internal;
        int priority;
        MessageHandler* destination;
        Message* message;
        Message::MessageType type;
    };

    typedef std::queue<MessageEntry*> MessageQueue;

    struct ThreadContext {
        std::string id;
        bool main;
        MessageQueue input;
        MessageQueue output;
    };

    void DeleteEntry(MessageEntry*& entry) {
        if (entry) {
            Message* msg = entry->message;
            safe_delete_item(msg);
            safe_delete_item(entry);
        }
    }

    void ClearQueue(MessageQueue& queue) {
        MessageEntry* entry = nullptr;
        if (!queue.empty()) {
            entry = queue.front();
            queue.pop();
            DeleteEntry(entry);
            entry = nullptr;
        }
    }

    void DeleteContext(ThreadContext* entry) {
        if (entry) {
            ClearQueue(entry->input);
            ClearQueue(entry->output);
            safe_delete_item(entry);
        }
    }

    typedef list<ThreadContext*> ContextList;
    typedef unordered_map<MessageHandler*, ThreadContext*> HandlersMap;
    typedef pair<MessageHandler*, ThreadContext*> HandlersMapEntry;
    boost::thread_specific_ptr<ThreadContext> threadContext(DeleteContext);

    ContextList threadContexts;
    HandlersMap globalHandlerMap;

    /*********************************
     * Internal messages
     *********************************/
    class InternalMessage : public Message {
    public:

        InternalMessage() {
            this->priority = 0;
        }

        virtual ~InternalMessage() {
        }
    };

    class AddHandlerMessage : public InternalMessage {
    public:

        AddHandlerMessage(ThreadContext* context, MessageHandler* handler)
        : context(context), handler(handler) {
        }

        virtual ~AddHandlerMessage() {
        }

        ThreadContext* GetContext() const {
            return context;
        }

        MessageHandler* GetHandler() const {
            return handler;
        }
    private:
        ThreadContext* context;
        MessageHandler* handler;
    };

    /*********************************
     * Internal main handler
     *********************************/
    class MainHandler : public MessageHandler {
    public:

        MainHandler()
        : MessageHandler(0) {
        }

        virtual void HandleMessage(Message::MessageType type, const Message& message) {
            switch (type) {
                case MSGI_ADD_HANDLER:
                {
                    const AddHandlerMessage& msg = static_cast<const AddHandlerMessage&> (message);
                    globalHandlerMap.insert(HandlersMapEntry(msg.GetHandler(), msg.GetContext()));
                    break;
                }
                default: break;
            }
        }
    };

    MainHandler mainHandler;

    void CheckMainThread() {
        if (!threadContext.get() && threadContext.get()->main) {
            throw runtime_error("This call should be done using the registered main thread context.");
        }
    }

    void CheckThreadContext() {
        if (!threadContext.get()) {
            throw runtime_error("This thread does not have any MessageCollector associated.");
        }
    }

    bool IsHandlerRegistered(MessageHandler* handler) {
        HandlersMap::iterator mapItr = globalHandlerMap.find(handler);
        return (mapItr != globalHandlerMap.end());
    }

    ThreadContext* GetThreadContext() {
        return threadContext.get();
    }

    ThreadContext* GetHandlerThreadContext(MessageHandler* handler) {
        ThreadContext* context = NULL;
        if (handler) {
            HandlersMap::iterator mapItr = globalHandlerMap.find(handler);
            if (mapItr != globalHandlerMap.end()) {
                context = mapItr->second;
            }
        }
        return context;
    }
}

void MessageBus::RegisterMainCollector() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->id = boost::lexical_cast<std::string>(boost::this_thread::get_id());
        context->main = true;
        threadContext.reset(context);
        globalHandlerMap.insert(HandlersMapEntry(&mainHandler, context));
    }
}

void MessageBus::RegisterThreadMessageQueue() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->id = boost::lexical_cast<std::string>(boost::this_thread::get_id());
        context->main = false;
        threadContext.reset(context);
        { //we only need this lock to register the thread context.
            upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
            upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
            threadContexts.push_back(context);
        }
    }
}

void MessageBus::RegisterHandler(MessageHandler* destination) {
    CheckThreadContext();
    if (destination) {
        ThreadContext* context = GetThreadContext();
        PostMessage(&mainHandler, MSGI_ADD_HANDLER,
                new AddHandlerMessage(context, destination));
    }
}

void MessageBus::DistributeMessages() {
    CheckMainThread();
    CollectMessages();
    // dispatch internal messages first.
    ThreadDispatchMessages();
    DispatchMessages();
}

void MessageBus::CollectMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        ContextList::iterator lstItr = threadContexts.begin();
        while (lstItr != threadContexts.end()) {
            ThreadContext* context = (*lstItr);
            while (!context->output.empty()) {
                MessageEntry* entry = context->output.front();
                context->output.pop();
                // internal messages go to the input queue of the main context.
                if (entry->internal) {
                    mainContext->input.push(entry);
                } else {
                    mainContext->output.push(entry);
                }
            }
            lstItr++;
        }
    } else {
        throw runtime_error("Main thread does not have a context associated.");
    }
}

void MessageBus::DispatchMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        while (!mainContext->output.empty()) {
            MessageEntry* entry = mainContext->output.front();
            mainContext->output.pop();
            if (entry) {
                ThreadContext* destinationContext = GetHandlerThreadContext(entry->destination);
                if (destinationContext) {
                    //sends the message to the input queue of the destination thread.
                    destinationContext->input.push(entry);
                }
            }
        }
    } else {
        throw runtime_error("Main thread does not have a context associated.");
    }
}

void MessageBus::ThreadDispatchMessages() {
    CheckThreadContext();
    //gets main collector;
    ThreadContext* context = GetThreadContext();
    if (context) {
        while (!context->input.empty()) {
            MessageEntry* entry = context->input.front();
            context->input.pop();
            if (entry && entry->destination && entry->message) {
                entry->destination->HandleMessage(entry->type, *(entry->message));
            }
            DeleteEntry(entry);
        }
    } else {
        throw runtime_error("This thread does not have a context associated.");
    }
}

void MessageBus::PostMessage(MessageHandler* destination, Message::MessageType type, Message* message) {
    CheckThreadContext();
    if (destination && message) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            InternalMessage* internalMsg = dynamic_cast<InternalMessage*> (message);
            MessageEntry* entry = new MessageEntry();
            entry->destination = destination;
            entry->type = type;
            entry->message = message;
            entry->priority = (!internalMsg && message->GetPriority() < MIN_CUSTOM_PRIORITY) ? MIN_CUSTOM_PRIORITY : message->priority;
            entry->internal = (internalMsg != nullptr);
            context->output.push(entry);
        } else {
            throw runtime_error("This thread does not have a context associated.");
        }
    }
}
