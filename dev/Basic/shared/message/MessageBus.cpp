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
using std::string;
using std::vector;

using boost::shared_mutex;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::upgrade_to_unique_lock;

namespace {
    const int MIN_CUSTOM_PRIORITY  = 5;
    const int MSGI_ADD_HANDLER     = 1;
    const int MSGI_REMOVE_HANDLER  = 2;
    const int MSGI_REMOVE_THREAD   = 3;


    /***************************************************************************
     *                              Internal Types
     **************************************************************************/

    /**
     * Represents the content stored for each messages.
     * 
     * Priorities less than MIN_CUSTOM_PRIORITY are for internal messages.
     * 
     * @param destination (not managed) Handler for destination.
     * @param message (managed) shared pointer with message instance. 
     *        Message should be managed by the MessageBus class.
     *        Notice that you should not keep with a reference for this message.
     * @param type identifies the type of the message.
     * @param internal tells if the message is internal or not.
     * @param priority tells the priority of the message. Notice that 
     *        Notice that each message should have a priority => MIN_CUSTOM_PRIORITY,
     *        Otherwise the default priority will be MIN_CUSTOM_PRIORITY.
     */
    struct MessageEntry {
        MessageHandler* destination;
        MessageBus::MessagePtr message;
        Message::MessageType type;
        bool internal;
        int priority;
    };
    typedef boost::shared_ptr<MessageEntry> MessageQueueEntry;
    
    class ComparePriority {
    public:

        bool operator()(MessageQueueEntry& t1, MessageQueueEntry& t2) {
            return (t1->priority < t2->priority);
        }
    };
    
    typedef priority_queue<MessageQueueEntry, vector<MessageQueueEntry>, ComparePriority> MessageQueue;

    /**
     * Represents a thread context.
     * 
     * @param threadId String id the thread identifier.
     * @param main tells the context is associated with the main thread. 
     * @param input queue for messages.
     * @param output queue for messages.
     */
    struct ThreadContext {
        ThreadContext() 
        : input(ComparePriority()), 
          output(ComparePriority()){
        }
        string threadId;
        bool main;
        MessageQueue input;
        MessageQueue output;
    };

    /***************************************************************************
     *                              Helper Functions
     **************************************************************************/

    /**
     * Destroys the content associated with the given 
     * MessageQueueEntry reference.
     * @param entry to destroy.
     */
    void DeleteEntry(MessageQueueEntry& entry) {
        if (entry) {
            if (!(entry->message.unique())) {
                throw runtime_error("Message must be managed only by MessageBus.");
            }
            entry->message.reset();
            if (!entry.unique()) {
                throw runtime_error("MessageQueueEntry must be managed only by MessageBus.");
            }
            entry.reset();
        }
    }

    /**
     * Cleanup the given queue reference.
     * @param queue to clean up.
     */
    void ClearQueue(MessageQueue& queue) {
        MessageQueueEntry entry;
        if (!queue.empty()) {
            entry = queue.top();
            queue.pop();
            DeleteEntry(entry);
        }
    }

    /**
     * Destroys the given thread context.
     * @param context to destroy. 
     */
    void DeleteContext(ThreadContext* context) {
        if (context) {
            ClearQueue(context->input);
            ClearQueue(context->output);
            safe_delete_item(context);
        }
    }

    typedef list<ThreadContext*> ContextList;
    boost::thread_specific_ptr<ThreadContext> threadContext(DeleteContext);
    ContextList threadContexts;

    /***************************************************************************
     *                              Helper Functions
     **************************************************************************/

    /**
     * Checks the if the current context belongs to the main thread.
     * Notice some actions must be processed in the main thread only.
     */
    void CheckMainThread() {
        if (!threadContext.get() && threadContext.get()->main) {
            throw runtime_error("This call should be done using the registered main thread context.");
        }
    }

    /**
     * Checks the thread that called the function has a registered context or not.
     */
    void CheckThreadContext() {
        if (!threadContext.get()) {
            throw runtime_error("This thread does not have any MessageCollector associated.");
        }
    }

    /**
     * Gets the context associated with the current thread.
     * @return ThreadContext pointer or NULL.
     */
    ThreadContext* GetThreadContext() {
        return threadContext.get();
    }

    /***************************************************************************
     *                              Internal Classes
     **************************************************************************/

    /**
     * Represents an internal message
     */
    class InternalMessage : public Message {
    public:

        InternalMessage(ThreadContext* context, MessageHandler* handler)
        : context(context), handler(handler) {
        }

        virtual ~InternalMessage() {
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

    boost::shared_mutex contextsMutex;
}// anonymous namespace

/***************************************************************************
 *                              BUS Implementation
 **************************************************************************/

void MessageBus::RegisterMainThread() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
        context->main = true;
        threadContext.reset(context);
        GetInstance().context = context;
        cout << "Registered Main: " << GetInstance().context << std::endl;
    } else {
        throw runtime_error("Main thread already has a context associated.");
    }
}

void MessageBus::RegisterThread() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
        context->main = false;
        threadContext.reset(context);
        { //we only need this lock to register the thread context.
            upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
            upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
            threadContexts.push_back(context);
            cout << "Registered Thread: " << context << std::endl;
        }
    } else {
        throw runtime_error("Thread already has a context associated.");
    }
}

void MessageBus::RegisterHandler(MessageHandler* handler) {
    CheckThreadContext();
    if (handler) {
        ThreadContext* context = GetThreadContext();
        PostMessage(&GetInstance(), MSGI_ADD_HANDLER,
                MessagePtr(new InternalMessage(context, handler)));
    }
}

void MessageBus::UnRegisterMainThread() {
    CheckMainThread();
    threadContexts.clear();
    threadContext.reset();
    cout << "UnRegistered Main." << std::endl;
}

void MessageBus::UnRegisterThread() {
    CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    PostMessage(&GetInstance(), MSGI_REMOVE_THREAD,
            MessagePtr(new InternalMessage(context, NULL)));
    threadContext.reset();
}

void MessageBus::UnRegisterHandler(MessageHandler* handler) {
    CheckThreadContext();
    if (handler) {
        ThreadContext* context = GetThreadContext();
        PostMessage(&GetInstance(), MSGI_REMOVE_HANDLER,
                MessagePtr(new InternalMessage(context, handler)));
    }
}

void MessageBus::DistributeMessages() {
    CheckMainThread();
    CollectMessages();
    // dispatch internal messages first.
    ThreadDispatchMessages();
    DispatchMessages();
}

void MessageBus::ThreadDispatchMessages() {
    CheckThreadContext();
    //gets main collector;
    ThreadContext* context = GetThreadContext();
    if (context) {
        while (!context->input.empty()) {
            MessageQueueEntry entry = context->input.top();
            context->input.pop();
            if (entry->destination && entry->message.get()) {
                entry->destination->HandleMessage(entry->type, *(entry->message.get()));
            }
            DeleteEntry(entry);
        }
    } else {
        throw runtime_error("This thread does not have a context associated.");
    }
}

void MessageBus::PostMessage(MessageHandler* destination, Message::MessageType type, MessageBus::MessagePtr message) {
    CheckThreadContext();
    if (destination) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            InternalMessage* internalMsg = dynamic_cast<InternalMessage*> (message.get());
            MessageEntry* entry = new MessageEntry();
            entry->destination = destination;
            entry->type = type;
            entry->message = message;
            entry->priority = (!internalMsg && message->GetPriority() < MIN_CUSTOM_PRIORITY) ? MIN_CUSTOM_PRIORITY : message->priority;
            entry->internal = (internalMsg != nullptr);
            context->output.push(MessageQueueEntry(entry));
        } else {
            throw runtime_error("This thread does not have a context associated.");
        }
    }
}

void MessageBus::CollectMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        ContextList::iterator lstItr = threadContexts.begin();
        while (lstItr != threadContexts.end()) {
            ThreadContext* context = (*lstItr);
            while (!context->output.empty()) {
                MessageQueueEntry entry = context->output.top();
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
            MessageQueueEntry entry = mainContext->output.top();
            mainContext->output.pop();
            ThreadContext* destinationContext = reinterpret_cast<ThreadContext*> (entry->destination->context);
            if (destinationContext) {
                //sends the message to the input queue of the destination thread.
                destinationContext->input.push(entry);
            }
        }
    } else {
        throw runtime_error("Main thread does not have a context associated.");
    }
}

MessageBus::MessageBus() : MessageHandler(0) {
}

void MessageBus::HandleMessage(Message::MessageType type, const Message& message) {
    CheckMainThread();
    switch (type) {
        case MSGI_ADD_HANDLER:
        {
            // adds a nee message handler to the system.
            const InternalMessage& msg = static_cast<const InternalMessage&> (message);
            msg.GetHandler()->context = msg.GetContext();
            cout << "Registered Handler: " << msg.GetHandler() << " into " << msg.GetHandler()->context << std::endl;
            break;
        }
        case MSGI_REMOVE_HANDLER:
        {
            // adds a nee message handler to the system.
            const InternalMessage& msg = static_cast<const InternalMessage&> (message);
            msg.GetHandler()->context = NULL;
            cout << "UnRegistered Handler: " << msg.GetHandler() << std::endl;
            break;
        }
        case MSGI_REMOVE_THREAD:
        {
            // adds a nee message handler to the system.
            const InternalMessage& msg = static_cast<const InternalMessage&> (message);
            threadContexts.remove(msg.GetContext());
            cout << "UnRegistered Thread: " << msg.GetContext() << std::endl;
            break;
        }
        default: break;
    }
}

MessageBus& MessageBus::GetInstance() {
    static MessageBus instance;
    return instance;
}