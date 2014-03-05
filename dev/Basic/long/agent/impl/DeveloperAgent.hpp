//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DeveloperAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Mar 5, 2014, 6:36 PM
 */
#pragma once
#include "agent/LT_Agent.hpp"
#include "database/entity/Developer.hpp"

namespace sim_mob {

    namespace long_term {

        class DeveloperAgent : public LT_Agent {
        public:
            DeveloperAgent(Developer* developer);
            virtual ~DeveloperAgent();
        protected:
            /**
             * Inherited from LT_Agent.
             */
            bool onFrameInit(timeslice now);
            sim_mob::Entity::UpdateStatus onFrameTick(timeslice now);
            void onFrameOutput(timeslice now);

            /**
             * Inherited from Entity. 
             */
            void onWorkerEnter();
            void onWorkerExit();
            virtual void HandleMessage(messaging::Message::MessageType type,
                    const messaging::Message& message);
        private:

            /**
             * Events callbacks.
             */
            virtual void onEvent(event::EventId eventId, event::Context ctxId,
                    event::EventPublisher* sender, const event::EventArgs& args);
        
        private:
            Developer* developer;
        };
    }
}

