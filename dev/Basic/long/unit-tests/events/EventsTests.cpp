//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   EventsTests.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 29, 2013, 5:22 PM
 */

#include "EventsTests.hpp"
#include <cstdio>
#include "event/EventPublisher.hpp"
#include <assert.h>

using namespace sim_mob::event;
using namespace unit_tests;
using std::cout;
using std::endl;

namespace {
    const int TEST_SIZE = 100000;

    DECLARE_CUSTOM_CALLBACK_TYPE(TestNonEventArgs)
    class TestNonEventArgs {
    };

    DECLARE_CUSTOM_CALLBACK_TYPE(TestEventArgs)

    class TestEventArgs : public EventArgs {
    };

    class TestPublisher : public EventPublisher {
    };

    class TestListener : public EventListener {
    public:

        TestListener() : receivedEvents(0), receivedCallbackEvents(0),
        receivedCallbackNonEvents(0) {
        }

        virtual void onEvent(sim_mob::event::EventId id,
                sim_mob::event::Context ctxId,
                sim_mob::event::EventPublisher* sender,
                const EventArgs& args) {
            receivedEvents++;
            receivedCallbackNonEvents++;
        }

        void onTest(sim_mob::event::EventId id,
                sim_mob::event::Context ctxId,
                sim_mob::event::EventPublisher* sender,
                const TestEventArgs& args) {
            receivedEvents++;
            receivedCallbackEvents++;
        }

        int receivedEvents;
        int receivedCallbackEvents;
        int receivedCallbackNonEvents;
    };

}

/**
 * Tests unregister
 */
void testUnregister() {
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.subscribe(1, &listener1);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.unRegisterEvent(1);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
    assert(listener1.receivedCallbackEvents == 0);
    assert(listener1.receivedCallbackNonEvents == 2);
    assert(listener1.receivedEvents == 2);
}

/**
 * Tests normal
 */
void testEvent() {
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.registerEvent(2);
    publisher.subscribe(1, &listener);
    publisher.subscribe(2, &listener);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs());
        publisher.publish(2, TestEventArgs());
    }
    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == TEST_SIZE * 2);
    assert(listener.receivedEvents == TEST_SIZE * 2);
}

/**
 * Tests callback
 * 
 */
void testCallback() {
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &TestListener::onTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs());
    }
    assert(listener.receivedCallbackEvents == TEST_SIZE);
    assert(listener.receivedCallbackNonEvents == 0);
    assert(listener.receivedEvents == TEST_SIZE);
}

/**
 * Tests context callback
 */
void testContextCallback() {
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &TestListener::onTest, &listener);
    publisher.subscribe(2, &listener, &TestListener::onTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs());
        publisher.publish(1, &listener, TestEventArgs());
    }
    assert(listener.receivedCallbackEvents == TEST_SIZE);
    assert(listener.receivedCallbackNonEvents == 0);
    assert(listener.receivedEvents == TEST_SIZE);
}

/**
 * Tests Unsubscribe event
 */
void testUnsubscribeEvent() {
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.unSubscribe(1, &listener);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
}

/**
 * Tests Unsubscribe event
 */
void testUnsubscribeContextEvent() {
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &listener);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());
    publisher.unSubscribe(1, &listener);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());
    publisher.unSubscribe(1, &listener, &listener);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener, TestEventArgs());

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 4);
    assert(listener.receivedEvents == 4);
}

/**
 * Tests Unsubscribe All listeners
 */
void testUnsubscribeAll() {
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.subscribe(1, &listener1);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());
    publisher.unSubscribeAll(1);
    publisher.publish(1, TestEventArgs());
    publisher.publish(1, TestEventArgs());

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
    assert(listener1.receivedCallbackEvents == 0);
    assert(listener1.receivedCallbackNonEvents == 2);
    assert(listener1.receivedEvents == 2);
}

/**
 * Tests Unsubscribe All listeners
 */
void testUnsubscribeAllWithContext() {
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &listener);
    publisher.subscribe(1, &listener1, &listener1);
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener1, TestEventArgs());
    publisher.unSubscribeAll(1);
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener1, TestEventArgs());
    publisher.unSubscribeAll(1, &listener);
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener1, TestEventArgs());
    publisher.unSubscribeAll(1, &listener1);
    publisher.publish(1, &listener, TestEventArgs());
    publisher.publish(1, &listener1, TestEventArgs());

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
    assert(listener1.receivedCallbackEvents == 0);
    assert(listener1.receivedCallbackNonEvents == 3);
    assert(listener1.receivedEvents == 3);
}

void EventsTests::testAll() {
    testUnregister();
    testEvent();
    testCallback();
    testContextCallback();
    testUnsubscribeEvent();
    testUnsubscribeContextEvent();
    testUnsubscribeAll();
    testUnsubscribeAllWithContext();
}