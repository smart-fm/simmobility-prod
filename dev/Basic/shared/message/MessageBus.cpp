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
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
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

const unsigned int MessageBus::MB_MIN_MSG_PRIORITY = 5;
const unsigned int MessageBus::MB_MSG_START = 1000000;
            
namespace {
    const unsigned int MB_MSGI_START = 1000;
    
    enum InternalMessages{
        MSGI_ADD_HANDLER = MB_MSGI_START,
        MSGI_REMOVE_HANDLER,
        MSGI_REMOVE_THREAD
    };

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
    typedef struct MessageEntry {

        MessageEntry() : destination(NULL), internal(false), priority(MessageBus::MB_MIN_MSG_PRIORITY) {
        }
        MessageHandler* destination;
        MessageBus::MessagePtr message;
        Message::MessageType type;
        bool internal;
        int priority;
    } *MessageEntryPtr;

    struct ComparePriority {

        bool operator()(const MessageEntryPtr t1, const MessageEntryPtr t2) const {
            return (t1 && t2 && t1->priority < t2->priority);
        }
    };

    typedef priority_queue<MessageEntryPtr, std::deque<MessageEntryPtr>, ComparePriority> MessageQueue;

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
        output(ComparePriority()),
        main(false),
        totalMessages(0),
        deletedMessages(0) {
        }
        boost::thread::id threadId;
        bool main;
        MessageQueue input;
        MessageQueue output;
        // statistics
        long long totalMessages;
        long long deletedMessages;
    };
    
    typedef list<ThreadContext*> ContextList;
    
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

    /***************************************************************************
     *                              Helper Functions
     **************************************************************************/

    /**
     * Checks the if the current context belongs to the main thread.
     * Notice some actions must be processed in the main thread only.
     */
    void CheckMainThread();

    /**
     * Checks the thread that called the function has a registered context or not.
     */
    void CheckThreadContext();

    /**
     * Gets the context associated with the current thread.
     * @return ThreadContext pointer or NULL.
     */
    ThreadContext* GetThreadContext();

    /**
     * Destroys the content associated with the given 
     * MessageQueueEntry reference.
     * @param entry to destroy.
     */
    void DeleteEntry(MessageEntryPtr& entry);

    /**
     * Cleanup the given queue reference.
     * @param queue to clean up.
     */
    void CleanUpQueue(MessageQueue& queue);

    /**
     * Destroys the given thread context.
     * @param context to destroy. 
     */
    void DeleteContext(ThreadContext* context);

    /***************************************************************************
     *                              Global variables
     **************************************************************************/

    boost::thread_specific_ptr<ThreadContext> threadContext(DeleteContext);
    ContextList threadContexts;
    boost::shared_mutex contextsMutex;
}// anonymous namespace

/***************************************************************************
 *                              BUS Implementation
 **************************************************************************/

void MessageBus::RegisterMainThread() {
    if (!threadContext.get()) {
        ThreadContext* mainContext = new ThreadContext();
        mainContext->threadId = boost::this_thread::get_id();
        mainContext->main = true;
        GetInstance().context = static_cast<void*> (mainContext);
        threadContext.reset(mainContext);
        threadContexts.push_back(mainContext);
        cout << "Registered Main: " << mainContext->threadId << std::endl;
    } else {
        throw runtime_error("MessageBus - Main thread already has a context associated.");
    }
}

void MessageBus::UnRegisterMainThread() {
    CheckMainThread();
    GetInstance().context = NULL;
    threadContexts.clear();
    threadContext.reset();
    cout << "UnRegistered Main." << std::endl;
}

void MessageBus::RegisterThread() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->threadId = boost::this_thread::get_id();
        context->main = false;
        {// thread-safe scope
            upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
            upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
            threadContexts.push_back(context);
        }
        threadContext.reset(context);
    } else {
        throw runtime_error("MessageBus - Current thread already has a context associated.");
    }
}

void MessageBus::UnRegisterThread() {
    CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        threadContexts.remove(context);
    }
    threadContext.reset();
}

void MessageBus::RegisterHandler(MessageHandler* handler) {
    CheckThreadContext();
    if (handler) {
        ThreadContext* context = GetThreadContext();
        if (!(handler->context)) {
            handler->context = static_cast<void*> (context);            
        } else if (context != handler->context) {
            // just assign the context.
            throw runtime_error("MessageBus - To register the handler in other thread context it is necessary to unregister it first.");
        }
    }
}

void MessageBus::UnRegisterHandler(MessageHandler* handler) {
    CheckThreadContext();
    if (handler && handler->context) {
        ThreadContext* context = GetThreadContext();
        if (context == handler->context) {
            handler->context = NULL;
        } else {
            throw runtime_error("MessageBus - To unregister the handler it is necessary to use the registered thread context.");
        }
    }
}

void MessageBus::DistributeMessages() {
    CheckMainThread();
    ThreadContext* context = GetThreadContext();
    //cout << "1 - Input: " << context->input.size() << " Output: " << context->output.size() << std::endl;
    CollectMessages();
    //cout << "2 - Input: " << context->input.size() << " Output: " << context->output.size() << std::endl;
    // dispatch internal messages first.
    ThreadDispatchMessages();
    //cout << "3 - Input: " << context->input.size() << " Output: " << context->output.size() << std::endl;
    DispatchMessages();
    //cout << "4 - Input: " << context->input.size() << " Output: " << context->output.size() << std::endl;
}

void MessageBus::CollectMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        ContextList::iterator lstItr = threadContexts.begin();
        while (lstItr != threadContexts.end()) {
            ThreadContext* context = (*lstItr);
            while (!context->output.empty()) {
                MessageEntryPtr entry = context->output.top();
                // internal messages go to the input queue of the main context.
                if (entry->internal || context == mainContext) {
                    mainContext->input.push(entry);
                } else {
                    mainContext->output.push(entry);
                }
                context->output.pop();
            }
            lstItr++;
        }
    }
}

void MessageBus::ThreadDispatchMessages() {
    CheckThreadContext();
    //gets main collector;
    ThreadContext* context = GetThreadContext();
    if (context) {
        while (!context->input.empty()) {
            MessageEntryPtr entry = context->input.top();
            if (entry && entry->destination && entry->message.get()) {
                ThreadContext* destinationContext = static_cast<ThreadContext*> (entry->destination->context);
                if (context->threadId != destinationContext->threadId) {
                    throw runtime_error("Thread contexts inconsistency.");
                }
                entry->destination->HandleMessage(entry->type, *(entry->message.get()));
            }
            context->input.pop();
            DeleteEntry(entry);
            context->deletedMessages++;
        }
    }
}

void MessageBus::DispatchMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        while (!mainContext->output.empty()) {
            MessageEntryPtr entry = mainContext->output.top();
            ThreadContext* destinationContext = static_cast<ThreadContext*> (entry->destination->context);
            if (destinationContext) {
                //sends the message to the input queue of the destination thread.
                destinationContext->input.push(entry);
            }
            mainContext->output.pop();
        }
    }
}

void MessageBus::PostMessage(MessageHandler* destination, Message::MessageType type, MessageBus::MessagePtr message) {
    CheckThreadContext();
    if (destination) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            InternalMessage* internalMsg = dynamic_cast<InternalMessage*> (message.get());
            MessageEntryPtr entry = new MessageEntry();
            entry->destination = destination;
            entry->type = type;
            entry->message = message;
            entry->priority = (!internalMsg && message->GetPriority() < MB_MIN_MSG_PRIORITY) ? MB_MIN_MSG_PRIORITY : message->priority;
            entry->internal = (internalMsg != nullptr);
            context->output.push(entry);
            context->totalMessages++;
        }
    }
}

MessageBus::MessageBus() : MessageHandler(0) {
}

void MessageBus::HandleMessage(Message::MessageType type, const Message& message) {
    CheckMainThread();
    // FUTURE 
    /*switch (type) {
        default: break;
    }*/
}

MessageBus& MessageBus::GetInstance() {
    static MessageBus instance;
    return instance;
}

/***************************************************************************
 *                          Helper Functions Impl
 **************************************************************************/
namespace {

    void CheckMainThread() {
        if (!threadContext.get() && threadContext.get()->main) {
            throw runtime_error("MessageBus - This call must be done using the registered main thread context.");
        }
    }

    void CheckThreadContext() {
        boost::thread::id threadId = boost::this_thread::get_id();
        if (!threadContext.get() && threadId == threadContext.get()->threadId) {
            throw runtime_error("MessageBus - This call must be done using a registered thread context.");
        }
    }

    ThreadContext* GetThreadContext() {
        return threadContext.get();
    }

    void DeleteEntry(MessageEntryPtr& entry) {
        if (entry) {
            if (!(entry->message.unique())) {
                throw runtime_error("MessageBus - Message must be managed only by the MessageBus.");
            }
            entry->message.reset();
            safe_delete_item(entry);
        }
    }

    void CleanUpQueue(MessageQueue& queue) {
        MessageEntryPtr entry = NULL;
        while (!queue.empty()) {
            entry = queue.top();
            DeleteEntry(entry);
            queue.pop();
        }
    }

    void DeleteContext(ThreadContext* context) {
        if (context) {
            CleanUpQueue(context->input);
            CleanUpQueue(context->output);
            /*cout << "Thread: " << context->threadId
                    << " Received: " << context->totalMessages
                    << " Deleted: " << context->deletedMessages
                    << std::endl;*/
            safe_delete_item(context);
        }
    }
}