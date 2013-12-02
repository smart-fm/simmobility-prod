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
#include "logging/Log.hpp"

using namespace sim_mob::event;
using namespace unit_tests;
using std::cout;
using std::endl;

namespace {
    const int TEST_SIZE = 1000;

    class TestObj {
    public:

        TestObj() : value(10), data("Obj") {
        }
        int value;
        std::string data;
    };

    class TestObjContainer {
    public:

        TestObjContainer(TestObj& obj) : value(20), data("Obj1"), obj(obj) {
        }
        TestObj& obj;
        int value;
        std::string data;
    };

    class TestNonEventArgs {
    };

    class TestEventArgs : public EventArgs {
    public:

        TestEventArgs(TestObjContainer& obj) : obj(obj) {
        }

        TestObjContainer& obj;
    };
    
    class TestSimpleEventArgs : public EventArgs{
    public:
        TestSimpleEventArgs() : value(30) {
        }
        int value;
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

            assert(args.obj.value == 20);
            assert(args.obj.data == "Obj1");
            assert(args.obj.obj.value == 10);
            assert(args.obj.obj.data == "Obj");
        }
        
        void onSimple(sim_mob::event::EventId id,
                sim_mob::event::Context ctxId,
                sim_mob::event::EventPublisher* sender,
                const TestSimpleEventArgs& args) {
            receivedEvents++;
            receivedCallbackEvents++;
            assert(args.value == 30);
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
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.subscribe(1, &listener1);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.unRegisterEvent(1);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));

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
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.registerEvent(2);
    publisher.subscribe(1, &listener);
    publisher.subscribe(2, &listener);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs(obj1));
        publisher.publish(2, TestEventArgs(obj1));
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
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &TestListener::onTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs(obj1));
    }
    assert(listener.receivedCallbackEvents == TEST_SIZE);
    assert(listener.receivedCallbackNonEvents == 0);
    assert(listener.receivedEvents == TEST_SIZE);
}

/**
 * Tests context callback
 */
void testContextCallback() {
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &TestListener::onTest, &listener);
    publisher.subscribe(2, &listener, &TestListener::onTest);
    for (int i = 0; i < TEST_SIZE; i++) {
        publisher.publish(1, TestEventArgs(obj1));
        publisher.publish(1, &listener, TestEventArgs(obj1));
    }
    assert(listener.receivedCallbackEvents == TEST_SIZE);
    assert(listener.receivedCallbackNonEvents == 0);
    assert(listener.receivedEvents == TEST_SIZE);
}

/**
 * Tests Unsubscribe event
 */
void testUnsubscribeEvent() {
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.unSubscribe(1, &listener);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
}

/**
 * Tests Unsubscribe event
 */
void testUnsubscribeContextEvent() {
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &listener);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.unSubscribe(1, &listener);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.unSubscribe(1, &listener, &listener);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener, TestEventArgs(obj1));

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 4);
    assert(listener.receivedEvents == 4);
}

/**
 * Tests Unsubscribe All listeners
 */
void testUnsubscribeAll() {
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener);
    publisher.subscribe(1, &listener1);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));
    publisher.unSubscribeAll(1);
    publisher.publish(1, TestEventArgs(obj1));
    publisher.publish(1, TestEventArgs(obj1));

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
    TestObj obj;
    TestObjContainer obj1(obj);
    TestListener listener;
    TestListener listener1;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.subscribe(1, &listener, &listener);
    publisher.subscribe(1, &listener1, &listener1);
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener1, TestEventArgs(obj1));
    publisher.unSubscribeAll(1);
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener1, TestEventArgs(obj1));
    publisher.unSubscribeAll(1, &listener);
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener1, TestEventArgs(obj1));
    publisher.unSubscribeAll(1, &listener1);
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(1, &listener1, TestEventArgs(obj1));

    assert(listener.receivedCallbackEvents == 0);
    assert(listener.receivedCallbackNonEvents == 2);
    assert(listener.receivedEvents == 2);
    assert(listener1.receivedCallbackEvents == 0);
    assert(listener1.receivedCallbackNonEvents == 3);
    assert(listener1.receivedEvents == 3);
}

/**
 * Tests Unsubscribe All listeners
 */
void testDifferentEventArgs() {
    TestObj obj;
    TestObjContainer obj1(obj);

    TestListener listener;
    TestPublisher publisher;
    publisher.registerEvent(1);
    publisher.registerEvent(2);
    publisher.subscribe(1, &listener, &listener);
    publisher.subscribe(1, &listener, &TestListener::onTest, &listener);
    publisher.subscribe(2, &listener, &TestListener::onSimple, &listener);
    publisher.publish(1, &listener, TestEventArgs(obj1));
    publisher.publish(2, &listener, TestSimpleEventArgs());
    assert(listener.receivedCallbackEvents == 2);
    assert(listener.receivedCallbackNonEvents == 1);
    assert(listener.receivedEvents == 3);
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
    testDifferentEventArgs();
}