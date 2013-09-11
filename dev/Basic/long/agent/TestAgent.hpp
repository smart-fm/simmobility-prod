/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Agent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 13, 2013, 6:36 PM
 */
#pragma once
#include "Common.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/Agent.hpp"
#include "event/EventManager.hpp"
#include "message/MessageHandler.hpp"

namespace sim_mob {

    namespace long_term {

        class TestAgent : public sim_mob::Agent{
        public:
            TestAgent(int id, messaging::MessageHandler* receiver);
            virtual ~TestAgent();

            /**
             * Inherited from Agent.
             */
            virtual void load(const std::map<std::string, std::string>& configProps);
            
            virtual void OnEvent(sim_mob::event::EventId id, 
                                 sim_mob::event::EventPublisher* sender, 
                                 const sim_mob::event::EventArgs& args);

            virtual void OnEvent(sim_mob::event::EventId id, 
                                 sim_mob::event::Context ctxId, 
                                 sim_mob::event::EventPublisher* sender, 
                                 const sim_mob::event::EventArgs& args);

        protected:

            /**
             * Inherited from MessageReceiver.
             */
            virtual void HandleMessage(messaging::Message::MessageType type,
                const messaging::Message& message);
            
            

            /**
             * Inherited from Agent.
             */
            bool frame_init(timeslice now);
            sim_mob::Entity::UpdateStatus frame_tick(timeslice now);
            void frame_output(timeslice now);
            bool isNonspatial();    
            bool isRegistered;
            messaging::MessageHandler* receiver;
        };
    }
}

