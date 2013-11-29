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

        virtual void OnEvent(sim_mob::event::EventId id,
                sim_mob::event::Context ctxId,
                sim_mob::event::EventPublisher* sender,
                const EventArgs& args) {
            receivedEvents++;
            receivedCallbackNonEvents++;
        }

        void OnTest(sim_mob::event::EventId id,
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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener);
    publisher.Subscribe(1, &listener1);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.UnRegisterEvent(1);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());

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
    publisher.RegisterEvent(1);
    publisher.RegisterEvent(2);
    publisher.Subscribe(1, &listener);
    publisher.Subscribe(2, &listener);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.Publish(1, TestEventArgs());
        publisher.Publish(2, TestEventArgs());
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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener, &TestListener::OnTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.Publish(1, TestEventArgs());
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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener, &TestListener::OnTest, &listener);
    publisher.Subscribe(2, &listener, &TestListener::OnTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.Publish(1, TestEventArgs());
        publisher.Publish(1, &listener, TestEventArgs());
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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.UnSubscribe(1, &listener);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());

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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener, &listener);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.UnSubscribe(1, &listener);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.UnSubscribe(1, &listener, &listener);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener, TestEventArgs());

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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener);
    publisher.Subscribe(1, &listener1);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());
    publisher.UnSubscribeAll(1);
    publisher.Publish(1, TestEventArgs());
    publisher.Publish(1, TestEventArgs());

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
    publisher.RegisterEvent(1);
    publisher.Subscribe(1, &listener, &listener);
    publisher.Subscribe(1, &listener1, &listener1);
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener1, TestEventArgs());
    publisher.UnSubscribeAll(1);
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener1, TestEventArgs());
    publisher.UnSubscribeAll(1, &listener);
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener1, TestEventArgs());
    publisher.UnSubscribeAll(1, &listener1);
    publisher.Publish(1, &listener, TestEventArgs());
    publisher.Publish(1, &listener1, TestEventArgs());

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