//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   MessageBus.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Aug 15, l2013, 9:30 PM
 */
#include "MessageBus.hpp"
#include "event/EventPublisher.hpp"
#include "util/LangHelpers.hpp"
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <queue>
#include <list>
#include <iostream>
#include "logging/Log.hpp"

using namespace sim_mob::messaging;
using namespace sim_mob::event;
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
    const unsigned int INTERNAL_EVENT_MSG_PRIORITY = 3;
    const unsigned int INTERNAL_EVENT_ACTION_PRIORITY = 4;
    
    const std::string REPORT_LINE = "# Id: %-25s Received: %-12s Processed: %-12s Events: %-12s Remaining: %-12s";

    enum InternalMessages {
        MSGI_ADD_HANDLER = MB_MSGI_START,
        MSGI_REMOVE_HANDLER,
        MSGI_REMOVE_THREAD,
        MSGI_EVENT_MSG,
        //events
        MSGI_PUBLISH_EVENT,
        MSGI_UNSUBSCRIBE_ALL,
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
     * @param processOnMainThread tells to process the message 
     *        within the main thread context.
     */
    typedef struct MessageEntry {

        MessageEntry()
        : destination(nullptr), internal(false), event(false),
        priority(MessageBus::MB_MIN_MSG_PRIORITY), processOnMainThread(false){
        }

        MessageEntry(const MessageEntry& source) {
            this->destination = source.destination;
            this->message = source.message;
            this->type = source.type;
            this->internal = source.internal;
            this->priority = source.priority;
            this->event = source.event;
            this->processOnMainThread = source.processOnMainThread;
        }

        MessageHandler* destination;
        MessageBus::MessagePtr message;
        Message::MessageType type;
        bool internal;
        int priority;
        bool event;
        bool processOnMainThread;
    } *MessageEntryPtr;

    struct ComparePriority {

        bool operator()(const MessageEntry& t1, const MessageEntry& t2) const {
            return (t1.priority > t2.priority);
        }
    };

    typedef priority_queue<MessageEntry, std::deque<MessageEntry>, ComparePriority> MessageQueue;

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
        : eventPublisher(nullptr),
        input(ComparePriority()),
        output(ComparePriority()),
        main(false),
        receivedMessages(0),
        processedMessages(0),
        eventMessages(0) {
        }

        virtual ~ThreadContext() {
            CleanUpQueue(input);
            CleanUpQueue(output);
            safe_delete_item(eventPublisher);
        }

        /**
         * Cleanup the given queue reference.
         * @param queue to clean up.
         */
        void CleanUpQueue(MessageQueue& queue) {
            while (!queue.empty()) {
                queue.pop();
            }
        }

        boost::thread::id threadId;
        bool main;
        MessageQueue input;
        MessageQueue output;
        //event publisher for each thread context.
        EventPublisher* eventPublisher;
        // statistics
        unsigned long long int receivedMessages;
        unsigned long long int processedMessages;
        unsigned long long int eventMessages;
        
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
            this->priority = 1;
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

    /**
     * Represents an internal message
     */
    class InternalEventMessage : public Message {
    public:

        InternalEventMessage(EventId id, Context ctx, MessageBus::EventArgsPtr data)
        : id(id), ctx(ctx), data(data) {
            this->priority = INTERNAL_EVENT_MSG_PRIORITY;
        }
        InternalEventMessage(EventId id, Context ctx)
        : id(id), ctx(ctx){
            this->priority = INTERNAL_EVENT_MSG_PRIORITY;
        }

        InternalEventMessage(const InternalEventMessage& source) : Message(source) {
            this->ctx = source.ctx;
            this->data = source.data;
            this->id = source.id;
        }

        virtual ~InternalEventMessage() {
        }

        EventId GetEventId() const {
            return id;
        }

        Context GetContext() const {
            return ctx;
        }

        MessageBus::EventArgsPtr GetData() const {
            return data;
        }

    private:
        EventId id;
        Context ctx;
        MessageBus::EventArgsPtr data;
    };

    /**
     *
     * Internal EventPublisher.
     *
     */
    class InternalEventPublisher : public EventPublisher, public MessageHandler {
    public:

        InternalEventPublisher() : EventPublisher(), MessageHandler(-1) {
        }

        virtual ~InternalEventPublisher() {
        }
    private:

        void HandleMessage(Message::MessageType type, const Message& message) {
            const InternalEventMessage& evt =
                    static_cast<const InternalEventMessage&> (message);
            bool isGlobalEvent = (evt.GetContext() == nullptr);
            switch (type) {
                case MSGI_PUBLISH_EVENT:
                {
                    if (isGlobalEvent) {
                        publish(evt.GetEventId(),
                                *(evt.GetData().get()));
                    } else {
                        publish(evt.GetEventId(),
                                evt.GetContext(), *(evt.GetData().get()));
                    }
                    break;
                }
                case MSGI_UNSUBSCRIBE_ALL:
                {
                    if (isGlobalEvent) {
                        unSubscribeAll(evt.GetEventId());
                    } else {
                        unSubscribeAll(evt.GetEventId(),evt.GetContext());
                    }
                    break;
                }
                default: break;
            }
        }
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
     * @return ThreadContext pointer or nullptr.
     */
    ThreadContext* GetThreadContext();
    
    void deleteContext(ThreadContext* ctx){}
    /**
     * Deletes all contexts in the system
     */
    void deleteAllContexts();
    
    /**
     * Prints the report.
     */
    void printReport();

    /***************************************************************************
     *                              Global variables
     **************************************************************************/

    boost::thread_specific_ptr<ThreadContext> threadContext (deleteContext);
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
        mainContext->eventPublisher = new InternalEventPublisher();
        mainContext->main = true;
        GetInstance().context = static_cast<void*> (mainContext);
        threadContext.reset(mainContext);
        threadContexts.push_back(mainContext);
        RegisterHandler(dynamic_cast<MessageHandler*> (mainContext->eventPublisher));
    } else {
        throw runtime_error("MessageBus - Main thread already has a context associated.");
    }
}

void MessageBus::UnRegisterMainThread() {
    CheckMainThread();
    printReport();
    GetInstance().context = nullptr;
    deleteAllContexts();
}

void MessageBus::RegisterThread() {
    if (!threadContext.get()) {
        ThreadContext* context = new ThreadContext();
        context->threadId = boost::this_thread::get_id();
        context->eventPublisher = new InternalEventPublisher();
        context->main = false;
        {// thread-safe scope
            upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
            upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
            threadContexts.push_back(context);
        }
        threadContext.reset(context);
        RegisterHandler(dynamic_cast<MessageHandler*> (context->eventPublisher));
    } else {
        throw runtime_error("MessageBus - Current thread already has a context associated.");
    }
}

void MessageBus::UnRegisterThread() {
    // Unsafe operation for now.
    // Context will remain until the end of the simulation.
   /* CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    {// thread-safe scope
        upgrade_lock<shared_mutex> upgradeLock(contextsMutex);
        upgrade_to_unique_lock<shared_mutex> lock(upgradeLock);
        threadContexts.remove(context);
    }
    threadContext.reset();*/
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
            handler->context = nullptr;
        } else {
            throw runtime_error("MessageBus - To unregister the handler it is necessary to use the registered thread context.");
        }
    }
}

void MessageBus::DistributeMessages() {
    CheckMainThread();
    DispatchMessages();
    // dispatch internal/main messages first.
    ThreadDispatchMessages();
}

void MessageBus::DispatchMessages() {
    CheckMainThread();
    ThreadContext* mainContext = GetThreadContext();
    if (mainContext) {
        ContextList::iterator lstItr = threadContexts.begin();
        while (lstItr != threadContexts.end()) {
            ThreadContext* context = (*lstItr);
            while (!context->output.empty()) {
                const MessageEntry& entry = context->output.top();
                if (entry.event) {
                    context->eventMessages++;
                    //if it is an event then we need to distribute the event for all
                    //publishers in the system.
                    ContextList::iterator lstItr1 = threadContexts.begin();
                    while (lstItr1 != threadContexts.end()) {
                        ThreadContext* ctx = (*lstItr1);
                        //main context will receive the original message 
                        //for other the message entry is cloned.
                        MessageEntry newEntry(entry);
                        newEntry.destination = dynamic_cast<MessageHandler*> (ctx->eventPublisher);
                        ctx->input.push(newEntry);
                        lstItr1++;
                    }
                } else {// it is a regular/single message
                    context->receivedMessages++;
                    if (entry.processOnMainThread) {
                        mainContext->input.push(entry);
                    } else {
                        ThreadContext* destinationContext = static_cast<ThreadContext*> (entry.destination->context);
                        if (destinationContext) {
                            destinationContext->input.push(entry);
                        }
                    }
                }
                // internal messages go to the input queue of the main context.
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
            const MessageEntry& entry = context->input.top();
            if (entry.destination && entry.message.get()) {
                ThreadContext* destinationContext = static_cast<ThreadContext*> (entry.destination->context);
                if (!entry.processOnMainThread && context->threadId != destinationContext->threadId) {
                    throw runtime_error("Thread contexts inconsistency.");
                }
                entry.destination->HandleMessage(entry.type, *(entry.message.get()));
            }
            context->input.pop();
            context->processedMessages++;
        }
    }
}

void MessageBus::PostMessage(MessageHandler* destination, Message::MessageType type, 
                             MessageBus::MessagePtr message, bool processOnMainThread) {
    CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    if (context) {
        InternalMessage* internalMsg = dynamic_cast<InternalMessage*> (message.get());
        InternalEventMessage* eventMsg = dynamic_cast<InternalEventMessage*> (message.get());
        if (destination || eventMsg) {
            MessageEntry entry;
            entry.destination = destination;
            entry.type = type;
            entry.message = message;
            entry.priority = (!internalMsg && !eventMsg && message->GetPriority() < MB_MIN_MSG_PRIORITY) ? MB_MIN_MSG_PRIORITY : message->priority;
            entry.internal = (internalMsg != nullptr);
            entry.event = (eventMsg != nullptr);
            entry.processOnMainThread = processOnMainThread;
            context->output.push(entry);
        }
    }
}

void MessageBus::SendContextualMessage(MessageHandler* destination,
		Message::MessageType type, MessagePtr message) {
	CheckThreadContext();
	ThreadContext* context = GetThreadContext();
	if (context) {
		if (destination && destination->context == context) {
			destination->HandleMessage(type, *(message.get()));
			context->receivedMessages++;
			context->processedMessages++;
		}
		else {
			throw std::runtime_error("SendContextualMessage() is called for sending messages outside thread context");
		}
	}
}

void MessageBus::SubscribeEvent(EventId id, EventListener* listener) {
    CheckThreadContext();
    if (listener) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            context->eventPublisher->registerEvent(id);
            context->eventPublisher->subscribe(id, listener);
        }
    }
}

void MessageBus::SubscribeEvent(EventId id, Context ctx, EventListener* listener) {
    CheckThreadContext();
    if (listener && ctx) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            context->eventPublisher->registerEvent(id);
            context->eventPublisher->subscribe(id, listener, ctx);
        }
    }
}

void MessageBus::UnSubscribeEvent(EventId id, EventListener* listener) {
    CheckThreadContext();
    if (listener) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            context->eventPublisher->unSubscribe(id, listener);
        }
    }
}

void MessageBus::UnSubscribeEvent(EventId id, Context ctx, EventListener* listener) {
    CheckThreadContext();
    if (listener && ctx) {
        ThreadContext* context = GetThreadContext();
        if (context) {
            context->eventPublisher->unSubscribe(id, ctx, listener);
        }
    }
}

void MessageBus::UnSubscribeAll(event::EventId id){
    UnSubscribeAll(id, nullptr);
}

void MessageBus::UnSubscribeAll(event::EventId id, event::Context ctx){
    CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    if (context) {
        InternalEventMessage*  msg = new InternalEventMessage(id, ctx);
        msg->priority = INTERNAL_EVENT_ACTION_PRIORITY;
        PostMessage(nullptr, MSGI_UNSUBSCRIBE_ALL, MessagePtr(msg));
    }
}

void MessageBus::PublishEvent(EventId id, EventArgsPtr args) {
    PublishEvent(id, nullptr, args);
}

void MessageBus::PublishEvent(event::EventId id, event::Context ctx, EventArgsPtr args) {
    CheckThreadContext();
    ThreadContext* context = GetThreadContext();
    if (context) {
        PostMessage(nullptr, MSGI_PUBLISH_EVENT, MessagePtr(new InternalEventMessage(id, ctx, args)));
    }
}

void sim_mob::messaging::MessageBus::PublishInstantaneousEvent(event::EventId id,
		event::Context ctx, EventArgsPtr args) {
	CheckThreadContext();
	ThreadContext* context = GetThreadContext();
	if (context && ctx == context) {
		InternalEventMessage eventMsg(id, ctx, args);
		MessageHandler* target = dynamic_cast<MessageHandler*> (context->eventPublisher);
		target->HandleMessage(MSGI_PUBLISH_EVENT, eventMsg);
		context->eventMessages++;
		context->processedMessages++;
	}
	else {
		throw std::runtime_error("PublishInstantaneousEvent() is called for publishing events outside thread context");
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
        if (!threadContext.get() || !threadContext.get()->main) {
            throw runtime_error("MessageBus - This call must be done using the registered main thread context.");
        }
    }

    void CheckThreadContext() {
        boost::thread::id threadId = boost::this_thread::get_id();
        if (!threadContext.get() || threadId != threadContext.get()->threadId) {
            throw runtime_error("MessageBus - This call must be done using a registered thread context.");
        }
    }

    ThreadContext* GetThreadContext() {
        return threadContext.get();
    }
    
    void deleteAllContexts() {
        ContextList::iterator itr = threadContexts.begin();
        while (itr != threadContexts.end()) {
            ThreadContext* ctx = (*itr);
            safe_delete_item(ctx);
            itr++;
        }
        threadContexts.clear();
    }

    void printReport() {
        PrintOut(endl);
        PrintOut("###################      MESSAGE BUS      ####################" << endl);
        //Simulation Statistics
        PrintOut(endl);
        unsigned long long int totalReceived = 0;
        unsigned long long int totalProcessed = 0;
        unsigned long long int totalEvents = 0;
        unsigned long long int totalRemaining = 0;
        ContextList::iterator itr = threadContexts.begin();
        while (itr != threadContexts.end()) {
            ThreadContext* ctx = (*itr);
            if (ctx) {
                long long int remaining = (ctx->input.size() + ctx->output.size());
                boost::format fmtr = boost::format(REPORT_LINE);
                fmtr % ctx->threadId %
                        ctx->receivedMessages %
                        ctx->processedMessages %
                        ctx->eventMessages %
                        (remaining);
                if (ctx->main){
                    PrintOut(fmtr.str() << " (main thread)" << endl);
                }else{
                    PrintOut(fmtr.str() << endl);
                }
                
                totalReceived += ctx->receivedMessages;
                totalProcessed += ctx->processedMessages;
                totalEvents += ctx->eventMessages;
                totalRemaining += remaining;
            }
            itr++;
        }
        PrintOut(endl);
        boost::format fmtr = boost::format(REPORT_LINE);
        fmtr % "TOTAL" %
                totalReceived %
                totalProcessed %
                totalEvents %
                totalRemaining;
        PrintOut(fmtr.str() << endl);
        PrintOut(endl);
        size_t numThreads = threadContexts.size();
        long long int balance = abs(totalProcessed - totalReceived - (totalEvents * numThreads)) - totalRemaining;
        PrintOut("Balance (Should be 0):  " << balance << std::endl); 
        PrintOut(endl);
        PrintOut("##############################################################" << endl);
        PrintOut(endl);
    }
}
