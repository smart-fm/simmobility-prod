/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Test.h
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 28, 2013, 6:09 PM
 */
#pragma once
#include "event/EventPublisher.hpp"
#include "event/EventManager.hpp"


namespace sim_mob {
    namespace long_term {
        
        
        DECLARE_CUSTOM_CALLBACK_TYPE(MyArgs)
        class MyArgs : public EventArgs {
        public:
            MyArgs();
            virtual ~MyArgs();
            const int Print() const;
        };
        
        class Test : public EventPublisher {
        public:
            Test();
            virtual ~Test();
        public:
            
 

            class Subscriber : public EventListener {
            public:
                Subscriber();
                virtual ~Subscriber();
                void OnEvent(EventId id, EventPublisher* sender, const EventArgs& args);
                void OnEvent(EventId id, Context ctxId, EventPublisher* sender, const EventArgs& args);
                void OnEvent1(EventId id, Context ctxId, EventPublisher* sender, const EM_EventArgs& args);
                void OnMyArgs(EventId id, EventPublisher* sender, const MyArgs& args);
                void OnMyArgs(EventId id, Context ctxId, EventPublisher* sender, const MyArgs& args);
            };
        };
       
    }
    
}

